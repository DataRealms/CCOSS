//////////////////////////////////////////////////////////////////////////////////////////
///\file    Main.cpp
///         Main driver implementation of the Retro Terrain Engine.
///\author  Copyright 2001 - 2006 Data Realms, LLC - http://www.datarealms.com
///\author  Daniel Tabar

// Without this nested includes somewhere deep inside Allegro will summon winsock.h and it will conflict with winsock2.h from RakNet
// and we can't move "Network.h" here because for whatever reasons everything will collapse
#define WIN32_LEAN_AND_MEAN

#include "RTEManagers.h"
#include "MetaMan.h"
#include "ConsoleMan.h"
#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUIProgressBar.h"
#include "GUIListBox.h"
#include "GUILabel.h"
#include "AllegroInput.h"
#include "AllegroScreen.h"
#include "AllegroBitmap.h"
#include "MainMenuGUI.h"
#include "ScenarioGUI.h"
#include "MetagameGUI.h"
#include "DataModule.h"
#include "SceneLayer.h"
#include "MOSParticle.h"
#include "MOSRotating.h"
#include "Controller.h"

#include "MultiplayerServerLobby.h"

#include "Network.h"

#include <DebugTool/DebugTool.h>
#include <Profiler/Profiler.h>

#include <algorithm>
#include <string>
#include <list>

#include "Reader.h"
#include "Writer.h"
#include "System.h"

#include <math.h>

#include <unzip.h>

#include <thread>


#if defined(__APPLE__)
#include "OsxUtil.h"
#endif // defined(__APPLE__)

#if defined(STEAM_BUILD)
#include "steam_api.h"
#include "SteamUGCMan.h"
#endif // defined (STEAM_BUILD)
// Has its own checks for steam build so we don't have to surround every achievement call
#include "AchievementMan.h"

#include "NetworkServer.h"
#include "NetworkClient.h"

#define MAX_FILENAME 512
#define FILEBUFFER_SIZE 8192
#define MAX_UNZIPPED_FILE_SIZE 104857600

#if defined(WIN32)
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
#endif

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Globals

enum TITLESEQUENCE
{
    START = 0,
    // DRL Logo
    LOGOFADEIN,
    LOGODISPLAY,
    LOGOFADEOUT,
    // Game notice
    NOTICEFADEIN,
    NOTICEDISPLAY,
    NOTICEFADEOUT,
    // Intro
    FADEIN,
    SPACEPAUSE1,
    SHOWSLIDE1,
    SHOWSLIDE2,
    SHOWSLIDE3,
    SHOWSLIDE4,
    SHOWSLIDE5,
    SHOWSLIDE6,
    SHOWSLIDE7,
    SHOWSLIDE8,
    PRETITLE,
    TITLEAPPEAR,
    PLANETSCROLL,
    PREMENU,
    MENUAPPEAR,
    // Main menu is active and operational
    MENUACTIVE,
    // Scenario mode views and transitions
    MAINTOSCENARIO,
    // Back from a scenario game to the scenario selection menu
    SCENARIOFADEIN,
    SCENARIOMENU,
    // Campaign mode views and transitions
    MAINTOCAMPAIGN,
    // Back from a battle to the campaign view
    CAMPAIGNFADEIN,
    CAMPAIGNPLAY,
    // Going back to the main menu view from a planet-centered view
    PLANETTOMAIN,
    FADESCROLLOUT,
    FADEOUT,
    END
};

// Intro slides
enum SLIDES
{
    SLIDEPAST = 0,
    SLIDENOW,
    SLIDEVR,
    SLIDETRAVEL,
    SLIDEALIENS,
    SLIDETRADE,
    SLIDEPEACE,
    SLIDEFRONTIER,
    SLIDECOUNT
};

volatile bool g_Quit = false;
bool g_InActivity = false;
bool g_ResetActivity = false;
bool g_ResumeActivity = false;
bool g_ReturnToMainMenu = false;
int g_IntroState = START;
int g_TeamCount = 2;
int g_PlayerCount = 3;
int g_DifficultySetting = 4;
int g_StationOffsetX, g_StationOffsetY;

std::string g_LoadSingleModule = "";

MainMenuGUI *g_pMainMenuGUI = 0;
ScenarioGUI *g_pScenarioGUI = 0;
GUIControlManager *g_pLoadingGUI = 0;

BITMAP * g_pLoadingGUIBitmap = 0;
int g_LoadingGUIPosX = 0;
int g_LoadingGUIPosY = 0;

Writer *g_pLoadingLogWriter = 0;
AllegroInput *g_pGUIInput = 0;
AllegroScreen *g_pGUIScreen = 0;
Controller *g_pMainMenuController = 0;

enum StarSize
{
    StarSmall = 0,
    StarLarge,
    StarHuge,
};

struct Star
{
    // Bitmap representation
    BITMAP *m_pBitmap;
    // Center locaiton on screen
    Vector m_Pos;
    // Bitmap offset
//    int m_Offset;
    // Scrolling ratio
    float m_ScrollRatio;
    // Normalized intensity 0-1.0
    float m_Intensity;
    // Type
    StarSize m_Size;

    Star() { m_pBitmap = 0; m_Pos.Reset(); m_ScrollRatio = 1.0; m_Intensity = 1.0; m_Size = StarSmall; }
    Star(BITMAP *pBitmap, Vector &pos, float scrollRatio, float intensity)
    { m_pBitmap = pBitmap; m_Pos = pos; m_ScrollRatio = scrollRatio; m_Intensity = intensity; }
};


//////////////////////////////////////////////////////////////////////////////////////////
// This handles when the quit or exit button is pressed on the window

void QuitHandler(void)
{
    g_Quit = true;
}
END_OF_FUNCTION(QuitHandler)


//////////////////////////////////////////////////////////////////////////////////////////
// This updates the loading list [DEPRECATED DUE TO HIGH OVERHEAD, BUT KEPT HERE JUST IN CASE]

