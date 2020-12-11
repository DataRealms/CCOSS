#include "LoadingGUI.h"
#include "Writer.h"
#include "DataModule.h"
#include "SceneLayer.h"
#include "SettingsMan.h"
#include "PresetMan.h"
#include "FrameMan.h"

#include "GUI/GUI.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUIListBox.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroInput.h"

#include "unzip.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::Clear() {
		m_ControlManager = nullptr;
		m_GUIInput = nullptr;
		m_GUIScreen = nullptr;
		m_LoadingLogWriter = nullptr;
		m_ProgressListboxBitmap = nullptr;
		m_PosX = 0;
		m_PosY = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::InitLoadingScreen() {
		g_FrameMan.LoadPalette("Base.rte/palette.bmp");
		g_FrameMan.ClearBackBuffer32();

		m_GUIInput.reset(new AllegroInput(-1));
		m_GUIScreen.reset(new AllegroScreen(g_FrameMan.GetBackBuffer32()));

		if (!m_ControlManager) {
			m_ControlManager.reset(new GUIControlManager());
			if (!m_ControlManager->Create(m_GUIScreen.get(), m_GUIInput.get(), "Base.rte/GUIs/Skins/MainMenu", "LoadingSkin.ini")) {
				RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu/LoadingSkin.ini");
			}
			m_ControlManager->Load("Base.rte/GUIs/LoadingGUI.ini");
		}
		if (!g_SettingsMan.DisableLoadingScreen()) { CreateProgressReportListbox(); }

		SceneLayer loadingSplash;
		loadingSplash.Create(ContentFile("Base.rte/GUIs/Title/LoadingSplash.png"), false, Vector(), true, false, Vector(1.0F, 0));

		// Hardcoded offset to make room for the loading box only if DisableLoadingScreen is false.
		int loadingSplashOffset = g_SettingsMan.DisableLoadingScreen() ? 14 : 120;
		loadingSplash.SetOffset(Vector(static_cast<float>(((loadingSplash.GetBitmap()->w - g_FrameMan.GetResX()) / 2) + loadingSplashOffset), 0));

		// Draw onto wrapped strip centered vertically on the screen
		Box splashBox(Vector(0, static_cast<float>((g_FrameMan.GetResY() - loadingSplash.GetBitmap()->h) / 2)), static_cast<float>(g_FrameMan.GetResX()), static_cast<float>(loadingSplash.GetBitmap()->h));
		loadingSplash.Draw(g_FrameMan.GetBackBuffer32(), splashBox);

		g_FrameMan.FlipFrameBuffers();

		// Overwrite Settings.ini after all the managers are created to fully populate the file. Up until this moment Settings.ini is populated only with minimal required properties to run.
		// When the overwrite happens there is a short delay which causes the screen to remain black, so this is done here after the flip to mask that black screen.
		if (g_SettingsMan.SettingsNeedOverwrite()) { g_SettingsMan.UpdateSettingsFile(); }

		if (!m_LoadingLogWriter) {
			m_LoadingLogWriter.reset(new Writer("LogLoading.txt"));
			if (!m_LoadingLogWriter->WriterOK()) {
				ShowMessageBox("Failed to instantiate the Loading Log writer!\nModule loading will proceed without being logged!");
				m_LoadingLogWriter.reset();
			}
		}

		LoadDataModules();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::CreateProgressReportListbox() {
		// Place and clear the sectionProgress box
		dynamic_cast<GUICollectionBox *>(m_ControlManager->GetControl("root"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
		GUIListBox *listBox = dynamic_cast<GUIListBox *>(m_ControlManager->GetControl("ProgressBox"));

		// Make the box a bit bigger if there's room in higher, HD resolutions
		if (g_FrameMan.GetResX() >= 960) { listBox->Resize((g_FrameMan.GetResX() / 3) - 12, listBox->GetHeight()); }

		// Make the loading progress box fill the right third of the screen
		listBox->SetPositionRel(g_FrameMan.GetResX() - listBox->GetWidth() - 12, (g_FrameMan.GetResY() / 2) - (listBox->GetHeight() / 2));
		listBox->ClearList();

		if (!m_ProgressListboxBitmap) {
			listBox->SetVisible(false);
			m_ProgressListboxBitmap = create_bitmap_ex(8, listBox->GetWidth(), listBox->GetHeight());
			clear_to_color(m_ProgressListboxBitmap, 54);
			rect(m_ProgressListboxBitmap, 0, 0, listBox->GetWidth() - 1, listBox->GetHeight() - 1, 33);
			rect(m_ProgressListboxBitmap, 1, 1, listBox->GetWidth() - 2, listBox->GetHeight() - 2, 33);
			m_PosX = listBox->GetXPos();
			m_PosY = listBox->GetYPos();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::Destroy() {
		if (m_ProgressListboxBitmap) { destroy_bitmap(m_ProgressListboxBitmap); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::LoadingSplashProgressReport(const std::string &reportString, bool newItem) {
		if (System::IsLoggingToCLI()) { System::PrintLoadingToCLI(reportString, newItem); }

		if (newItem) {
			// Write out the last line to the log file before starting a new one and scroll the bitmap upwards
			if (g_LoadingGUI.m_LoadingLogWriter) { *g_LoadingGUI.m_LoadingLogWriter << reportString << "\n"; }
			if (g_LoadingGUI.m_ProgressListboxBitmap) { blit(g_LoadingGUI.m_ProgressListboxBitmap, g_LoadingGUI.m_ProgressListboxBitmap, 2, 12, 2, 2, g_LoadingGUI.m_ProgressListboxBitmap->w - 3, g_LoadingGUI.m_ProgressListboxBitmap->h - 12); }
		}

		if (g_LoadingGUI.m_ProgressListboxBitmap) {
			AllegroBitmap drawBitmap(g_LoadingGUI.m_ProgressListboxBitmap);

			// Clear current line
			rectfill(g_LoadingGUI.m_ProgressListboxBitmap, 2, g_LoadingGUI.m_ProgressListboxBitmap->h - 12, g_LoadingGUI.m_ProgressListboxBitmap->w - 3, g_LoadingGUI.m_ProgressListboxBitmap->h - 3, 54);
			// Print new line
			g_FrameMan.GetSmallFont()->DrawAligned(&drawBitmap, 5, g_LoadingGUI.m_ProgressListboxBitmap->h - 12, reportString.c_str(), GUIFont::Left);
			// DrawAligned - MaxWidth is useless here, so we're just drawing lines manually
			vline(g_LoadingGUI.m_ProgressListboxBitmap, g_LoadingGUI.m_ProgressListboxBitmap->w - 2, g_LoadingGUI.m_ProgressListboxBitmap->h - 12, g_LoadingGUI.m_ProgressListboxBitmap->h - 2, 33);
			vline(g_LoadingGUI.m_ProgressListboxBitmap, g_LoadingGUI.m_ProgressListboxBitmap->w - 1, g_LoadingGUI.m_ProgressListboxBitmap->h - 12, g_LoadingGUI.m_ProgressListboxBitmap->h - 2, 33);

			// Draw onto current frame buffer
			blit(g_LoadingGUI.m_ProgressListboxBitmap, g_FrameMan.GetBackBuffer32(), 0, 0, g_LoadingGUI.m_PosX, g_LoadingGUI.m_PosY, g_LoadingGUI.m_ProgressListboxBitmap->w, g_LoadingGUI.m_ProgressListboxBitmap->h);

			g_FrameMan.FlipFrameBuffers();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LoadingGUI::LoadDataModules() {
		g_PresetMan.Destroy();
		g_PresetMan.Initialize();
		ExtractZippedModules();
		g_PresetMan.LoadAllDataModules();

		// Free any resources used by this because we won't be needing it anymore
		Destroy();

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::ExtractZippedModules() const {
		for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(System::GetWorkingDirectory())) {
			std::string zippedModulePath = std::filesystem::path(directoryEntry).generic_string();
			std::string zippedModuleName = std::filesystem::path(directoryEntry).filename().generic_string();

			if (zippedModulePath.find(System::GetZippedModulePackageExtension()) == zippedModulePath.length() - System::GetZippedModulePackageExtension().length()) {
				LoadingSplashProgressReport("Extracting Data Module from: " + zippedModuleName, true);

				// Try to open the zipped and unzip it into place as an exposed data module
				unzFile zippedModule = unzOpen(zippedModuleName.c_str());
				bool abortExtract = false;

				if (zippedModule) {
					unz_global_info zippedModuleInfo;
					if (unzGetGlobalInfo(zippedModule, &zippedModuleInfo) != UNZ_OK) {
						LoadingSplashProgressReport("\tSkipping: " + zippedModuleName + " - Could not read global file info!", true);
						abortExtract = true;
					}
					// Buffer to hold data read from the zip file
					std::array<char, s_FileBufferSize> fileBuffer;

					// Go through and extract every file inside this zip, overwriting every colliding file that already exists in the install directory
					for (int i = 0; i < zippedModuleInfo.number_entry && !abortExtract; ++i) {
						unz_file_info currentFileInfo;
						std::array<char, s_MaxFileName> outputFileInfoData;
						if (unzGetCurrentFileInfo(zippedModule, &currentFileInfo, outputFileInfoData.data(), s_MaxFileName, nullptr, 0, nullptr, 0) != UNZ_OK) {
							LoadingSplashProgressReport("\tSkipping: " + std::string(outputFileInfoData.data()) + " - Could not read file info!", true);
							continue;
						}
						std::string outputFileName = outputFileInfoData.data();
#ifdef _WIN32
						// TODO: Windows 10 adds support for paths over 260 characters so investigate how to get Windows version and whether the setting is enabled at runtime.
						// Windows doesn't support paths over 260 characters long.
						if ((System::GetWorkingDirectory() + outputFileName).length() >= MAX_PATH) {
							LoadingSplashProgressReport("\tSkipping file: " + outputFileName + " - Full path to file exceeds 260 characters!", true);
							continue;
						}
#endif
						// Check if the directory we are trying to extract into exists, and if not, create it
						std::string outputFileDirectory = outputFileName.substr(0, outputFileName.find_last_of("/\\") + 1);
						if (!std::filesystem::exists(outputFileDirectory)) {
							LoadingSplashProgressReport("\tCreating directory: " + outputFileName, true);
							if (!System::MakeDirectory(System::GetWorkingDirectory() + outputFileDirectory)) {
								LoadingSplashProgressReport("\tFailed to create directory: " + outputFileName + " - Aborting extraction!", true);
								abortExtract = true;
								continue;
							}
						}
						// If the output file is a directly, go the next entry listed in the zip file
						if (std::filesystem::is_directory(outputFileName)) {
							unzCloseCurrentFile(zippedModule);
							if ((i + 1) < zippedModuleInfo.number_entry && unzGoToNextFile(zippedModule) != UNZ_OK) {
								LoadingSplashProgressReport("\tCould not read next file inside zip - Aborting extraction!", true);
								abortExtract = true;
							}
							continue;
						}

						// Validate so only certain file types are extracted
						std::string fileExtension = std::filesystem::path(outputFileName).extension().generic_string();
						std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower);

						if (s_SupportedExtensions.find(fileExtension) == s_SupportedExtensions.end()) {
							LoadingSplashProgressReport("\tSkipping file: " + outputFileName + " - Bad extension!", true);
							unzCloseCurrentFile(zippedModule);

							// Go the next entry listed in the zip file
							if ((i + 1) < zippedModuleInfo.number_entry && unzGoToNextFile(zippedModule) != UNZ_OK) {
								LoadingSplashProgressReport("\tCould not read next file inside zip - Aborting extraction!", true);
								abortExtract = true;
							}
							continue;
						}

						LoadingSplashProgressReport("\tExtracting file: " + outputFileName, true);
						if (unzOpenCurrentFile(zippedModule) != UNZ_OK) { LoadingSplashProgressReport("\tSkipping file: " + zippedModuleName + " - Could not open file!", true); }

						// Open a file to write out the data.
						FILE *outputFile = fopen(outputFileName.c_str(), "wb");
						if (outputFile == nullptr) { LoadingSplashProgressReport("\tSkipping file: " + outputFileName + " - Could not open/create destination file!", true); }

						// Write the entire file out, reading in buffer size chunks and spitting them out to the output stream
						bool abortWrite = false;
						int bytesRead = 0;
						int totalBytesRead = 0;
						do {
							bytesRead = unzReadCurrentFile(zippedModule, fileBuffer.data(), s_FileBufferSize);
							totalBytesRead += bytesRead;

							if (bytesRead < 0) {
								LoadingSplashProgressReport("\tSkipping file: " + outputFileName + " - File is empty or corrupt!", true);
								abortWrite = true;
							// Sanity check how damn big this file we're writing is becoming. could prevent zip bomb exploits: http://en.wikipedia.org/wiki/Zip_bomb
							} else if (totalBytesRead >= s_MaxUnzippedFileSize) {
								LoadingSplashProgressReport("\tSkipping file: " + outputFileName + " - File is too large, extract it manually!", true);
								abortWrite = true;
							}
							if (abortWrite) {
								break;
							}
							fwrite(fileBuffer.data(), bytesRead, 1, outputFile);
						// Keep going while bytes are still being read (0 means end of file)
						} while (bytesRead > 0 && outputFile);

						fclose(outputFile);
						unzCloseCurrentFile(zippedModule);

						// Go the next entry listed in the zip file.
						if ((i + 1) < zippedModuleInfo.number_entry && unzGoToNextFile(zippedModule) != UNZ_OK) {
							LoadingSplashProgressReport("\tCould not read next file inside zip - Aborting extraction!", true);
							abortExtract = true;
						}
					}
					unzClose(zippedModule);
					LoadingSplashProgressReport("Successfully extracted Data Module from: " + zippedModuleName + " - Deleting zip file!\n", true);
					std::remove((System::GetWorkingDirectory() + zippedModuleName).c_str());
				} else {
					bool makeDirResult = false;
					if (!std::filesystem::exists(System::GetWorkingDirectory() + "_FailedExtract")) { makeDirResult = System::MakeDirectory(System::GetWorkingDirectory() + "_FailedExtract"); }
					if (makeDirResult) {
						LoadingSplashProgressReport("Failed to extract Data module from: " + zippedModuleName + " - Moving zip file to failed extract directory!\n", true);
						std::filesystem::rename(System::GetWorkingDirectory() + zippedModuleName, System::GetWorkingDirectory() + "_FailedExtract/" + zippedModuleName);
					} else {
						LoadingSplashProgressReport("Failed to extract Data module from: " + zippedModuleName + " - Failed to create directory to move zip file into, deleting zip file!\n", true);
						std::remove((System::GetWorkingDirectory() + zippedModuleName).c_str());
					}
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::GUIControlManagerDeleter::operator()(GUIControlManager *ptr) const { ptr->Destroy(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::AllegroInputDeleter::operator()(AllegroInput *ptr) const { ptr->Destroy(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::AllegroScreenDeleter::operator()(AllegroScreen *ptr) const { ptr->Destroy(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::WriterDeleter::operator()(Writer *ptr) const { ptr->EndWrite(); }
}