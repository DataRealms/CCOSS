#include "LoadingGUI.h"
#include "System.h"
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
		m_ControlManager = 0;
		m_GUIInput = 0;
		m_GUIScreen = 0;
		m_LoadingLogWriter = 0;
		m_LoadingGUIBitmap = 0;
		m_PosX = 0;
		m_PosY = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//TODO Consider splitting this into a Create method and Init method to standardize LoadingGUI. Need to research if it can viably be done.
	void LoadingGUI::InitLoadingScreen() {
		g_FrameMan.LoadPalette("Base.rte/palette.bmp");

		// Create the main GUI
		m_GUIInput = new AllegroInput(-1);
		m_GUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32());

		// Loading splash screen
		g_FrameMan.ClearBackBuffer32();
		SceneLayer *loadingSplash = new SceneLayer();
		loadingSplash->Create(ContentFile("Base.rte/GUIs/Title/LoadingSplash.png"), false, Vector(), true, false, Vector(1.0, 0));

		// Hardcoded offset to make room for the loading box only if DisableLoadingScreen is false.
		int loadingSplashOffset = g_SettingsMan.DisableLoadingScreen() ? 14 : 120;
		loadingSplash->SetOffset(Vector(((loadingSplash->GetBitmap()->w - g_FrameMan.GetResX()) / 2) + loadingSplashOffset, 0));

		// Draw onto wrapped strip centered vertically on the screen
		Box splashBox(Vector(0, (g_FrameMan.GetResY() - loadingSplash->GetBitmap()->h) / 2), g_FrameMan.GetResX(), loadingSplash->GetBitmap()->h);
		loadingSplash->Draw(g_FrameMan.GetBackBuffer32(), splashBox);
		delete loadingSplash;
		loadingSplash = 0;

		g_FrameMan.FlipFrameBuffers();

		if (g_SettingsMan.SettingsNeedOverwrite()) {
			// Overwrite Settings.ini after all the managers are created to fully populate the file. Up until this moment Settings.ini is populated only with minimal required properties to run.
			// When the overwrite happens there is a short delay which causes the screen to remain black, so this is done here after the flip to mask that black screen.
			g_SettingsMan.UpdateSettingsFile();
		}

		// Set up the loading GUI
		if (!m_ControlManager) {
			m_ControlManager = new GUIControlManager();

			// TODO: This should be using the 32bpp main menu skin, but isn't because it needs the config of the base for its listbox
			// Can get away with this hack for now because the list box that the loading menu uses displays ok when drawn on a 32bpp buffer,
			// when it's 8bpp internally, since it does not use any masked_blit calls to draw list boxes.
			// Note also how the GUIScreen passed in here has been created with an 8bpp bitmap, since that is what determines what the GUI manager uses internally
			if (!m_ControlManager->Create(m_GUIScreen, m_GUIInput, "Base.rte/GUIs/Skins/MainMenu", "LoadingSkin.ini")) {
				RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu/LoadingSkin.ini");
			}
			m_ControlManager->Load("Base.rte/GUIs/LoadingGUI.ini");
		}

		// Place and clear the sectionProgress box
		dynamic_cast<GUICollectionBox *>(m_ControlManager->GetControl("root"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
		GUIListBox *listBox = dynamic_cast<GUIListBox *>(m_ControlManager->GetControl("ProgressBox"));
		// Make the box a bit bigger if there's room in higher, HD resolutions
		if (g_FrameMan.GetResX() >= 960) {
			// Make the loading progress box fill the right third of the screen
			listBox->Resize((g_FrameMan.GetResX() / 3) - 12, listBox->GetHeight());
			listBox->SetPositionRel(g_FrameMan.GetResX() - listBox->GetWidth() - 12, (g_FrameMan.GetResY() / 2) - (listBox->GetHeight() / 2));
		} else {
			// Legacy positioning and sizing when running low resolutions
			listBox->SetPositionRel(g_FrameMan.GetResX() - listBox->GetWidth() - 12, (g_FrameMan.GetResY() / 2) - (listBox->GetHeight() / 2));
		}
		listBox->ClearList();

		// New mechanism to speed up loading times as it turned out that a massive amount of time is spent to update GUI control.
		if (!g_SettingsMan.DisableLoadingScreen() && !m_LoadingGUIBitmap) {
			listBox->SetVisible(false);
			m_LoadingGUIBitmap = create_bitmap_ex(8, listBox->GetWidth(), listBox->GetHeight());
			clear_to_color(m_LoadingGUIBitmap, 54);
			rect(m_LoadingGUIBitmap, 0, 0, listBox->GetWidth() - 1, listBox->GetHeight() - 1, 33);
			rect(m_LoadingGUIBitmap, 1, 1, listBox->GetWidth() - 2, listBox->GetHeight() - 2, 33);
			m_PosX = listBox->GetXPos();
			m_PosY = listBox->GetYPos();
		}
		// Create the loading log writer
		if (!m_LoadingLogWriter) { m_LoadingLogWriter = new Writer("LogLoading.txt"); }

		// Load all the data modules
		LoadDataModules();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::LoadingSplashProgressReport(std::string reportString, bool newItem) {
		if (g_System.GetLogToCLI()) { g_System.PrintLoadingToCLI(reportString, newItem); }

		if (g_LoadingGUI.m_ControlManager) {
			if (newItem) {
				// Write out the last line to the log file before starting a new one
				if (g_LoadingGUI.m_LoadingLogWriter->WriterOK()) { *g_LoadingGUI.m_LoadingLogWriter << reportString << "\n"; }
				// Scroll bitmap upwards
				if (g_LoadingGUI.m_LoadingGUIBitmap) { blit(g_LoadingGUI.m_LoadingGUIBitmap, g_LoadingGUI.m_LoadingGUIBitmap, 2, 12, 2, 2, g_LoadingGUI.m_LoadingGUIBitmap->w - 3, g_LoadingGUI.m_LoadingGUIBitmap->h - 12); }
			}
			if (g_LoadingGUI.m_LoadingGUIBitmap) {
				AllegroBitmap bmp(g_LoadingGUI.m_LoadingGUIBitmap);
				// Clear current line
				rectfill(g_LoadingGUI.m_LoadingGUIBitmap, 2, g_LoadingGUI.m_LoadingGUIBitmap->h - 12, g_LoadingGUI.m_LoadingGUIBitmap->w - 3, g_LoadingGUI.m_LoadingGUIBitmap->h - 3, 54);
				// Print new line
				g_FrameMan.GetSmallFont()->DrawAligned(&bmp, 5, g_LoadingGUI.m_LoadingGUIBitmap->h - 12, reportString.c_str(), GUIFont::Left);
				// DrawAligned - MaxWidth is useless here, so we're just drawing lines manually
				vline(g_LoadingGUI.m_LoadingGUIBitmap, g_LoadingGUI.m_LoadingGUIBitmap->w - 2, g_LoadingGUI.m_LoadingGUIBitmap->h - 12, g_LoadingGUI.m_LoadingGUIBitmap->h - 2, 33);
				vline(g_LoadingGUI.m_LoadingGUIBitmap, g_LoadingGUI.m_LoadingGUIBitmap->w - 1, g_LoadingGUI.m_LoadingGUIBitmap->h - 12, g_LoadingGUI.m_LoadingGUIBitmap->h - 2, 33);

				// Draw onto current frame buffer
				blit(g_LoadingGUI.m_LoadingGUIBitmap, g_FrameMan.GetBackBuffer32(), 0, 0, g_LoadingGUI.m_PosX, g_LoadingGUI.m_PosY, g_LoadingGUI.m_LoadingGUIBitmap->w, g_LoadingGUI.m_LoadingGUIBitmap->h);

				g_FrameMan.FlipFrameBuffers();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LoadingGUI::LoadDataModules() {
		g_PresetMan.Destroy();
		g_PresetMan.Create();
		ExtractZippedModules();
		g_PresetMan.LoadAllDataModules();

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::ExtractZippedModules() {
		al_ffblk zippedModuleInfo;
		unzFile zipFile;
		for (int result = al_findfirst("*.rte.zip", &zippedModuleInfo, FA_ALL); result == 0; result = al_findnext(&zippedModuleInfo)) {

			// Report that we are attempting to unzip this thing
			LoadingSplashProgressReport("Unzipping " + std::string(zippedModuleInfo.name), true);

			// Try to open the zipped and unzip it into place as an exposed data module
			if (std::strlen(zippedModuleInfo.name) > 0 && (zipFile = unzOpen(zippedModuleInfo.name))) {
				// Go through and extract every file inside this zip, overwriting every colliding file that already exists in the install directory 

				// Get info about the zip file
				unz_global_info zipFileInfo;
				if (unzGetGlobalInfo(zipFile, &zipFileInfo) != UNZ_OK) {
					LoadingSplashProgressReport("Could not read global file info of: " + string(zippedModuleInfo.name), true);
				}

				// Buffer to hold data read from the zip file.
				char fileBuffer[s_FileBufferSize];

				// Loop to extract all files
				bool abortExtract = false;
				for (unsigned long i = 0; i < zipFileInfo.number_entry && !abortExtract; ++i) {
					// Get info about current file.
					unz_file_info fileInfo;
					char outputFileName[s_MaxFileName];
					if (unzGetCurrentFileInfo(zipFile, &fileInfo, outputFileName, s_MaxFileName, NULL, 0, NULL, 0) != UNZ_OK) {
						LoadingSplashProgressReport("Could not read file info of: " + std::string(outputFileName), true);
					}

					// Check if the directory we are trying to extract into exists, and if not, create it
					char outputDirName[s_MaxFileName];
					char parentDirName[s_MaxFileName];
					// Copy the file path to a separate directory path
					strcpy_s(outputDirName, sizeof(outputDirName), outputFileName);
					// Find the last slash in the directory path, so we can cut off everything after that (ie the actual filename), and only have the directory path left
					char *slashPos = strrchr(outputDirName, '/');
					// Try to find the other kind of slash if we found none
					if (!slashPos) { slashPos = strrchr(outputDirName, '\\'); }
					// Now that we have the slash position, terminate the directory path string right after there
					if (slashPos) { *(++slashPos) = 0; }

					// If that file's directory doesn't exist yet, then create it, and all its parent directories above if need be
					for (int nested = 0; !std::filesystem::exists(outputDirName) && slashPos; ++nested) {
						// Keep making new working copies of the path that we can dice up
						strcpy_s(parentDirName, sizeof(parentDirName), outputDirName[0] == '.' ? &(outputDirName[2]) : outputDirName);
						// Start off at the beginning
						slashPos = parentDirName;
						for (int j = 0; j <= nested && slashPos; ++j) {
							// Find the first slash so we can isolate the folders in the hierarchy, in descending seniority
							slashPos = strchr(slashPos, '/');
							// If we can't find any more slashes, then quit
							if (!slashPos) { break; }
							// If we did find a slash, go to one past it slash and try to find the next one
							slashPos++;
						}
						// No more nested folders to make
						if (!slashPos) { break; }
						// Terminate there so we are making the most senior folder
						*(slashPos) = 0;
						g_System.MakeDirectory(parentDirName);
					}

					// Check if this entry is a directory or file
					if (outputFileName[strlen(outputFileName) - 1] == '/' || outputFileName[strlen(outputFileName) - 1] == '\\') {
						// Entry is a directory, so create it.
						LoadingSplashProgressReport("Creating Dir: " + std::string(outputFileName), true);
						g_System.MakeDirectory(outputFileName);
						// It's a file
					} else {
						// Validate so only certain file types are extracted:  .ini .txt .lua .cfg .bmp .png .jpg .jpeg .wav .ogg .mp3
						// Get the file extension
						std::string fileExtension = std::filesystem::path(outputFileName).extension().string();
						std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
						const char *ext = fileExtension.c_str();
						// Validate only certain file types to be included! .ini .txt .lua .cfg .bmp .png .jpg .jpeg .wav .ogg .mp3
						if (!(std::strcmp(ext, ".ini") == 0 || std::strcmp(ext, ".txt") == 0 || std::strcmp(ext, ".lua") == 0 || std::strcmp(ext, ".cfg") == 0 ||
							std::strcmp(ext, ".bmp") == 0 || std::strcmp(ext, ".png") == 0 || std::strcmp(ext, ".jpg") == 0 || std::strcmp(ext, ".jpeg") == 0 ||
							std::strcmp(ext, ".wav") == 0 || std::strcmp(ext, ".ogg") == 0 || std::strcmp(ext, ".mp3") == 0)) {
							LoadingSplashProgressReport("Skipping: " + std::string(outputFileName) + " - bad extension!", true);

							// Keep going through!!
							// Close the read file within the zip archive
							unzCloseCurrentFile(zipFile);
							// Go the next entry listed in the zip file.
							if ((i + 1) < zipFileInfo.number_entry) {
								if (unzGoToNextFile(zipFile) != UNZ_OK) {
									LoadingSplashProgressReport("Could not read next file inside zip " + std::string(zippedModuleInfo.name) + " - Aborting extraction!", true);
									abortExtract = true;
									break;
								}
							}
							// Onto the next file
							continue;
						}
						// Entry is a file, so extract it.
						LoadingSplashProgressReport("Extracting: " + std::string(outputFileName), true);
						if (unzOpenCurrentFile(zipFile) != UNZ_OK) { LoadingSplashProgressReport("Could not open file within " + std::string(zippedModuleInfo.name), true);	}

						// Open a file to write out the data.
						FILE *outputFile = fopen(outputFileName, "wb");
						if (outputFile == NULL) { LoadingSplashProgressReport("Could not open/create destination file while unzipping " + std::string(zippedModuleInfo.name), true); }

						// Write the entire file out, reading in buffer size chunks and spitting them out to the output stream
						int bytesRead = 0;
						int64_t totalBytesRead = 0;
						do {
							// Read a chunk
							bytesRead = unzReadCurrentFile(zipFile, fileBuffer, s_FileBufferSize);
							// Add to total tally
							totalBytesRead += bytesRead;
							// Sanity check how damn big this file we're writing is becoming.. could prevent zip bomb exploits: http://en.wikipedia.org/wiki/Zip_bomb
							if (totalBytesRead >= s_MaxUnzippedFileSize) {
								LoadingSplashProgressReport("File inside zip " + std::string(zippedModuleInfo.name) + " is turning out WAY TOO LARGE - Aborting extraction!", true);
								abortExtract = true;
								break;
							}
							// Write data to the output file
							if (bytesRead > 0) {
								fwrite(fileBuffer, bytesRead, 1, outputFile);
							} else if (bytesRead < 0) {
								LoadingSplashProgressReport("Error while reading zip " + std::string(zippedModuleInfo.name), true);
								abortExtract = true;
								break;
							}
						}
						// Keep going while bytes are still being read (0 means end of file)
						while (bytesRead > 0 && outputFile);
						// Close the output file
						fclose(outputFile);
						// Close the read file within the zip archive
						unzCloseCurrentFile(zipFile);
					}
					// Go the next entry listed in the zip file.
					if ((i + 1) < zipFileInfo.number_entry) {
						if (unzGoToNextFile(zipFile) != UNZ_OK) {
							LoadingSplashProgressReport("Could not read next file inside zip " + std::string(zippedModuleInfo.name) + " - Aborting extraction!", true);
							break;
						}
					}
				}
				// Close the zip file we've opened
				unzClose(zipFile);

				LoadingSplashProgressReport("Deleting extracted Data Module zip: " + std::string(zippedModuleInfo.name), true);
				// DELETE the zip in the install directory after decompression
				std::remove(zippedModuleInfo.name);
			} else {
				// Indicate that the unzip went awry
				LoadingSplashProgressReport("FAILED to unzip " + std::string(zippedModuleInfo.name) + " - deleting it now!", true);
				// DELETE the zip in the install directory after decompression
				// (whether successful or not - any rte.zip in the install directory is throwaway and shouldn't keep failing each load in case they do fail)
				std::remove(zippedModuleInfo.name);
			}
		}
		// Close the file search to avoid memory leaks
		al_findclose(&zippedModuleInfo);
	}
}