void _LoadingSplashProgressReport(std::string reportString, bool newItem = false)
{
    if (g_pLoadingGUI)
    {
//        GUIProgressBar *pProgressBar = dynamic_cast<GUIProgressBar *>(g_pLoadingGUI->GetControl("ProgressBar"));
        GUIListBox *pProgressBox = dynamic_cast<GUIListBox *>(g_pLoadingGUI->GetControl("ProgressBox"));

        if (newItem || pProgressBox->GetItemList()->empty())
        {
            // Write out the last line to the log file before starting a new one
            if (g_pLoadingLogWriter->WriterOK() && !pProgressBox->GetItemList()->empty())
                *g_pLoadingLogWriter << pProgressBox->GetItemList()->back()->m_Name << "\n";

            // Add the new report line
            pProgressBox->AddItem(reportString);
        }
        else
        {
            int lastItemIndex = pProgressBox->GetItemList()->size() - 1;
            GUIListPanel::Item *pItem = pProgressBox->GetItem(lastItemIndex);
            pItem->m_Name = reportString;
            pProgressBox->SetItemValues(lastItemIndex, *pItem);
        }

        g_UInputMan.Update();
        g_pLoadingGUI->Update();
        g_pLoadingGUI->Draw();
        g_FrameMan.FlipFrameBuffers();
	
#if defined(STEAM_BUILD)
		// pump steam while loading game too
		// g_SteamUGCMan.Update();
		// Two managers use the Steam API. Probably better to just use it here directly.
		SteamAPI_RunCallbacks();
#endif // 

        // Quit if we're commanded to during loading
        if (g_Quit)
            exit(0);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// This updates the loading list. An optimizied version compared to previous one

void LoadingSplashProgressReport(std::string reportString, bool newItem = false)
{
	if (g_pLoadingGUI)
	{
		g_UInputMan.Update();
		if (newItem)
		{
			// Write out the last line to the log file before starting a new one
			if (g_pLoadingLogWriter->WriterOK())
				*g_pLoadingLogWriter << reportString << "\n";

			// Scroll bitmap upwards
			if (g_pLoadingGUIBitmap)
				blit(g_pLoadingGUIBitmap, g_pLoadingGUIBitmap, 2, 12, 2, 2, g_pLoadingGUIBitmap->w - 3, g_pLoadingGUIBitmap->h - 12);
		}
		if (g_pLoadingGUIBitmap)
		{
			AllegroBitmap bmp(g_pLoadingGUIBitmap);
			// Clear current line
			rectfill(g_pLoadingGUIBitmap, 2, g_pLoadingGUIBitmap->h - 12, g_pLoadingGUIBitmap->w - 3, g_pLoadingGUIBitmap->h - 3, 54);
			// Print new line
			g_FrameMan.GetSmallFont()->DrawAligned(&bmp, 5, g_pLoadingGUIBitmap->h - 12, reportString.c_str(), GUIFont::Left);
			// DrawAligned - MaxWidth is useless here, so we're just drawing lines manually
			vline(g_pLoadingGUIBitmap, g_pLoadingGUIBitmap->w - 2, g_pLoadingGUIBitmap->h - 12, g_pLoadingGUIBitmap->h - 2, 33);
			vline(g_pLoadingGUIBitmap, g_pLoadingGUIBitmap->w - 1, g_pLoadingGUIBitmap->h - 12, g_pLoadingGUIBitmap->h - 2, 33);

			// Draw onto current frame buffer
			blit(g_pLoadingGUIBitmap, g_FrameMan.GetBackBuffer32(), 0, 0, g_LoadingGUIPosX, g_LoadingGUIPosY, g_pLoadingGUIBitmap->w, g_pLoadingGUIBitmap->h);

			g_FrameMan.FlipFrameBuffers();
		}


#if defined(STEAM_BUILD)
		// pump steam while loading game too
		// g_SteamUGCMan.Update();
		// Two managers use the Steam API. Probably better to just use it here directly.
		SteamAPI_RunCallbacks();
#endif // 

		// Quit if we're commanded to during loading
		if (g_Quit)
			exit(0);
	}
}



//////////////////////////////////////////////////////////////////////////////////////////
// Finding and loading all DataModule:s

bool LoadDataModules()
{
// TODO: REMOVE
//    return true;

    // Loading splash screen
    g_FrameMan.ClearBackBuffer32();
//    g_FrameMan.LoadPalette("Base.rte/palette.bmp");
    SceneLayer *pLoadingSplash = new SceneLayer();
    pLoadingSplash->Create(ContentFile("Base.rte/GUIs/LoadingSplash.bmp"), false, Vector(), true, false, Vector(1.0, 0));
    // hcoded offset to make room for the loading box
    pLoadingSplash->SetOffset(Vector(((pLoadingSplash->GetBitmap()->w - g_FrameMan.GetResX()) / 2) + 110, 0));
    // Draw onto wrapped strip centered vertically on the screen
	Box splashBox(Vector(0, (g_FrameMan.GetResY() - pLoadingSplash->GetBitmap()->h) / 2), g_FrameMan.GetResX(), pLoadingSplash->GetBitmap()->h);
    pLoadingSplash->Draw(g_FrameMan.GetBackBuffer32(), splashBox);
    delete pLoadingSplash;
    pLoadingSplash = 0;

	/*int x = g_FrameMan.GetResX() / 2;
	int y = g_FrameMan.GetResY() - 50;

	AllegroBitmap bmp(g_FrameMan.GetBackBuffer32());

	g_FrameMan.GetLargeFont()->DrawAligned(&bmp, x, y, "PLANETOID PIONEERS FACT #1", 1);
	g_FrameMan.GetLargeFont()->DrawAligned(&bmp, x, y + 12, "Mods are loaded orders of magnitude faster!", 1);*/

    g_FrameMan.FlipFrameBuffers();

    // Set up the loading GUI
    if (!g_pLoadingGUI)
    {
        g_pLoadingGUI = new GUIControlManager();
// TODO: This should be using the 32bpp main menu skin, but isn't becuase it needs the config of the base for its listbox
// Can get away with this hack for now because the list box that the laoding menu uses displays ok when drawn on a 32bpp buffer,
// when it's 8bpp internally, since it does not use any masked_blit calls to draw listboxes.
// Note also how the GUIScreen passed in here has been created with an 8bpp bitmap, since that is what determines what the gui manager uses internally
        if(!g_pLoadingGUI->Create(g_pGUIScreen, g_pGUIInput, "Base.rte/GUIs/Skins/MainMenu", "LoadingSkin.ini"))
            DDTAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu/LoadingSkin.ini");
        g_pLoadingGUI->Load("Base.rte/GUIs/LoadingGUI.ini");
    }
    // Place and clear the sectionProgress box
    dynamic_cast<GUICollectionBox *>(g_pLoadingGUI->GetControl("root"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
    GUIListBox *pBox = dynamic_cast<GUIListBox *>(g_pLoadingGUI->GetControl("ProgressBox"));
    // Make the box a bit bigger if there's room in higher, HD resolutions
    if (g_FrameMan.GetResX() >= 960)
    {
        // Make the loading progress box fill the right third of the screen
        pBox->Resize((g_FrameMan.GetResX() / 3) - 12, pBox->GetHeight());
        pBox->SetPositionRel(g_FrameMan.GetResX() - pBox->GetWidth() - 12, (g_FrameMan.GetResY() / 2) - (pBox->GetHeight() / 2));
    }
    // Legacy positioning and sizing when running low resolutions
    else
        pBox->SetPositionRel(g_FrameMan.GetResX() - pBox->GetWidth() - 12, (g_FrameMan.GetResY() / 2) - (pBox->GetHeight() / 2));
    pBox->ClearList();

    // Show and position the registration notice label
    GUILabel *pRegLabel = dynamic_cast<GUILabel *>(g_pLoadingGUI->GetControl("LabelRegisterNotice"));
    if (!g_LicenseMan.HasValidatedLicense())
    {
        // Make room for the notice over the list
        pBox->SetSize(pBox->GetWidth(), 258);
        pBox->SetPositionRel(pBox->GetXPos(), pBox->GetYPos() + pRegLabel->GetHeight());
        // Show the notice label
        pRegLabel->SetPositionRel(pBox->GetXPos(), pBox->GetYPos() - pRegLabel->GetHeight() - 6);
        pRegLabel->SetText("Unregistered Copy");
        pRegLabel->SetVisible(true);
    }
    else
        pRegLabel->SetVisible(false);

	if (!g_SettingsMan.DisableLoadingScreen())
	{
		//New mechanism to speed up loading times as it turned out that a massive amount of time is spent
		// to update UI control.
		if (!g_pLoadingGUIBitmap)
		{
			pBox->SetVisible(false);
			g_pLoadingGUIBitmap = create_bitmap_ex(8, pBox->GetWidth(), pBox->GetHeight());
			clear_to_color(g_pLoadingGUIBitmap, 54);
			rect(g_pLoadingGUIBitmap, 0, 0, pBox->GetWidth() - 1, pBox->GetHeight() - 1, 33);
			rect(g_pLoadingGUIBitmap, 1, 1, pBox->GetWidth() - 2, pBox->GetHeight() - 2, 33);
			g_LoadingGUIPosX = pBox->GetXPos();
			g_LoadingGUIPosY = pBox->GetYPos();
		}
	}

    // Create the loading log writer
    if (!g_pLoadingLogWriter)
        g_pLoadingLogWriter = new Writer("LogLoading.txt");

    // Clear out the PresetMan and all its DataModules
    g_PresetMan.Destroy();
    g_PresetMan.Create();

#if defined(STEAM_BUILD)
    // Download all subscribed-to data module files from Steam and copy them all into the game install dir
    // so they can be unzipped next and loaded
    g_SteamUGCMan.DownloadAllWorkshopDataModules(&LoadingSplashProgressReport);
#endif // STEAM_BUILD

    // Unzip all *.rte.zip files found in the install dir, overwriting all files already existing
    // This will cause extracted and available data modules to be updated to whatever is within their corresponding zip files
    // The point of this is that it facilitates downloaded mods being loaded without having to be manually unzipped first by the user
    al_ffblk zippedModuleInfo;
    unzFile zipFile;
    for (int result = al_findfirst("*.rte.zip", &zippedModuleInfo, FA_ALL); result == 0; result = al_findnext(&zippedModuleInfo))
    {
        // Report that we are attempting to unzip this thing
        LoadingSplashProgressReport("Unzipping " + string(zippedModuleInfo.name), true);
// THIS IS WRONG - rely on the working directory instead; this hard method will fail when the exe is not in the install dir like when running in visual studio
/*
        // Get the absolute path to the zip, which lies next to the game exe in the same dir
        get_executable_name(zipFilePath, sizeof(zipFilePath));
        // Replace the exe filename with the zip one we found while enumerating all rte.zip files
        replace_filename(zipFilePath, zipFilePath, zippedModuleInfo.name, sizeof(zipFilePath));
*/

        // Try to open the zipped and unzip it into place as an exposed data module
        if (strlen(zippedModuleInfo.name) > 0 && (zipFile = unzOpen(zippedModuleInfo.name)))
        {
            // Go through and extract every file inside this zip, overwriting every colliding file that already exists in the install dir 

            // Get info about the zip file
            unz_global_info zipFileInfo;
            if (unzGetGlobalInfo(zipFile, &zipFileInfo) != UNZ_OK)
                LoadingSplashProgressReport("Could not read global file info of: " + string(zippedModuleInfo.name), true);

            // Buffer to hold data read from the zip file.
            char fileBuffer[FILEBUFFER_SIZE];

            // Loop to extract all files
            bool abortExtract = false;
            for (uLong i = 0; i < zipFileInfo.number_entry && !abortExtract; ++i)
            {
                // Get info about current file.
                unz_file_info fileInfo;
                char outputFileName[MAX_FILENAME];
                if (unzGetCurrentFileInfo(zipFile, &fileInfo, outputFileName, MAX_FILENAME, NULL, 0, NULL, 0) != UNZ_OK)
                    LoadingSplashProgressReport("Could not read file info of: " + string(outputFileName), true);

                // Check if the directory we are trying to extract into exists, and if not, create it
                char outputDirName[MAX_FILENAME];
                char parentDirName[MAX_FILENAME];
                // Copy the file path to a separate dir path
                strcpy(outputDirName, outputFileName);
                // Find the last slash in the dir path, so we can cut off everything after that (ie the actual filename), and only have the directory path left
                char *pSlashPos = strrchr(outputDirName, '/');
                // Try to find the other kind of slash if we found none
                if (!pSlashPos)
                    pSlashPos = strrchr(outputDirName, '\\');
                // Now that we have the slash position, terminate the directory path string right after there
                if (pSlashPos)
                    *(++pSlashPos) = 0;

                // If that file's directory doesn't exist yet, then create it, and all its parent directories above if need be
                for (int nested = 0; !file_exists(outputDirName, FA_DIREC, 0) && pSlashPos; ++nested)
                {
                    // Keep making new working copies of the path that we can dice up
                    strcpy(parentDirName, outputDirName[0] == '.' ? &(outputDirName[2]) : outputDirName);
                    // Start off at the beginning
                    pSlashPos = parentDirName;
                    for (int j = 0; j <= nested && pSlashPos; ++j)
                    {
                        // Find the first slash so we can isolate the folders in the hierarchy, in descending seniority
                        pSlashPos = strchr(pSlashPos, '/');
                        // If we can't find any more slashes, then quit
                        if (!pSlashPos)
                            break;
                        // If we did find a slash, go to one past it slash and try to find the next one
                        pSlashPos++;
                    }
                    // No more nested folders to make
                    if (!pSlashPos)
                        break;
                    // Terminate there so we are making the most senior folder
                    *(pSlashPos) = 0;
                    g_System.MakeDirectory(parentDirName);
                }

                // Check if this entry is a directory or file
                if (outputFileName[strlen(outputFileName) - 1] == '/' || outputFileName[strlen(outputFileName) - 1] == '\\')
                {
                    // Entry is a directory, so create it.
                    LoadingSplashProgressReport("Creating Dir: " + string(outputFileName), true);
                    g_System.MakeDirectory(outputFileName);
                }
                else
                // So it's a file
                {
                    // Validate so only certain filetypes are extracted:  .ini .txt .lua .cfg .bmp .png .jpg .jpeg .wav .ogg .mp3
                    // Get the file extension
                    string extension(get_extension(outputFileName));
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                    const char *ext = extension.c_str();
                    // Validate only certain filetypes to be included! .ini .txt .lua .cfg .bmp .png .jpg .jpeg .wav .ogg .mp3
                    if (!(strcmp(ext, "ini") == 0 || strcmp(ext, "txt") == 0 || strcmp(ext, "lua") == 0 || strcmp(ext, "cfg") == 0 ||
                          strcmp(ext, "bmp") == 0 || strcmp(ext, "png") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0 || 
                          strcmp(ext, "wav") == 0 || strcmp(ext, "ogg") == 0 || strcmp(ext, "mp3") == 0 ||
                          strcmp(ext, "xlsx") == 0 || strcmp(ext, "rtf") == 0 || strcmp(ext, "dat") == 0)) 
                    {
                        LoadingSplashProgressReport("Skipping: " + string(outputFileName) + " - bad extension!", true);

                        // Keep going though!!
                        // Close the read file within the zip archive
                        unzCloseCurrentFile(zipFile);
                        // Go the the next entry listed in the zip file.
                        if ((i + 1) < zipFileInfo.number_entry)
                        {
                            if (unzGoToNextFile(zipFile) != UNZ_OK)
                            {
                                LoadingSplashProgressReport("Could not read next file inside zip " + string(zippedModuleInfo.name) +  " - Aborting extraction!", true);
                                abortExtract = true;
                                break;
                            }
                        }
                        // Onto the next file
                        continue;
                    }

                    // Entry is a file, so extract it.
                    LoadingSplashProgressReport("Extracting: " + string(outputFileName), true);
                    if (unzOpenCurrentFile(zipFile) != UNZ_OK)
                        LoadingSplashProgressReport("Could not open file within " + string(zippedModuleInfo.name), true);

                    // Open a file to write out the data.
                    FILE *outputFile = fopen(outputFileName, "wb");
                    if (outputFile == NULL)
                        LoadingSplashProgressReport("Could not open/create destination file while unzipping " + string(zippedModuleInfo.name), true);

                    // Write the entire file out, reading in buffer size chunks and spitting them out to the output stream
                    int bytesRead = 0;
                    int64_t totalBytesRead = 0;
                    do
                    {
                        // Read a chunk
                        bytesRead = unzReadCurrentFile(zipFile, fileBuffer, FILEBUFFER_SIZE);
                        // Add to total tally
                        totalBytesRead += bytesRead;

                        // Sanity check how damn big this file we're writing is becoming.. could prevent zip bomb exploits: http://en.wikipedia.org/wiki/Zip_bomb
                        if (totalBytesRead >= MAX_UNZIPPED_FILE_SIZE)
                        {
                            LoadingSplashProgressReport("File inside zip " + string(zippedModuleInfo.name) +  " is turning out WAY TOO LARGE - Aborting extraction!", true);
                            abortExtract = true;
                            break;
                        }

                        // Write data to the output file
                        if (bytesRead > 0)
                            fwrite(fileBuffer, bytesRead, 1, outputFile);
                        else if (bytesRead < 0)
                        {
                            LoadingSplashProgressReport("Error while reading zip " + string(zippedModuleInfo.name), true);
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

                // Go the the next entry listed in the zip file.
                if ((i + 1) < zipFileInfo.number_entry)
                {
                    if (unzGoToNextFile(zipFile) != UNZ_OK)
                    {
                        LoadingSplashProgressReport("Could not read next file inside zip " + string(zippedModuleInfo.name) +  " - Aborting extraction!", true);
                        break;
                    }
                }
            }

            // Close the zip file we've opened
            unzClose(zipFile);

            // DELETE the zip in the install dir after decompression
            // (whether successful or not - any rte.zip in the install dir is throwaway and shouldn't keep failing each load in case they do fail)
            LoadingSplashProgressReport("Deleting extracted Data Module zip: " + string(zippedModuleInfo.name), true);
            delete_file(zippedModuleInfo.name);
        }
        // Indicate that the unzip went awry
        else
        {
            // DELETE the zip in the install dir after decompression
            // (whether successful or not - any rte.zip in the install dir is throwaway and shouldn't keep failing each load in case they do fail)
            LoadingSplashProgressReport("FAILED to unzip " + string(zippedModuleInfo.name) + " - deleting it now!", true);
            delete_file(zippedModuleInfo.name);
        }
    }
    // Close the file search to avoid memory leaks
    al_findclose(&zippedModuleInfo);

    ///////////////////////////////////////////////////////////////
    // Load all the official modules first!

    if (!g_PresetMan.LoadDataModule("Base.rte", true, &LoadingSplashProgressReport))
        return false;

	if (g_LoadSingleModule != "")
	{
		if (g_LoadSingleModule != "Base.rte")
			if (!g_PresetMan.LoadDataModule(g_LoadSingleModule, false, &LoadingSplashProgressReport))
				return false;
		return true;
	}

///* TODO: REPLACE
    if (!g_PresetMan.LoadDataModule("Coalition.rte", true, &LoadingSplashProgressReport))
        return false;
    if (!g_PresetMan.LoadDataModule("Techion.rte", true, &LoadingSplashProgressReport))
        return false;
    if (!g_PresetMan.LoadDataModule("Imperatus.rte", true, &LoadingSplashProgressReport))
        return false;
    if (!g_PresetMan.LoadDataModule("Ronin.rte", true, &LoadingSplashProgressReport))
        return false;
    if (!g_PresetMan.LoadDataModule("Dummy.rte", true, &LoadingSplashProgressReport))
        return false;
    if (!g_PresetMan.LoadDataModule("Browncoats.rte", true, &LoadingSplashProgressReport))
        return false;
    if (!g_PresetMan.LoadDataModule("Tutorial.rte", true, &LoadingSplashProgressReport))
        return false;
    if (!g_PresetMan.LoadDataModule("Missions.rte", true, &LoadingSplashProgressReport))
        return false;

	//Read module properties to find out which modules should be loaded earlier than others
    al_ffblk moduleInfo;
	int moduleID = 0;

	std::list<std::string> loadFirst;

    for (int result = al_findfirst("*.rte", &moduleInfo, FA_DIREC | FA_RDONLY); result == 0; result = al_findnext(&moduleInfo))
    {
        moduleID = g_PresetMan.GetModuleID(moduleInfo.name);
        // Make sure we don't load properties of already loaded official modules
        if (strlen(moduleInfo.name) > 0 && (moduleID < 0 || moduleID >= g_PresetMan.GetOfficialModuleCount()) && string(moduleInfo.name) != "Metagames.rte" && string(moduleInfo.name) != "Scenes.rte")
        {
            // See if we can find that phantom property in this data module's index.ini that would indicate it should have prioritized loading
			if (ASCIIFileContainsString(string(moduleInfo.name) + "/Index.ini", "LoadFirst = 1"))
				loadFirst.push_back(moduleInfo.name);
        }
        else
        {
        }
    }
    // Close the file search to avoid memory leaks
    al_findclose(&moduleInfo);

	//Load preceding modules first
	for (std::list<std::string>::iterator itr = loadFirst.begin(); itr != loadFirst.end(); ++itr)
	{
		if (!g_SettingsMan.IsModDisabled(*itr))
		{
			moduleID = g_PresetMan.GetModuleID(*itr);
			// Make sure we don't add the official metagames module among these; they should be loaded in explicit order before and after these unofficial ones
			if ((*itr).length() > 0 && (moduleID < 0 || moduleID >= g_PresetMan.GetOfficialModuleCount()) && *itr != "Metagames.rte" && *itr != "Scenes.rte")
			{
				// Actually load the unofficial data module
				if (!g_PresetMan.LoadDataModule(*itr, false, &LoadingSplashProgressReport))
				{
					// Report error?
				}
			}
		}
	}

	loadFirst.clear();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Search for any additional data modules, if license is registered! - NAH let them load mods

//    if (g_LicenseMan.HasValidatedLicense())
//    {
        moduleID = 0;
        for (int result = al_findfirst("*.rte", &moduleInfo, FA_DIREC | FA_RDONLY); result == 0; result = al_findnext(&moduleInfo))
        {
			if (!g_SettingsMan.IsModDisabled(moduleInfo.name))
			{
				moduleID = g_PresetMan.GetModuleID(moduleInfo.name);
				// Make sure we don't add the official metagames module among these; they should be loaded in explicit order before and after these unofficial ones
				if (strlen(moduleInfo.name) > 0 && (moduleID < 0 || moduleID >= g_PresetMan.GetOfficialModuleCount()) && string(moduleInfo.name) != "Metagames.rte" && string(moduleInfo.name) != "Scenes.rte")
				{
	/* Redundant with weegee's other ssytem that allows sideloading etc
					// If workshop is enabled, then SKIP loading any unofficial mods that are neither subscribed-to nor published by this user
					if (g_SteamUGCMan.IsCloudEnabled() && (!g_SteamUGCMan.IsModuleSubscribedTo(moduleInfo.name) && !g_SteamUGCMan.IsModulePublished(moduleInfo.name)))
					{
						LoadingSplashProgressReport("NOT Loading Data Module: " + string(moduleInfo.name) + " - it is not subscribed to in the Workshop!", true);
						continue;
					}
	*/              
					// Actually load the unofficial data module
					if (!g_PresetMan.LoadDataModule(string(moduleInfo.name), false, &LoadingSplashProgressReport))
					{
						// Report error?
					}
				}
				else
				{
		// TODO: Log this and continue gracefully instead
	// LoadDataModule can return false (esp since it amy try to load already loaded modules, and that's ok) and shouldn't cause stop
	//                char error[512];
	//                sprintf(error, "Failed to load Data Module: %s\n\nMake sure it contains an Index.ini file that defines a \"DataModule\"!", moduleInfo.name);
	//                DDTAbort(error);
	//                return false;

				}
			}
        }
        // Close the file search to avoid memory leaks
        al_findclose(&moduleInfo);
//    }
//*/

    // Load scenes and metagames AFTER all other techs etc are loaded; might be referring to stuff in user mods
    if (!g_PresetMan.LoadDataModule("Scenes.rte", false, &LoadingSplashProgressReport))
        return false;

    if (!g_PresetMan.LoadDataModule("Metagames.rte", false, &LoadingSplashProgressReport))
        return false;


/* We are now doing this as line by line reports come in to LoadingSplashProgressReport
    // Write out entire loading log to a file
    Writer writer("LogLoading.txt");
    if (writer.WriterOK())
    {
        GUIListBox *pProgressBox = dynamic_cast<GUIListBox *>(g_pLoadingGUI->GetControl("ProgressBox"));
        for (std::vector<GUIListBox::Item *>::iterator itr = pProgressBox->GetItemList()->begin(); itr != pProgressBox->GetItemList()->end(); itr++)
            writer << (*itr)->m_Name << "\n";
    }
*/
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Load and init the Main menu

bool InitMainMenu()
{
    // Load the palette
    g_FrameMan.LoadPalette("Base.rte/palette.bmp");

    // Create the Main Menu GUI
    g_pGUIInput = new AllegroInput(-1);
    g_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32());

    // Have to load the data modules in here becuase it needs the GUIScreen and input for the loading GUI
    LoadDataModules();

    // Create the main menu interface
	g_pMainMenuGUI = new MainMenuGUI();
    g_pMainMenuController = new Controller(Controller::CIM_PLAYER, 0);
    g_pMainMenuController->SetTeam(0);
    g_pMainMenuGUI->Create(g_pMainMenuController);
    // As well as the Scenario setup menu interface
	g_pScenarioGUI = new ScenarioGUI();
    g_pScenarioGUI->Create(g_pMainMenuController);
    // And the Metagame GUI too
    g_MetaMan.GetGUI()->Create(g_pMainMenuController);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Load all Managers

bool ResetActivity()
{
    g_ResetActivity = false;

    // Clear and reset out things
    g_FrameMan.ClearBackBuffer8();
    g_FrameMan.FlipFrameBuffers();
    g_AudioMan.StopAll();

    // Quit if we should
    if (g_Quit)
        return false;

// TODO: Deal with GUI resetting here!$@#")

    // Clear out all MO's
    g_MovableMan.PurgeAllMOs();
    // Have to reset TimerMan before creating anything else because all timers are reset against it
    g_TimerMan.ResetTime();
/*
    // Load all the modules anew
    LoadDataModules();

//    g_PresetMan.Create();
//    g_AudioMan.Create();

    char report[512];
    sprintf(report, "Building Scene: \"%s\"...", g_ActivityMan.GetActivity()->GetSceneName().c_str());
    LoadingSplashProgressReport(report, true);

    g_SceneMan.LoadScene(g_ActivityMan.GetActivity()->GetSceneName());

    sprintf(report, "\tDone! %c", -42);
    LoadingSplashProgressReport(report, true);
    LoadingSplashProgressReport(" ", true);

    // Ask user to press key before start
    LoadingSplashProgressReport("PRESS ANY KEY TO START!", true);
    Timer blinkTimer;
    do
    {
        sprintf(report, "PRESS ANY KEY TO START! %c", blinkTimer.AlternateSim(300) ? -65 : ' ');
        LoadingSplashProgressReport(report, false);

        // Reset the key press states
        g_UInputMan.Update();
        g_TimerMan.Update();
        rest(30);
    }
    while (!g_UInputMan.AnyPress() && !g_Quit);

*/
    g_FrameMan.LoadPalette("Base.rte/palette.bmp");
    g_FrameMan.FlipFrameBuffers();

    // Reset timerman again after loading so there's no residual delay
    g_TimerMan.ResetTime();
    // Enable time averaging since it helps with animation jerkyness
    g_TimerMan.EnableAveraging(true);
    // Unpause
    g_TimerMan.PauseSim(false);

// TODO: Remove
//    g_ActivityMan.GetActivity()->SetActivityState(Activity::TESTING);
    // Start the game with previous settings
    int error = g_ActivityMan.RestartActivity();

    if (error >= 0)
        g_InActivity = true;
    // Somehting went wrong when restarting, so drop out to scenario menu and open the console to show the error messages
    else
    {
        g_InActivity = false;
        g_ActivityMan.PauseActivity();
        g_ConsoleMan.SetEnabled(true);
        g_IntroState = MAINTOSCENARIO;
        return false;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Start the simulation back up after being paused

void ResumeActivity()
{
    if (g_ActivityMan.GetActivity()->GetActivityState() != Activity::NOTSTARTED)
    {
        g_Quit = false;
        g_InActivity = true;
        g_ResumeActivity = false;

        g_FrameMan.ClearBackBuffer8();
        g_FrameMan.FlipFrameBuffers();
        // Load in-game palette
        g_FrameMan.LoadPalette("Base.rte/palette.bmp");

        // Unpause the game
        g_FrameMan.ResetFrameTimer();
        // Enable time averaging since it helps with animation jerkyness
        g_TimerMan.EnableAveraging(true);
        // Unpause the sim
        g_TimerMan.PauseSim(false);
        g_ActivityMan.PauseActivity(false);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Launch multiplayer lobby activity

void EnterMultiplayerLobby()
{
	//g_ActivityMan.EndActivity();

	// Start multiplayer lobby
	g_SceneMan.SetSceneToLoad("Multiplayer Scene");
	MultiplayerServerLobby *pMultiplayerServerLobby = new MultiplayerServerLobby;
	pMultiplayerServerLobby->Create();

	pMultiplayerServerLobby->ClearPlayers(true);
	pMultiplayerServerLobby->AddPlayer(0, true, 0, 0);
	pMultiplayerServerLobby->AddPlayer(1, true, 0, 1);
	pMultiplayerServerLobby->AddPlayer(2, true, 0, 2);
	pMultiplayerServerLobby->AddPlayer(3, true, 0, 3);

	//g_FrameMan.ResetSplitScreens(true, true);
	g_ActivityMan.SetStartActivity(pMultiplayerServerLobby);
	g_ResetActivity = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Load and display the into, title and menu sequence

bool PlayIntroTitle()
{
    // Disable time averaging since it can make the music timing creep off target.
    g_TimerMan.EnableAveraging(false);
    
    // Untrap the mouse and keyboard
    g_UInputMan.DisableKeys(false);
    g_UInputMan.TrapMousePos(false);

    // Stop all audio
    g_AudioMan.StopAll();

    g_FrameMan.ClearBackBuffer32();
    g_FrameMan.FlipFrameBuffers();
    int resX = g_FrameMan.GetResX();
    int resY = g_FrameMan.GetResY();

    // The fadein/out screens
    BITMAP *pFadeScreen = create_bitmap_ex(32, resX, resY);
    clear_to_color(pFadeScreen, 0);
    int fadePos = 0;

    // Load the Intro slides
    BITMAP **apIntroSlides = new BITMAP *[SLIDECOUNT];
    ContentFile introSlideFile("Base.rte/Title/IntroSlideA.bmp");
    apIntroSlides[SLIDEPAST] = introSlideFile.LoadAndReleaseBitmap();
    introSlideFile.SetDataPath("Base.rte/Title/IntroSlideB.bmp");
    apIntroSlides[SLIDENOW] = introSlideFile.LoadAndReleaseBitmap();
    introSlideFile.SetDataPath("Base.rte/Title/IntroSlideC.bmp");
    apIntroSlides[SLIDEVR] = introSlideFile.LoadAndReleaseBitmap();
    introSlideFile.SetDataPath("Base.rte/Title/IntroSlideD.bmp");
    apIntroSlides[SLIDETRAVEL] = introSlideFile.LoadAndReleaseBitmap();
    introSlideFile.SetDataPath("Base.rte/Title/IntroSlideE.bmp");
    apIntroSlides[SLIDEALIENS] = introSlideFile.LoadAndReleaseBitmap();
    introSlideFile.SetDataPath("Base.rte/Title/IntroSlideF.bmp");
    apIntroSlides[SLIDETRADE] = introSlideFile.LoadAndReleaseBitmap();
    introSlideFile.SetDataPath("Base.rte/Title/IntroSlideG.bmp");
    apIntroSlides[SLIDEPEACE] = introSlideFile.LoadAndReleaseBitmap();
    introSlideFile.SetDataPath("Base.rte/Title/IntroSlideH.bmp");
    apIntroSlides[SLIDEFRONTIER] = introSlideFile.LoadAndReleaseBitmap();
    int normalSlideWidth = 640;
    int slideFadeDistance = 42;

    ContentFile alphaFile;
    BITMAP *pAlpha = 0;

    MOSParticle *pDRLogo = new MOSParticle();
    pDRLogo->Create(ContentFile("Base.rte/Title/DRLogo5x.bmp"));
    pDRLogo->SetWrapDoubleDrawing(false);

    SceneLayer *pBackdrop = new SceneLayer();
    pBackdrop->Create(ContentFile("Base.rte/Title/Nebula.bmp"), false, Vector(), false, false, Vector(0, -1.0));//startYOffset + resY));
    float backdropScrollRatio = 1.0f / 3.0f;

    MOSParticle *pTitle = new MOSParticle();
    pTitle->Create(ContentFile("Base.rte/Title/Title.bmp"));
    pTitle->SetWrapDoubleDrawing(false);
    // Logo glow effect
    MOSParticle *pTitleGlow = new MOSParticle();
    pTitleGlow->Create(ContentFile("Base.rte/Title/TitleGlow.bmp"));
    pTitleGlow->SetWrapDoubleDrawing(false);
    // Add alpha
    alphaFile.SetDataPath("Base.rte/Title/TitleAlpha.bmp");
    set_write_alpha_blender();
    draw_trans_sprite(pTitle->GetSpriteFrame(0), alphaFile.GetAsBitmap(), 0, 0);

    MOSParticle *pPlanet = new MOSParticle();
    pPlanet->Create(ContentFile("Base.rte/Title/Planet.bmp"));
    pPlanet->SetWrapDoubleDrawing(false);
    // Add alpha
    alphaFile.SetDataPath("Base.rte/Title/PlanetAlpha.bmp");
    set_write_alpha_blender();
    draw_trans_sprite(pPlanet->GetSpriteFrame(0), alphaFile.GetAsBitmap(), 0, 0);

    MOSParticle *pMoon = new MOSParticle();
    pMoon->Create(ContentFile("Base.rte/Title/Moon.bmp"));
    pMoon->SetWrapDoubleDrawing(false);
    // Add alpha
    alphaFile.SetDataPath("Base.rte/Title/MoonAlpha.bmp");
    set_write_alpha_blender();
    draw_trans_sprite(pMoon->GetSpriteFrame(0), alphaFile.GetAsBitmap(), 0, 0);

    MOSRotating *pStation = new MOSRotating();
    pStation->Create(ContentFile("Base.rte/Title/Station.bmp"));
    pStation->SetWrapDoubleDrawing(false);

	MOSRotating *pPioneerCapsule = new MOSRotating();
	pPioneerCapsule->Create(ContentFile("Base.rte/Title/PioneerCapsule.bmp"));
	pPioneerCapsule->SetWrapDoubleDrawing(false);

	MOSRotating *pPioneerScreaming = new MOSRotating();
	pPioneerScreaming->Create(ContentFile("Base.rte/Title/PioneerScreaming.bmp"));
	pPioneerScreaming->SetWrapDoubleDrawing(false);

	MOSRotating *pPioneerPromo = new MOSRotating();
	pPioneerPromo->Create(ContentFile("Base.rte/Title/PioneerPromo.bmp"));
	pPioneerPromo->SetWrapDoubleDrawing(false);

	MOSParticle * pFirePuffLarge = dynamic_cast<MOSParticle *>(g_PresetMan.GetEntityPreset("MOSParticle", "Fire Puff Large", "Base.rte")->Clone());
	MOSParticle * pFirePuffMedium = dynamic_cast<MOSParticle *>(g_PresetMan.GetEntityPreset("MOSParticle", "Fire Puff Medium", "Base.rte")->Clone());

	long long lastShake = 0;
	long long lastPuffFrame = 0;
	long long lastPuff = 0;
	bool puffActive = false;
	int puffFrame = 0;
	int puffCount = 0;

	Vector shakeOffset(0, 0);

	bool pioneerPhase = false;

    // Generate stars!
    int starArea = resX * pBackdrop->GetBitmap()->h;
    int starCount = starArea / 1000;
    ContentFile starSmallFile("Base.rte/Title/Stars/StarSmall.bmp");
    ContentFile starLargeFile("Base.rte/Title/Stars/StarLarge.bmp");
    ContentFile starHugeFile("Base.rte/Title/Stars/StarHuge.bmp");
    int starSmallBitmapCount = 4;
    int starLargeBitmapCount = 1;
    int starHugeBitmapCount = 2;
    BITMAP **apStarSmallBitmaps = starSmallFile.GetAsAnimation(starSmallBitmapCount);
    BITMAP **apStarLargeBitmaps = starLargeFile.GetAsAnimation(starLargeBitmapCount);
    BITMAP **apStarHugeBitmaps = starHugeFile.GetAsAnimation(starHugeBitmapCount);
    Star *aStars = new Star[starCount];
    StarSize size;

    for (int star = 0; star < starCount; ++star)
    {
        aStars[star].m_Size = size = PosRand() < 0.95 ? StarSmall : (PosRand() < 0.85 ? StarLarge : StarHuge);
        aStars[star].m_pBitmap = size  == StarSmall ? apStarSmallBitmaps[SelectRand(0, starSmallBitmapCount - 1)] :
                                (size  == StarLarge ? apStarLargeBitmaps[SelectRand(0, starLargeBitmapCount - 1)] : apStarHugeBitmaps[SelectRand(0, starLargeBitmapCount - 1)]);
        aStars[star].m_Pos.SetXY(resX * PosRand(), pBackdrop->GetBitmap()->h * PosRand());//resY * PosRand());
        aStars[star].m_Pos.Floor();
        // To match the nebula scroll
        aStars[star].m_ScrollRatio = backdropScrollRatio;
        aStars[star].m_Intensity = size == StarSmall ? RangeRand(0.001, 0.5) : (size == StarLarge ? RangeRand(0.6, 1.0) : RangeRand(0.9, 1.0));
    }

    // Font stuff
    GUISkin *pSkin = g_pMainMenuGUI->GetGUIControlManager()->GetSkin();
    GUIFont *pFont = pSkin->GetFont("fatfont.bmp");
    AllegroBitmap backBuffer(g_FrameMan.GetBackBuffer32());
    int yTextPos = 0;
    // Timers
    Timer totalTimer, songTimer, sectionTimer;
    // Convenience for how many seconds have elapsed on each section
    double elapsed = 0;
    // How long each section is, in s
    double duration = 0, scrollDuration = 0, scrollStart = 0, slideFadeInDuration = 0.5, slideFadeOutDuration = 0.5;
    // Progress made on a section, from 0.0 to 1.0
    double sectionProgress = 0, scrollProgress = 0;
    // When a section is supposed to end, relative to the songtimer
    long sectionSongEnd = 0;

    // Scrolling data
    bool keyPressed = false, sectionSwitch = true;
    float planetRadius = 240;
    float orbitRadius = 274;
    float orbitRotation = HalfPI - EigthPI;
    // Set the start so that the nebula is fully scolled up
    int startYOffset = pBackdrop->GetBitmap()->h / backdropScrollRatio - (resY / backdropScrollRatio);
    int titleAppearYOffset = 900;
    int preMenuYOffset = 100;
    int topMenuYOffset = 0;
    // So planet is centered on the screen regardless of resolution
    int planetViewYOffset = 325 + planetRadius - (resY / 2);
    // Set Y to title offset so there's no jump when entering the main menu
    Vector scrollOffset(0, preMenuYOffset), planetPos, stationOffset, capsuleOffset, slidePos;

    totalTimer.Reset();
    sectionTimer.Reset();
    while (!g_Quit && g_IntroState != END && !g_ResumeActivity)
    {
        keyPressed = g_UInputMan.AnyStartPress();
//        g_Quit = key[KEY_ESC];
        // Reset the key press states
        g_UInputMan.Update();
        g_TimerMan.Update();
        g_TimerMan.UpdateSim();
        g_ConsoleMan.Update();

#if __USE_SOUND_GORILLA
		g_FrameMan.StartPerformanceMeasurement(FrameMan::PERF_SOUND);
		g_AudioMan.Update();
		g_FrameMan.StopPerformanceMeasurement(FrameMan::PERF_SOUND);
#endif

        if (sectionSwitch)
            sectionTimer.Reset();
        elapsed = sectionTimer.GetElapsedRealTimeS();
        // Calculate the normalized sectionProgress scalar
        if (duration > 0)
            sectionProgress = elapsed / duration;
        else
            sectionProgress = 0;
        // Clamp the sectionProgress scalar
        if (sectionProgress > 0.9999)
            sectionProgress = 0.9999;

		if (g_NetworkServer.IsServerModeEnabled())
			g_NetworkServer.Update();

        ////////////////////////////////
        // Scrolling logic

        if (g_IntroState >= FADEIN && g_IntroState <= PRETITLE)
        {
            if (g_IntroState == FADEIN && sectionSwitch)
            {
                songTimer.SetElapsedRealTimeS(0.05);
                scrollStart = songTimer.GetElapsedRealTimeS();
                // 66.6s This is the end of PRETITLE
                scrollDuration = 66.6 - scrollStart;
            }
            scrollProgress = (double)(songTimer.GetElapsedRealTimeS() - scrollStart) / (double)scrollDuration;
            scrollOffset.m_Y = LERP(0, 1.0, startYOffset, titleAppearYOffset,  scrollProgress);
        }
        // Scroll after the slideshow
        else if (g_IntroState >= TITLEAPPEAR && g_IntroState <= PLANETSCROLL)
        {
            if (g_IntroState == TITLEAPPEAR && sectionSwitch)
            {
                scrollStart = songTimer.GetElapsedRealTimeS();
                // This is the end of PLANETSCROLL
                scrollDuration = 92.4 - scrollStart;
            }
            scrollProgress = (double)(songTimer.GetElapsedRealTimeS() - scrollStart) / (double)scrollDuration;
//            scrollOffset.m_Y = LERP(scrollStart, 92.4, titleAppearYOffset, preMenuYOffset, songTimer.GetElapsedRealTimeS());
            scrollOffset.m_Y = EaseOut(titleAppearYOffset, preMenuYOffset, scrollProgress);
        }
        // Scroll the last bit to reveal the menu appears
        else if (g_IntroState == MENUAPPEAR)
        {
            scrollOffset.m_Y = EaseOut(preMenuYOffset, topMenuYOffset, sectionProgress);
        }
        // Scroll down to the planet screen
        else if (g_IntroState == MAINTOSCENARIO || g_IntroState == MAINTOCAMPAIGN)
        {
            scrollOffset.m_Y = EaseOut(topMenuYOffset, planetViewYOffset, sectionProgress);
        }
        // Scroll back up to the main screen from campaign
        else if (g_IntroState == PLANETTOMAIN)
        {
            scrollOffset.m_Y = EaseOut(planetViewYOffset, topMenuYOffset, sectionProgress);
        }

        ///////////////////////////////////////////////////////
        // DRL Logo drawing

        if (g_IntroState >= LOGOFADEIN && g_IntroState <= LOGOFADEOUT)
        {
            // Draw the early build notice
            g_FrameMan.ClearBackBuffer32();
            pDRLogo->SetPos(Vector(g_FrameMan.GetResX() / 2, (g_FrameMan.GetResY() / 2) - 35));
            pDRLogo->Draw(g_FrameMan.GetBackBuffer32());
        }

        ///////////////////////////////////////////////////////
        // Notice drawing

        if (g_IntroState >= NOTICEFADEIN && g_IntroState <= NOTICEFADEOUT)
        {
            // Draw the early build notice
            g_FrameMan.ClearBackBuffer32();
            yTextPos = g_FrameMan.GetResY() / 3;
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, string("N O T E :"), GUIFont::Centre);
            yTextPos += pFont->GetFontHeight() * 2;
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, string("This game plays great with up to FOUR people on a BIG-SCREEN TV!"), GUIFont::Centre);
            yTextPos += pFont->GetFontHeight() * 2;
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, string("but it also plays great with up to four people on a BIG SCREEN TV!"), GUIFont::Centre);
//            yTextPos += pFont->GetFontHeight() * 1;
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, string("So invite some friends/enemies over, plug in those USB controllers, and have a blast -"), GUIFont::Centre);
            yTextPos += pFont->GetFontHeight() * 4;
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, string("Press ALT+ENTER to toggle FULLSCREEN mode"), GUIFont::Centre);

            // Draw the copyright notice
            yTextPos = g_FrameMan.GetResY() - pFont->GetFontHeight();
            char copyRight[512];
            sprintf(copyRight, "Cortex Command is TM and %c 2017 Data Realms, LLC", -35);
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, copyRight, GUIFont::Centre);
        }

        //////////////////////////////////////////////////////////
        // Scene drawing

        if (g_IntroState >= FADEIN)
        {
            g_FrameMan.ClearBackBuffer32();

			Box backdropBox;
// Use the override of the SL drawing isntead
//            pBackdrop->SetOffset(scrollOffset * backdropScrollRatio);
            pBackdrop->Draw(g_FrameMan.GetBackBuffer32(), backdropBox, scrollOffset * backdropScrollRatio);

            Vector starDrawPos;
            StarSize size;
            for (int star = 0; star < starCount; ++star)
            {
                size = aStars[star].m_Size;
                int intensity = 185 * aStars[star].m_Intensity + (size == StarSmall ? 35 : (size == StarLarge ? 70 : 70)) * PosRand();
                set_screen_blender(intensity, intensity, intensity, intensity);
                starDrawPos.SetXY(aStars[star].m_Pos.m_X, aStars[star].m_Pos.m_Y - scrollOffset.m_Y * aStars[star].m_ScrollRatio);
                draw_trans_sprite(g_FrameMan.GetBackBuffer32(), aStars[star].m_pBitmap, starDrawPos.GetFloorIntX(), starDrawPos.GetFloorIntY());
            }

            planetPos.SetXY(g_FrameMan.GetResX() / 2, 567 - scrollOffset.GetFloorIntY());
            pMoon->SetPos(Vector(planetPos.m_X + 200, 364 - scrollOffset.GetFloorIntY() * 0.60));
            pPlanet->SetPos(planetPos);

            pMoon->Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawAlpha);
            pPlanet->Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawAlpha);

			// Manually shake our shakeOffset to randomize some effects
			if (g_TimerMan.GetAbsoulteTime() > lastShake + 50000)
			{
				shakeOffset.m_X = RangeRand(-3, 3);
				shakeOffset.m_Y = RangeRand(-3, 3);
				lastShake = g_TimerMan.GetAbsoulteTime();
			}

			// Tell the menu that PP promo is off
			g_pMainMenuGUI->DisablePioneerPromoButton();


			// Draw pioneer promo capsule
			if (g_IntroState < MAINTOCAMPAIGN && orbitRotation < -PI * 1.27 && orbitRotation > -PI * 1.85)
			{
				// Start drawig pioneer apsule
				// Slowly decrease radius to show that the capsule is falling
				float radiusperc = 1 - ((fabs(orbitRotation) - (1.27 * PI)) / (0.35 * PI) / 4);
				// Slowly decrease size to make the capsule disappear after a while
				float sizeperc = 1 - ((fabs(orbitRotation) - (1.27 * PI)) / (0.35 * PI) / 1.5);

				// Rotate, place and draw capsule
				capsuleOffset.SetXY(orbitRadius * radiusperc, 0);
				capsuleOffset.RadRotate(orbitRotation);
				pPioneerCapsule->SetScale(sizeperc);
				pPioneerCapsule->SetPos(planetPos + capsuleOffset);
				pPioneerCapsule->SetRotAngle(orbitRotation);
				pPioneerCapsule->Draw(g_FrameMan.GetBackBuffer32());
			}

			// Enable promo clickables only if we're in main menu and the station is at the required location (under the menu)
			if (g_IntroState == MENUACTIVE && g_pMainMenuGUI->AllowPioneerPromo() &&  orbitRotation < -PI * 1.25 && orbitRotation > -PI * 1.95)
			{
				// After capsule flew some time, start showing angry pioneer
				if (orbitRotation < -PI * 1.32 && orbitRotation > -PI * 1.65)
				{
					Vector pioneerScreamPos = planetPos - Vector(320 - 130, 320 + 44);

					// Draw line to indicate that the screaming guy is the one in the drop pod
					drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
					g_pScenarioGUI->SetPlanetInfo(Vector(0,0), planetRadius);
					g_pScenarioGUI->DrawScreenLineToSitePoint(g_FrameMan.GetBackBuffer32(), pioneerScreamPos, pPioneerCapsule->GetPos(), makecol(255, 255, 255), -1, -1, 40, 0.20);
					drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

					// Draw pioneer
					pPioneerScreaming->SetPos(pioneerScreamPos + shakeOffset);
					pPioneerScreaming->Draw(g_FrameMan.GetBackBuffer32());

					// Enable the promo banner and tell the menu where it can be clicked
					g_pMainMenuGUI->EnablePioneerPromoButton();

					Box promoBox(pioneerScreamPos.m_X - 125, pioneerScreamPos.m_Y - 70, pioneerScreamPos.m_X + 125, pioneerScreamPos.m_Y + 70);
					g_pMainMenuGUI->SetPioneerPromoBox(promoBox);
				} 

				if (orbitRotation < -PI * 1.65 && orbitRotation > -PI * 1.95)
				{
					Vector promoPos = planetPos - Vector(320 - 128, 320 + 29);

					// Draw pioneer promo
					pPioneerPromo->SetPos(promoPos);
					pPioneerPromo->Draw(g_FrameMan.GetBackBuffer32());

					// Enable the promo banner and tell the menu where it can be clicked
					g_pMainMenuGUI->EnablePioneerPromoButton();

					Box promoBox(promoPos.m_X - 128, promoPos.m_Y - 80, promoPos.m_X + 128, promoPos.m_Y + 80);
					g_pMainMenuGUI->SetPioneerPromoBox(promoBox);
				}
			}
				
			// Place, rotate and draw station
			stationOffset.SetXY(orbitRadius, 0);
			stationOffset.RadRotate(orbitRotation);
			pStation->SetPos(planetPos + stationOffset);
			pStation->SetRotAngle(-HalfPI + orbitRotation);
			pStation->Draw(g_FrameMan.GetBackBuffer32());

			// Start explosion effects to show that there's something wrong with the station
			// but only if we're not in campaign
			if (g_IntroState < MAINTOCAMPAIGN && orbitRotation < -PI * 1.25 && orbitRotation > -TwoPI)
			{
				// Add explosions delay and count them
				if (g_TimerMan.GetAbsoulteTime() > lastPuff + 1000000)
				{
					lastPuff = g_TimerMan.GetAbsoulteTime();
					puffActive = true;
					puffCount++;
				}

				// If explosion was authorized
				if (puffActive)
				{
					// First explosion is big while other are smaller
					if (puffCount == 1)
					{
						pFirePuffLarge->SetPos(planetPos + stationOffset);
						if (g_TimerMan.GetAbsoulteTime() > lastPuffFrame + 50000)
						{
							lastPuffFrame = g_TimerMan.GetAbsoulteTime();
							puffFrame++;

							if (puffFrame >= pFirePuffLarge->GetFrameCount())
							{
								// Manually reset frame counters and disable other explosions until it's time
								puffFrame = 0;
								puffActive = 0;
							}

							pFirePuffLarge->SetFrame(puffFrame);
						}
						pFirePuffLarge->Draw(g_FrameMan.GetBackBuffer32());
					} else {
						pFirePuffMedium->SetPos(planetPos + stationOffset + shakeOffset);
						if (g_TimerMan.GetAbsoulteTime() > lastPuffFrame + 50000)
						{
							lastPuffFrame = g_TimerMan.GetAbsoulteTime();
							puffFrame++;

							if (puffFrame >= pFirePuffLarge->GetFrameCount())
							{
								// Manually reset frame counters and disable other explosions until it's time
								puffFrame = 0;
								puffActive = 0;
							}

							pFirePuffMedium->SetFrame(puffFrame);
						}
						pFirePuffMedium->Draw(g_FrameMan.GetBackBuffer32());
					}
				}
			} else {
				//Reset explosions counter
				puffCount = 0;
			}

			orbitRotation -= 0.0020; //0.0015

            // Keep the rotation angle from getting too large
            if (orbitRotation < -TwoPI)
                orbitRotation += TwoPI;
            g_StationOffsetX = stationOffset.m_X;
            g_StationOffsetY = stationOffset.m_Y;
        }

        /////////////////////////////
        // Game Logo drawing

        if ((g_IntroState >= TITLEAPPEAR && g_IntroState < SCENARIOFADEIN) || g_IntroState == MAINTOCAMPAIGN)
        {
            if (g_IntroState == TITLEAPPEAR)
                pTitle->SetPos(Vector(resX / 2, (resY / 2) - 20));
            else if (g_IntroState == PLANETSCROLL && sectionProgress > 0.5)
                pTitle->SetPos(Vector(resX / 2, EaseIn((resY / 2) - 20, 120, (sectionProgress - 0.5) / 0.5)));//LERP(0.5, 1.0, (resY / 2) - 20, 120, sectionProgress)));
            else if (g_IntroState == MENUAPPEAR)
                pTitle->SetPos(Vector(resX / 2, EaseOut(120, 64, sectionProgress)));
            else if (g_IntroState == MAINTOSCENARIO || g_IntroState == MAINTOCAMPAIGN)
                pTitle->SetPos(Vector(resX / 2, EaseOut(64, -150, sectionProgress)));
            else if (g_IntroState >= MENUAPPEAR)
                pTitle->SetPos(Vector(resX / 2, 64));

            pTitleGlow->SetPos(pTitle->GetPos());

            pTitle->Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawAlpha);
            // Screen blend the title glow on top, with some flickering in its intensity
            int blendAmount = 220 + 35 * NormalRand();
            set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
            pTitleGlow->Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawTrans);
        }

        /////////////////////////////
        // Menu drawing

        // Main Menu updating and drawing, behind title
        if (g_IntroState >= MENUAPPEAR)
        {
            if (g_IntroState == MENUAPPEAR)
            {
// TODO: some fancy transpernecy effect here
/*
                g_pMainMenuGUI->Update();
                clear_to_color(pFadeScreen, 0xFFFF00FF);
                g_pMainMenuGUI->Draw(pFadeScreen);
                fadePos = 255 * sectionProgress;
                set_trans_blender(fadePos, fadePos, fadePos, fadePos);
                draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);
*/
            }
            else if (g_IntroState == MENUACTIVE)
            {
                g_pMainMenuGUI->Update();
                g_pMainMenuGUI->Draw(g_FrameMan.GetBackBuffer32());
            }
        }

        // Scenario setup menu update and drawing
        if (g_IntroState == SCENARIOMENU)
        {
            g_pScenarioGUI->SetPlanetInfo(planetPos, planetRadius);
            g_pScenarioGUI->Update();
            g_pScenarioGUI->Draw(g_FrameMan.GetBackBuffer32());
        }

        // Metagame menu update and drawing
        if (g_IntroState == CAMPAIGNPLAY)
        {
            g_MetaMan.GetGUI()->SetPlanetInfo(planetPos, planetRadius);
            g_MetaMan.Update();
            g_MetaMan.Draw(g_FrameMan.GetBackBuffer32());
        }

        ////////////////////////////////////
        // Slides drawing

        if (g_IntroState >= SHOWSLIDE1 && g_IntroState <= SHOWSLIDE8)
        {
            int slide = g_IntroState - SHOWSLIDE1;
            Vector slideCenteredPos((resX / 2) - (apIntroSlides[slide]->w / 2), (resY / 2) - (apIntroSlides[slide]->h / 2));

            // Screenwide slide
            if (apIntroSlides[slide]->w <= resX)
                slidePos.m_X = (resX / 2) - (apIntroSlides[slide]->w / 2);
            // The slides wider than the screen, pan sideways
            else
            {
                if (elapsed < slideFadeInDuration)
                    slidePos.m_X = 0;
                else if (elapsed < duration - slideFadeOutDuration)
                    slidePos.m_X = EaseInOut(0, resX - apIntroSlides[slide]->w, (elapsed - slideFadeInDuration) / (duration - slideFadeInDuration - slideFadeOutDuration));
                else
                    slidePos.m_X = resX - apIntroSlides[slide]->w;
            }

            // TEMP?
            slidePos.m_Y = slideCenteredPos.m_Y;
            // TEMP?
            if (elapsed < slideFadeInDuration)
            {
                fadePos = EaseOut(0, 255, elapsed / slideFadeInDuration);
//                slidePos.m_Y = EaseOut(slideCenteredPos.m_Y - slideFadeDistance, slideCenteredPos.m_Y, elapsed / slideFadeInDuration);
            }
            else if (elapsed < duration - slideFadeOutDuration)
            {
                fadePos = 255;
                slidePos.m_Y = slideCenteredPos.m_Y;
            }
            else
            {
                fadePos = EaseIn(255, 0, (elapsed - duration + slideFadeOutDuration) / slideFadeOutDuration);
//                slidePos.m_Y = EaseIn(slideCenteredPos.m_Y, slideCenteredPos.m_Y + slideFadeDistance, (elapsed - duration + slideFadeOutDuration) / slideFadeOutDuration);
            }

            if (fadePos > 0)
            {
                set_trans_blender(fadePos, fadePos, fadePos, fadePos);
                draw_trans_sprite(g_FrameMan.GetBackBuffer32(), apIntroSlides[slide], slidePos.m_X, slidePos.m_Y);
            }
        }

        //////////////////////////////
        // Letterbox drawing
/*
        if (g_IntroState >= FADEIN && g_IntroState < MENUACTIVE)
        {
            int thick = 60;
            if (g_IntroState == MENUAPPEAR)
            {
                rectfill(g_FrameMan.GetBackBuffer32(), 0, 0, resX, EaseOut(thick, 0, sectionProgress), 0);
                rectfill(g_FrameMan.GetBackBuffer32(), 0, EaseOut(resY - thick, resY, sectionProgress), resX, resY, 0);
            }
            else
            {
                rectfill(g_FrameMan.GetBackBuffer32(), 0, 0, resX, thick, 0);
                rectfill(g_FrameMan.GetBackBuffer32(), 0, resY - thick, resX, resY, 0);
            }
        }
*/
        //////////////////////////////////////////////////////////
        // Intro sequence logic

        if (g_IntroState == START)
        {
            g_IntroState = LOGOFADEIN;
            sectionSwitch = true;
			if (g_SettingsMan.ModsInstalledLastTime().size() >= 5)
			{
				g_AchievementMan.UnlockAchievement("CC_5WORKSHOP");
			}
        }
        else if (g_IntroState == LOGOFADEIN)
        {
            if (sectionSwitch)
            {
                // Play juicy logo signature jingle/sound
                Sound logoSound;
                logoSound.Create("Base.rte/GUIs/Sounds/MetaStart.wav", false);
                logoSound.Play();
                // Black fade
                clear_to_color(pFadeScreen, 0);
                duration = 0.25;
                sectionSwitch = false;
            }

            fadePos = 255 - (255 * sectionProgress);
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            if (elapsed >= duration)
            {
                g_IntroState = LOGODISPLAY;
                sectionSwitch = true;
            }
            else if (keyPressed)
            {
                g_IntroState = LOGOFADEOUT;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == LOGODISPLAY)
        {
            if (sectionSwitch)
            {
                duration = 2.0;
                sectionSwitch = false;
            }
            if (elapsed > duration || keyPressed)
            {
                g_IntroState = LOGOFADEOUT;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == LOGOFADEOUT)
        {
            if (sectionSwitch)
            {
                // Black fade
                clear_to_color(pFadeScreen, 0);
                duration = 0.5;
                sectionSwitch = false;
            }

            fadePos = 255 * sectionProgress;
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            if (elapsed >= duration || keyPressed)
            {
                g_IntroState = NOTICEFADEIN;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == NOTICEFADEIN)
        {
            if (sectionSwitch)
            {
                // Black fade
                clear_to_color(pFadeScreen, 0);
                duration = 0.5;
                sectionSwitch = false;
            }

            fadePos = 255 - (255 * sectionProgress);
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            if (elapsed >= duration)
            {
                g_IntroState = NOTICEDISPLAY;
                sectionSwitch = true;
            }
            else if (keyPressed)
            {
                g_IntroState = NOTICEFADEOUT;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == NOTICEDISPLAY)
        {
            if (sectionSwitch)
            {
                duration = 7.0;
                sectionSwitch = false;
            }
            if (elapsed > duration || keyPressed)
            {
                g_IntroState = NOTICEFADEOUT;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == NOTICEFADEOUT)
        {
            if (sectionSwitch)
            {
                // Black fade
                clear_to_color(pFadeScreen, 0);
                duration = 0.5;
                sectionSwitch = false;
            }

            fadePos = 255 * sectionProgress;
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            if (elapsed >= duration || keyPressed)
            {
                g_IntroState = FADEIN;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == FADEIN)
        {
            if (sectionSwitch)
            {
                // Start scroll at the bottom
                scrollOffset.m_Y = startYOffset;
                // Black fade
                clear_to_color(pFadeScreen, 0);

                duration = 1.0;
                sectionSwitch = false;

                // Play intro music
                g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccintro.ogg", 0);
                g_AudioMan.SetMusicPosition(0.05);
                // Override music volume setting for the intro if it's set to anything
                if (g_AudioMan.GetMusicVolume() > 0.1)
                    g_AudioMan.SetTempMusicVolume(1.0);
//                songTimer.Reset();
                songTimer.SetElapsedRealTimeS(0.05);
            }

            fadePos = 255 - (255 * sectionProgress);
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            if (elapsed >= duration)
            {
                g_IntroState = SPACEPAUSE1;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SPACEPAUSE1)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 3.7;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

//            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "In a not too distant future...", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = SHOWSLIDE1;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SHOWSLIDE1)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 11.4;
                slideFadeInDuration = 2.0;
                slideFadeOutDuration = 0.5;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
            if (elapsed > 1.25)
                pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "At the end of humanity's darkest century...", GUIFont::Centre);
//            yTextPos += 20;
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "but also the restrictions of their physical bodies...", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = SHOWSLIDE2;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SHOWSLIDE2)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 17.3;
                slideFadeInDuration = 0.5;
                slideFadeOutDuration = 2.5;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
            if (elapsed < duration - 1.75)
                pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "...a curious symbiosis between man and machine emerged.", GUIFont::Centre);
//            yTextPos += 20;
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "their natural habitats and societies,", GUIFont::Centre);


            if (elapsed >= duration)
            {
                g_IntroState = SHOWSLIDE3;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SHOWSLIDE3)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 25.1;
                slideFadeInDuration = 0.5;
                slideFadeOutDuration = 0.5;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
            if (/*elapsed > 0.75 && */sectionProgress < 0.49)
                pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "This eventually enabled humans to leave their natural bodies...", GUIFont::Centre);
            else if (sectionProgress > 0.51)
                pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "...and to free their minds from obsolete constraints.", GUIFont::Centre);
//            yTextPos += 20;
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "their disembodied brains maintained by artifical means.", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = SHOWSLIDE4;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SHOWSLIDE4)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 31.3;
                slideFadeInDuration = 0.5;
                slideFadeOutDuration = 0.5;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "With their brains sustained by artificial means, space travel also became feasible.", GUIFont::Centre);
//            yTextPos += 20;
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "they still require physical materials to sustain themselves.", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = SHOWSLIDE5;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SHOWSLIDE5)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 38.0;
                slideFadeInDuration = 0.5;
                slideFadeOutDuration = 0.5;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "Other civilizations were encountered...", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = SHOWSLIDE6;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SHOWSLIDE6)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 44.1;
                slideFadeInDuration = 0.5;
                slideFadeOutDuration = 0.5;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "...and peaceful intragalactic trade soon established.", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = SHOWSLIDE7;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SHOWSLIDE7)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 51.5;
                slideFadeInDuration = 0.5;
                slideFadeOutDuration = 0.5;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "Now, the growing civilizations create a huge demand for resources...", GUIFont::Centre);
//            yTextPos += 20;
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "said resources are to harvest profitably.", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = SHOWSLIDE8;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SHOWSLIDE8)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 64.5;
                slideFadeInDuration = 0.5;
                slideFadeOutDuration = 0.5;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2) + (apIntroSlides[g_IntroState - SHOWSLIDE1]->h / 2) + 12;
            if (sectionProgress < 0.30)
                pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "...which can only be satisfied by the ever-expanding frontier.", GUIFont::Centre);
            else if (sectionProgress > 0.33 && sectionProgress < 0.64)
                pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "Competition is brutal and anything goes in this galactic gold rush.", GUIFont::Centre);
            else if (sectionProgress > 0.67)
                pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "You will now join it on a venture to an untapped planet...", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = PRETITLE;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == PRETITLE)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 66.6;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            yTextPos = (g_FrameMan.GetResY() / 2);
            if (elapsed > 0.05)
                pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "Prepare to assume...", GUIFont::Centre);

            if (elapsed >= duration)
            {
                g_IntroState = TITLEAPPEAR;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == TITLEAPPEAR)
        {
            if (sectionSwitch)
            {
                // White fade
                clear_to_color(pFadeScreen, 0xFFFFFFFF);
                sectionSongEnd = 68.2;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            fadePos = LERP(0, 0.5, 255, 0, sectionProgress);
            if (fadePos >= 0)
            {
                set_trans_blender(fadePos, fadePos, fadePos, fadePos);
                draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);
            }

            if (elapsed >= duration)
            {
				g_AchievementMan.UnlockAchievement( "CC_WATCHINTRO" );
                g_IntroState = PLANETSCROLL;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == PLANETSCROLL)
        {
            if (sectionSwitch)
            {
                sectionSongEnd = 92.4;
                duration = sectionSongEnd - songTimer.GetElapsedRealTimeS();
                sectionSwitch = false;
            }

            if (elapsed >= duration)
            {
                g_IntroState = PREMENU;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == PREMENU)
        {
            if (sectionSwitch)
            {
                duration = 3.0;
                sectionSwitch = false;
                scrollOffset.m_Y = preMenuYOffset;
            }

            if (elapsed >= duration || keyPressed)
            {
                g_IntroState = MENUAPPEAR;

                sectionSwitch = true;
            }
        }
        else if (g_IntroState == MENUAPPEAR)
        {
            if (sectionSwitch)
            {
                duration = 1.0;
                sectionSwitch = false;
                scrollOffset.m_Y = preMenuYOffset;

                // Play the main menu ambient
                g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccmenu.ogg", -1);
            }

            if (elapsed >= duration || g_NetworkServer.IsServerModeEnabled())
            {
                g_IntroState = MENUACTIVE;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == MENUACTIVE)
        {
            if (sectionSwitch)
            {
                scrollOffset.m_Y = topMenuYOffset;
                // Fire up the menu
                g_pMainMenuGUI->SetEnabled(true);
                // Indicate that we're now in the main menu
                g_InActivity = false;

                sectionSwitch = false;
            }

//            yTextPos = (g_FrameMan.GetResY() / 3) - 15;
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "Press Any Key to Start", GUIFont::Centre);
//            yTextPos += pFont->GetFontHeight();
//            pFont->DrawAligned(&backBuffer, g_FrameMan.GetResX() / 2, yTextPos, "Press Any Key to Start", GUIFont::Centre);

//            g_pMainMenuGUI->Draw();

            // Detect quitting of the program from the menu button
            g_Quit = g_Quit || g_pMainMenuGUI->QuitProgram();

            // Detect if a scenario mode has been commanded to start
            if (g_pMainMenuGUI->ScenarioStarted())
            {
                g_IntroState = MAINTOSCENARIO;
                sectionSwitch = true;
            }

            // Detect if a campaign mode has been commanded to start
            if (g_pMainMenuGUI->CampaignStarted())
            {
                g_IntroState = MAINTOCAMPAIGN;
                sectionSwitch = true;
            }

            // Detect if the current game has been commanded to resume
            if (g_pMainMenuGUI->ActivityResumed())
                g_ResumeActivity = true;

            // Detect if a game has been commanded to restart
            if (g_pMainMenuGUI->ActivityRestarted())
            {
                // Make sure the scene is going to be reset with the new parameters
                g_ResetActivity = true;

                g_IntroState = FADESCROLLOUT;
                sectionSwitch = true;
            }

			if (g_NetworkServer.IsServerModeEnabled())
			{
				EnterMultiplayerLobby();
				g_IntroState = FADESCROLLOUT;
				sectionSwitch = true;
			}
        }
        else if (g_IntroState == MAINTOSCENARIO)
        {
            if (sectionSwitch)
            {
                duration = 2.0;
                sectionSwitch = false;

                // Reset the Scenario menu
                g_pScenarioGUI->SetEnabled(true);

                // Play the scenario music with juicy start sound
                Sound metaSound;
                metaSound.Create("Base.rte/GUIs/Sounds/MetaStart.wav", false);
                metaSound.Play();
                g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/thisworld5.ogg", -1);
            }

            if (elapsed >= duration || g_NetworkServer.IsServerModeEnabled())// || keyPressed)
            {
                g_IntroState = SCENARIOMENU;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SCENARIOFADEIN)
        {
            if (sectionSwitch)
            {
                // Scroll to planet pos
                scrollOffset.m_Y = planetViewYOffset;
                // Black fade
                clear_to_color(pFadeScreen, 0);

                duration = 1.0;
                sectionSwitch = false;
/* Restart campaign scenario setup screen music
                // Play intro music at max volume regardless of setting
                g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccintro.ogg", 0, 1.0);
                g_AudioMan.SetMusicPosition(0.05);
                // Override music volume setting for the intro
                g_AudioMan.SetTempMusicVolume(1.0);
//                songTimer.Reset();
                songTimer.SetElapsedRealTimeS(0.05);
*/
            }

            fadePos = 255 - (255 * sectionProgress);
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            if (elapsed >= duration)
            {
                g_IntroState = SCENARIOMENU;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == SCENARIOMENU)
        {
            if (sectionSwitch)
            {
                scrollOffset.m_Y = planetViewYOffset;
//                // Fire up the CampaignMan
//                g_pMainMenuGUI->SetEnabled(true);
//                // Indicate that we're now in the main menu
//                g_InActivity = false;

                sectionSwitch = false;
            }

//            g_CampaignMan->Draw();

            // Detect quitting of the program from the menu button
            g_Quit = g_Quit || g_pScenarioGUI->QuitProgram();

            // Detect if user wants to go back to main menu
            if (g_pScenarioGUI->BackToMain())
            {
                g_IntroState = PLANETTOMAIN;
                sectionSwitch = true;
            }

            // Detect if the current game has been commanded to resume
            if (g_pScenarioGUI->ActivityResumed())
                g_ResumeActivity = true;

            // Detect if a game has been commanded to restart
            if (g_pScenarioGUI->ActivityRestarted())
            {
                // Make sure the scene is going to be reset with the new parameters
                g_ResetActivity = true;

                g_IntroState = FADEOUT;
                sectionSwitch = true;
            }

			// In server mode once we exited to main or scenario menu we need to start Lobby activity 
			if (g_NetworkServer.IsServerModeEnabled())
			{
				EnterMultiplayerLobby();
				g_IntroState = FADEOUT;
				sectionSwitch = true;
			}
        }
        else if (g_IntroState == MAINTOCAMPAIGN)
        {
            if (sectionSwitch)
            {
                duration = 2.0;
                sectionSwitch = false;

                // Play the campaign music with metasound start
                Sound metaSound;
                metaSound.Create("Base.rte/GUIs/Sounds/MetaStart.wav", false);
                metaSound.Play();
                g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/thisworld5.ogg", -1);
            }

            if (elapsed >= duration)// || keyPressed)
            {
                g_IntroState = CAMPAIGNPLAY;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == CAMPAIGNFADEIN)
        {
            if (sectionSwitch)
            {
                // Scroll to campaign pos
                scrollOffset.m_Y = planetViewYOffset;
                // Black fade
                clear_to_color(pFadeScreen, 0);

                duration = 1.0;
                sectionSwitch = false;
/* Restart campaign metagame screen music
                // Play intro music at max volume regardless of setting
                g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccintro.ogg", 0, 1.0);
                g_AudioMan.SetMusicPosition(0.05);
                // Override music volume setting for the intro
                g_AudioMan.SetTempMusicVolume(1.0);
//                songTimer.Reset();
                songTimer.SetElapsedRealTimeS(0.05);
*/
            }

            fadePos = 255 - (255 * sectionProgress);
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            if (elapsed >= duration)
            {
                g_IntroState = CAMPAIGNPLAY;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == CAMPAIGNPLAY)
        {
            if (sectionSwitch)
            {
                scrollOffset.m_Y = planetViewYOffset;
//                // Fire up the CampaignMan
//                g_pMainMenuGUI->SetEnabled(true);
//                // Indicate that we're now in the main menu
//                g_InActivity = false;

                sectionSwitch = false;
            }

//            g_CampaignMan->Draw();

            // Detect quitting of the program from the menu button
            g_Quit = g_Quit || g_MetaMan.GetGUI()->QuitProgram();

            // Detect if user wants to go back to main menu
            if (g_MetaMan.GetGUI()->BackToMain())
            {
                g_IntroState = PLANETTOMAIN;
                sectionSwitch = true;
            }

            // Detect if a game has been commanded to restart
            if (g_MetaMan.GetGUI()->ActivityRestarted())
            {
                // Make sure the scene is going to be reset with the new parameters
                g_ResetActivity = true;

                g_IntroState = FADEOUT;
                sectionSwitch = true;
            }
            // Detect if the current game has been commanded to resume
            if (g_MetaMan.GetGUI()->ActivityResumed())
                g_ResumeActivity = true;
        }
        else if (g_IntroState == PLANETTOMAIN)
        {
            if (sectionSwitch)
            {
                duration = 2.0;
                sectionSwitch = false;
            }

            if (elapsed >= duration)// || keyPressed)
            {
                g_IntroState = MENUACTIVE;
                sectionSwitch = true;
            }
        }
        else if (g_IntroState == FADESCROLLOUT)
        {
            if (sectionSwitch)
            {
                // Black fade
                clear_to_color(pFadeScreen, 0x00000000);
                duration = 1.5;
                sectionSwitch = false;
            }

            scrollOffset.m_Y = EaseIn(topMenuYOffset, 250, sectionProgress);

            fadePos = EaseIn(0, 255, sectionProgress);
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            // Fade out the music as well
            g_AudioMan.SetTempMusicVolume(EaseIn(g_AudioMan.GetMusicVolume(), 0, sectionProgress));

            if (elapsed >= duration)
            {
                g_IntroState = END;
                sectionSwitch = true;
                g_FrameMan.ClearBackBuffer32();
            }
        }
        else if (g_IntroState == FADEOUT)
        {
            if (sectionSwitch)
            {
                // White fade
                clear_to_color(pFadeScreen, 0x00000000);
                duration = 1.5;
                sectionSwitch = false;
            }

//            scrollOffset.m_Y = EaseIn(topMenuYOffset, 250, sectionProgress);

            fadePos = EaseIn(0, 255, sectionProgress);
            set_trans_blender(fadePos, fadePos, fadePos, fadePos);
            draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pFadeScreen, 0, 0);

            // Fade out the music as well
//            g_AudioMan.SetTempMusicVolume(g_AudioMan.GetMusicVolume() * 1.0 - sectionProgress);
            g_AudioMan.SetTempMusicVolume(EaseIn(g_AudioMan.GetMusicVolume(), 0, sectionProgress));

            if (elapsed >= duration)
            {
                g_IntroState = END;
                sectionSwitch = true;
                g_FrameMan.ClearBackBuffer32();
            }
        }

        ////////////////////////////////
        // Additional user input and skipping handling
        
        if (g_IntroState >= FADEIN && g_IntroState <= SHOWSLIDE8 && keyPressed)
        {
            g_IntroState = MENUAPPEAR;
            sectionSwitch = true;

            scrollOffset.m_Y = preMenuYOffset;
            orbitRotation = HalfPI - EigthPI;

			orbitRotation = -PI * 1.20;
/*
            // Start/Jump the song to the theme spot
            g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccintro.ogg", 0);
            g_AudioMan.SetMusicPosition(66.7);
            songTimer.SetElapsedRealTimeS(66.7);
*/
        }

        // Draw the console in the menu
        g_ConsoleMan.Draw(g_FrameMan.GetBackBuffer32());

        // Wait for vertical synch before flipping frames
        vsync();
        g_FrameMan.FlipFrameBuffers();
    }

    // Clean up heap data
    destroy_bitmap(pFadeScreen); pFadeScreen = 0;
    for (int slide = 0; slide < SLIDECOUNT; ++slide)
    {
        destroy_bitmap(apIntroSlides[slide]);
        apIntroSlides[slide] = 0;
    }
    delete [] apIntroSlides; apIntroSlides = 0;
    delete pBackdrop; pBackdrop = 0;
    delete pTitle; pTitle = 0;
    delete pPlanet; pPlanet = 0;
    delete pMoon; pMoon = 0;
    delete pStation; pStation = 0;
	delete pPioneerCapsule; pPioneerCapsule = 0;
	delete pPioneerScreaming; pPioneerScreaming = 0;
	delete pFirePuffLarge; pFirePuffLarge = 0;
	delete pFirePuffMedium; pFirePuffMedium = 0;
    delete [] aStars; aStars = 0;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Game simulation loop pump

bool RunGameLoop()
{
    if (g_Quit)
        return true;

    g_FrameMan.ResetFrameTimer();
    g_TimerMan.EnableAveraging(true);
    g_TimerMan.PauseSim(false);
// Done in ResetActivity
//    g_ActivityMan.GetActivity()->StartActivity();
    if (g_ResetActivity)
        ResetActivity();

    while (!g_Quit)
    {
        SLICK_PROFILENAME("Game Loop", 0xFFFF0000);

        {
            // Need to clear this out; sometimes background layers don't cover the whole back
            g_FrameMan.ClearBackBuffer8();

#ifdef SLICK_PROFILER
            // Force to only one sim update for this graphics frame if the profiling tool is connected
// Can switch this on and off in the console now
//            g_TimerMan.SetOneSimUpdatePerFrame(Debug::DebugTool::Instance()->IsDebugToolConnected());
#endif // SLICK_PROFILER

            // Update the real time measurement and increment
            g_TimerMan.Update();
#if __USE_SOUND_GORILLA
			g_AudioMan.Update();
#endif

			bool serverUpdated = false;

            // Simulation update, as many times as the fixed update step allows in the span since last frame draw
            while (g_TimerMan.TimeForSimUpdate())
            {
				serverUpdated = false;
                SLICK_PROFILENAME("Simulation Update", 0xFFFFFF00);
				g_FrameMan.NewPerformanceSample();

                // Advance the simulation time by the fixed amount
                g_TimerMan.UpdateSim();

				g_FrameMan.StartPerformanceMeasurement(FrameMan::PERF_SIM_TOTAL);
				g_UInputMan.Update();
				// It is vital that server is updated after input manager but before activity because unput manager will clear 
				// received pressed and released events on next update.
				if (g_NetworkServer.IsServerModeEnabled())
				{
					g_NetworkServer.Update(true);
					serverUpdated = true;
				}
				g_FrameMan.Update();
				g_AudioMan.Update();
				g_LuaMan.Update();
				g_FrameMan.StartPerformanceMeasurement(FrameMan::PERF_ACTIVITY);
				g_ActivityMan.Update();
				g_FrameMan.StopPerformanceMeasurement(FrameMan::PERF_ACTIVITY);
				g_MovableMan.Update();

				g_ActivityMan.LateUpdateGlobalScripts();

				g_ConsoleMan.Update();
				g_FrameMan.StopPerformanceMeasurement(FrameMan::PERF_SIM_TOTAL);

                if (!g_InActivity)
                {
					g_TimerMan.PauseSim(true);
					// If we're not in a metagame, then show main menu
					if (g_MetaMan.GameInProgress())
						g_IntroState = CAMPAIGNFADEIN;
					else
					{
						Activity * pActivity = g_ActivityMan.GetActivity();
						// If we edited something then return to main menu instead of scenario menu
						// player will probably switch to area/scene editor
						if (pActivity && pActivity->GetPresetName() == "None")
							g_IntroState = MENUAPPEAR;
						else
							g_IntroState = MAINTOSCENARIO;
					}
					PlayIntroTitle();
                }
                // Resetting the simulation
                if (g_ResetActivity)
                {
                    // Reset and quit if user quit during reset loading
                    if (!ResetActivity())
                        break;
                }
                // Resuming the simulation
                if (g_ResumeActivity)
                    ResumeActivity();
            }

			if (g_NetworkServer.IsServerModeEnabled())
			{
				// Pause sim while we're waiting for scene transmission or scene will
				// start changing before cleints receive them and those changes will be lost
				if (!g_NetworkServer.ReadyForSimulation())
					g_TimerMan.PauseSim(true);
				else 
					if (g_InActivity)
						g_TimerMan.PauseSim(false);

				if (!serverUpdated)
				{
					g_NetworkServer.Update();
					serverUpdated = true;
				}

				if (g_SettingsMan.GetServerSimSleepWhenIdle())
				{
					signed long long ticksToSleep = g_TimerMan.GetTimeToSleep();
					if (ticksToSleep > 0)
					{
						double secsToSleep = (double)ticksToSleep / (double)g_TimerMan.GetTicksPerSecond();
						long long milisToSleep = (long long)secsToSleep * (1000);
						std::this_thread::sleep_for(std::chrono::milliseconds(milisToSleep));
					}
				}
			}


#if defined(STEAM_BUILD)
		g_SteamUGCMan.Update();
#endif // 
        }

        // Frame draw update
        if (!g_Quit)
        {
            SLICK_PROFILENAME("Rendering Frame", 0xFFFF00FF);
            g_FrameMan.Draw();
            g_FrameMan.FlipFrameBuffers();
        }

// Slick Profiler updates - the debug tool needs to update BEFORE the profile system because control messages may have been sent.
#ifdef SLICK_PROFILER
        Debug::DebugTool::Instance()->Update();
	    ProfileSystem::Instance()->Update();
#endif
    }

    return true;
}


#ifndef __OPEN_SOURCE_EDITION

/////////////////////////////
// TURN OPTIMIZATIONS OFF
// This is so the EXECryptor markers don't get mangled by the optimizer

#pragma optimize("", off)

#endif


//////////////////////////////////////////////////////////////////////////////////////////
// Command-line argument handling, returns false if app should quit right after this
// The appExitVar is what the program should exit with if this returns false

bool HandleMainArgs(int argc, char *argv[], int &appExitVar)
{
#ifndef __OPEN_SOURCE_EDITION
    CRYPT_START
#endif

    // If no additional args passed, just continue (first arg is the program path)
    if (argc == 1)
        return true;

    // Default program return var is fail
    appExitVar = 2;

    // If one additional arg is passed
    if (argc == 2)
    {
        // If -register was passed as param, try to register with last used key and quit immediately (regardless of success)
        if (strcmp(argv[1], "-register") == 0)
        {
            // Try to register the last used key, if we're not currently registered and we do have a last used key
            if (!g_LicenseMan.HasValidatedLicense())
            {
                int result = g_LicenseMan.Register(g_LicenseMan.GetLastLicenseEmail(), g_LicenseMan.GetLastLicenseKey());
                if (result == LicenseMan::SUCCESS || result == LicenseMan::DEPRECATEDKEY)
                    appExitVar = 0;
                g_SettingsMan.WriteLicenseKey();
            }
            // If already registered, then report that
            else
                appExitVar = 1;
            return false;
        }
        // If -unregister was passed as param, just try to unregister the license and quit immediately (regardless of success)
        else if (strcmp(argv[1], "-unregister") == 0)
        {
            if (g_LicenseMan.HasValidatedLicense())
            {
                if (g_LicenseMan.Unregister() == LicenseMan::SUCCESS)
                    appExitVar = 0;
                g_SettingsMan.WriteLicenseKey();
//                g_SettingsMan.Save(Writer("Base.rte/Settings.ini"));
            }
            // If already unregistered, then report that
            else
                appExitVar = 1;
            return false;
        }
    }

	if (argc > 2)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-server") == 0 && i + 1 < argc)
			{
				std::string port = argv[i + 1];
				g_NetworkServer.EnableServerMode();
				g_NetworkServer.SetServerPort(port);
			}

			if (strcmp(argv[i], "-module") == 0 && i + 1 < argc)
			{
				g_LoadSingleModule = argv[i + 1];
			}
		}
	}

	/*
	if (argc > 3)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-activity") == 0 && i + 2 < argc)
			{
				g_SettingsMan.SetPlayIntro(false);
				g_ActivityMan.SetDefaultActivityType(argv[i + 1]);
				g_ActivityMan.SetDefaultActivityName(argv[i + 2]);
			}
		}
	}*/

    return true;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


#ifndef __OPEN_SOURCE_EDITION

/////////////////////////////
// TURN OPTIMIZATIONS ON
// This is so the EXECryptor markers don't get mangled by the optimizer

#pragma optimize("", on)

#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Implementation of the main function.

int main(int argc, char *argv[])
{
    ///////////////////////////////////////////////////////////////////
	// Change to working directory (necessary for some platforms)
	g_System.ChangeWorkingDirectory();

#if defined(__APPLE__)
	OsxUtil::Create();
#endif // defined(__APPLE__)
	
    ///////////////////////////////////////////////////////////////////
    // Init Allegro

/* This is obsolete; only applied when we were loading from a compressed Base.rte
    // Load the Allegro config data from the base datafile
    DATAFILE *pConfigFile = load_datafile_object("Base.rte/Base.dat", "AConfig");
    if (pConfigFile)
    {
        set_config_data((char *)pConfigFile->dat, pConfigFile->size);
        // The above copies the data, so this is safe to do
        unload_datafile_object(pConfigFile);
    }
*/
    set_config_file("Base.rte/AllegroConfig.txt");
    allegro_init();
    // Enable the exit button on the window
    LOCK_FUNCTION(QuitHandler);
    set_close_button_callback(QuitHandler);
    //set_window_title("Cortex Command");

    // Seed the random number generator
// PUT BACK
    SeedRand();
// REMOVE!
//    srand(100);

    ///////////////////////////////////////////////////////////////////
    // Init the Slick Profiler

#ifdef SLICK_PROFILER
    Debug::DebugTool::Initialize();
	ProfileSystem::Initialize();
#endif

    ///////////////////////////////////////////////////////////////////
    // Instantiate all the managers

    new ConsoleMan();
    new LuaMan();
    new LicenseMan();
    new SettingsMan();
    new TimerMan();
    new PresetMan();
    new FrameMan();
    new AudioMan();
    new UInputMan();
    new ActivityMan();
    new MovableMan();
    new SceneMan();
    new MetaMan();

	new NetworkServer();
	new NetworkClient();

#if defined(STEAM_BUILD)
	SteamAPI_Init();
	new SteamUGCMan();
#endif
    // Outside because it has its own internal preproc conditions and will be called many times
    // which would be annoying to have to add conditions for each call
	new AchievementMan();

    ///////////////////////////////////////////////////////////////////
    // Create the essential managers

    g_LuaMan.Create();
    g_LicenseMan.Create();
    // It's ok if ReadLicenseKey fails, it just means we're not registered yet
    g_SettingsMan.ReadLicenseKey();
	
	Reader settingsReader("Base.rte/Settings.ini", false, 0, true);
    g_SettingsMan.Create(settingsReader);

	g_NetworkServer.Create();
	g_NetworkClient.Create();

    int exitVar = 0;
    if (!HandleMainArgs(argc, argv, exitVar))
        return exitVar;
    g_TimerMan.Create();
    g_PresetMan.Create();
    g_FrameMan.Create();
    g_AudioMan.Create();
    g_UInputMan.Create();
	if (g_NetworkServer.IsServerModeEnabled())
		g_UInputMan.SetMultiplayerMode(true);
    g_ConsoleMan.Create();
    g_ActivityMan.Create();
    g_MovableMan.Create();
    g_MetaMan.Create();

	// [CHRISK] STEAM SUPPORT
#if defined(STEAM_BUILD)
	g_SteamUGCMan.Create();

    // Update once and push out the console contents after manager init so we can check it before having to load everything
    g_SteamUGCMan.Update();
    g_ConsoleMan.Update();
    g_ConsoleMan.SaveAllText("LogConsole.txt");
#endif 

// TODO: REMOVE
/*
    if (g_LuaMan.RunScriptFile("Base.rte/Scripts/Test.lua"))
        g_FrameMan.ShowMessageBox(string("Script error: ") + g_LuaMan.GetLastError());

    if (g_LuaMan.RunScriptFile("Base.rte/Scripts/Test2.lua"))
        g_FrameMan.ShowMessageBox(string("Script error: ") + g_LuaMan.GetLastError());

    if (g_LuaMan.RunScriptFile("Base.rte/Scripts/Test3.lua"))
        g_FrameMan.ShowMessageBox(string("Script error: ") + g_LuaMan.GetLastError());
*/
    ///////////////////////////////////////////////////////////////////
    // Main game driver

	string fullscreenDriver = "";
	string windowedDriver = "";

	if (g_SettingsMan.ForceSoftwareGfxDriver())
		fullscreenDriver = "MSG: Using software DirectX fullscreen driver!";
	if (g_SettingsMan.ForceSafeGfxDriver())
		fullscreenDriver = "MSG: Using safe DirectX fullscrteen driver!";

	if (g_SettingsMan.ForceOverlayedWindowGfxDriver()) 
		windowedDriver = "MSG: Using overlay DirectX windowed driver!";
	if (g_SettingsMan.ForceNonOverlayedWindowGfxDriver()) 
		windowedDriver = "MSG: Using non-overlay DirectX windowed driver!";
	if (g_SettingsMan.ForceVirtualFullScreenGfxDriver()) 
		windowedDriver = "MSG: Using DirectX fullscreen-windowed driver!";

	if (fullscreenDriver != "")
		g_ConsoleMan.PrintString(fullscreenDriver);

	if (windowedDriver != "")
		g_ConsoleMan.PrintString(windowedDriver);

	if (g_NetworkServer.IsServerModeEnabled())
	{
		g_NetworkServer.Start();
		g_FrameMan.SetStoreNetworkBackBuffer(true);

		g_AudioMan.SetMultiplayerMode(true);
		g_AudioMan.SetSoundsVolume(0);
		g_AudioMan.SetMusicVolume(0);
	}

    InitMainMenu();
    if (g_SettingsMan.PlayIntro() && !g_NetworkServer.IsServerModeEnabled())
        PlayIntroTitle();

	// NETWORK Create multiplayer lobby activity to start as default if server is running
	if (g_NetworkServer.IsServerModeEnabled())
	{
		EnterMultiplayerLobby();
	}

    // If we fail to start/reset the activity, then revert to the intro/menu
    if (!ResetActivity())
        PlayIntroTitle();
	
    RunGameLoop();

    ///////////////////////////////////////////////////////////////////
    // Clean up

    // Save settings
	// We don't need to save settings every time the game ends, sometimes it corrupts settings
	// and the game fails to start
	//Writer writer("Base.rte/Settings.ini");
    //g_SettingsMan.Save(writer);

	g_NetworkClient.Destroy();
	g_NetworkServer.Destroy();

    g_MetaMan.Destroy();
    g_MovableMan.Destroy();
    g_SceneMan.Destroy();
    g_ActivityMan.Destroy();
    g_AudioMan.Destroy();
    g_PresetMan.Destroy();
    g_UInputMan.Destroy();
    g_FrameMan.Destroy();
    g_TimerMan.Destroy();
    g_SettingsMan.Destroy();
    g_LicenseMan.Destroy();
    g_LuaMan.Destroy();
    ContentFile::FreeAllLoaded();
    g_ConsoleMan.Destroy();

#if defined(STEAM_BUILD)
	g_SteamUGCMan.Destroy();
	g_AchievementMan.Destroy();
	SteamAPI_Shutdown();
#endif

#ifdef _DEBUG
    // Dump out the info about how well memory cleanup went
    Entity::ClassInfo::DumpPoolMemoryInfo(Writer("MemCleanupInfo.txt"));
#endif // _DEBUG

#ifdef SLICK_PROFILER
	ProfileSystem::Finalize();
	Debug::DebugTool::Finalize();
#endif // SLICK_PROFILER

#if defined(__APPLE__)
	OsxUtil::Destroy();
#endif // defined(__APPLE__)
	
    return 0;
}
END_OF_MAIN();
