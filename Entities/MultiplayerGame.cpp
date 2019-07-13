//////////////////////////////////////////////////////////////////////////////////////////
// File:            MultiplayerGame.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Project:         Retro Terrain Engine
// Author(s):       


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

// Without this nested includes somewhere deep inside Allegro will summon winsock.h and it will conflict with winsock2.h from RakNet
// and we can't move "Network.h" here because for whatever reasons everything will collapse
#define WIN32_LEAN_AND_MEAN

#include "MultiplayerGame.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "LicenseMan.h"
#include "SettingsMan.h"
#include "SLTerrain.h"
#include "Controller.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"
#include "Scene.h"
#include "DataModule.h"
#include "AudioMan.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUITab.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"
#include "GUI/GUIComboBox.h"

#include "MultiplayerGameGUI.h"
#include "PieMenuGUI.h"

#include "NetworkClient.h"


extern bool g_ResetActivity;
extern bool g_InActivity;

using namespace std;

namespace RTE
{
	CONCRETECLASSINFO(MultiplayerGame, Activity, 0)

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Clear
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Clears all the member variables of this MultiplayerGame, effectively
	//                  resetting the members of this abstraction level only.

	void MultiplayerGame::Clear()
	{
		m_pGUIController = 0;
		m_pGUIInput = 0;
		m_pGUIScreen = 0;

		m_pServerNameTextBox = 0;
		m_pPlayerNameTextBox = 0;
		m_pConnectButton = 0;

		m_pNATServiceServerNameTextBox = 0;
		m_pNATServerNameTextBox = 0;
		m_pNATServerPasswordTextBox = 0;
		m_pConnectNATButton = 0;

		m_pStatusLabel = 0;

		m_Mode = SETUP;

		m_LastMusic = "";
		m_LastMusicPos = 0;

		// Sounds
		m_EnterMenuSound.Reset();
		m_ExitMenuSound.Reset();
		m_ExitMenuAltSound.Reset();
		m_FocusChangeSound.Reset();
		m_SelectionChangeSound.Reset();
		m_ButtonPressSound.Reset();
		m_BackButtonPressSound.Reset();
		m_ItemChangeSound.Reset();
		m_TestSound.Reset();
		m_UserErrorSound.Reset();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the MultiplayerGame object ready for use.

	int MultiplayerGame::Create()
	{
		if (Activity::Create() < 0)
			return -1;

		// Load pie menu sounds so they could be played when their network events arrive
		// We don't care about the sound object, only the underlying FileContent objects it loads during creation
		Sound s;
		s.Create("Base.rte/GUIs/Sounds/PieMenuEnter1.wav", false);
		s.Create("Base.rte/GUIs/Sounds/PieMenuExit1.wav", false);
		s.Create("Base.rte/GUIs/Sounds/Blip01.wav", false);
		s.Create("Base.rte/GUIs/Sounds/Blip02.wav", false);
		s.Create("Base.rte/GUIs/Sounds/SlicePicked2.wav", false);
		s.Create("Base.rte/GUIs/Sounds/PieMenuExit1.wav", false);

		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Creates a MultiplayerGame to be identical to another, by deep copy.

	int MultiplayerGame::Create(const MultiplayerGame &reference)
	{
		if (Activity::Create(reference) < 0)
			return -1;

		if (m_Description.empty())
			m_Description = "Edit this Scene, including placement of all terrain objects and movable objects, AI blueprints, etc.";

		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  ReadProperty
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Reads a property value from a reader stream. If the name isn't
	//                  recognized by this class, then ReadProperty of the parent class
	//                  is called. If the property isn't recognized by any of the base classes,
	//                  false is returned, and the reader's position is untouched.

	int MultiplayerGame::ReadProperty(std::string propName, Reader &reader)
	{
		// See if the base class(es) can find a match instead
		return Activity::ReadProperty(propName, reader);
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Save
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Saves the complete state of this MultiplayerGame with a Writer for
	//                  later recreation with Create(Reader &reader);

	int MultiplayerGame::Save(Writer &writer) const
	{
		Activity::Save(writer);
		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Destroy
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Destroys and resets (through Clear()) the MultiplayerGame object.

	void MultiplayerGame::Destroy(bool notInherited)
	{
		g_FrameMan.SetDrawNetworkBackBuffer(false);
		g_NetworkClient.Disconnect();

		delete m_pGUIController;
		delete m_pGUIInput;
		delete m_pGUIScreen;

		if (!notInherited)
			Activity::Destroy();
		Clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Start
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Officially starts this. Creates all the data etc necessary to start
	//                  the activity.

	int MultiplayerGame::Start()
	{
		int error = Activity::Start();

		g_AudioMan.ClearMusicQueue();
		g_AudioMan.StopMusic();

		if (!m_pGUIScreen)
			m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer8());
		if (!m_pGUIInput)
			m_pGUIInput = new AllegroInput(-1, true);
		if (!m_pGUIController)
			m_pGUIController = new GUIControlManager();
		if (!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/Base"))
			DDTAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Base");

		m_pGUIController->Load("Base.rte/GUIs/MultiplayerGameGUI.ini");
		m_pGUIController->EnableMouse(true);

		// Resize the invisible root container so it matches the screen rez
		GUICollectionBox *pRootBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"));
		if (pRootBox)
			pRootBox->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

		GUICollectionBox *pDialogBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ConnectDialogBox"));
		if (pDialogBox)
		{
			pDialogBox->SetPositionAbs(g_FrameMan.GetResX() / 2 - pDialogBox->GetWidth() / 2, g_FrameMan.GetResY() / 2 - pDialogBox->GetHeight() / 2);
		}

		m_pServerNameTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("ServerNameTB"));
		m_pPlayerNameTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("PlayerNameTB"));
		m_pConnectButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ConnectButton"));

		m_pNATServiceServerNameTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("NATServiceNameTB"));
		m_pNATServerNameTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("NATServiceServerNameTB"));
		m_pNATServerPasswordTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("NATServiceServerPasswordTB"));
		m_pConnectNATButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ConnectNATButton"));

		m_pStatusLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("StatusLabel"));

		m_pServerNameTextBox->SetText(g_SettingsMan.GetNetworkServerName());
		m_pPlayerNameTextBox->SetText(g_SettingsMan.GetPlayerNetworkName());

		m_pNATServiceServerNameTextBox->SetText(g_SettingsMan.GetNATServiceAddress());
		m_pNATServerNameTextBox->SetText(g_SettingsMan.GetNATServerName());
		m_pNATServerPasswordTextBox->SetText(g_SettingsMan.GetNATServerPassword());


		m_EnterMenuSound.Create("Base.rte/GUIs/Sounds/MenuEnter.wav", false);
		m_ExitMenuSound.Create("Base.rte/GUIs/Sounds/MenuExit.wav", false);
		m_ExitMenuSound.AddSample("Base.rte/GUIs/Sounds/MenuExitB.wav");
		m_ExitMenuAltSound.Create("Base.rte/GUIs/Sounds/MenuExitB.wav", false);
		m_FocusChangeSound.Create("Base.rte/GUIs/Sounds/Blip00.wav", false);
		m_SelectionChangeSound.Create("Base.rte/GUIs/Sounds/Blip02.wav", false);
		m_ButtonPressSound.Create("Base.rte/GUIs/Sounds/Splip01.wav", false);
		m_BackButtonPressSound.Create("Base.rte/GUIs/Sounds/Splip02.wav", false);
		m_ItemChangeSound.Create("Base.rte/GUIs/Sounds/Click00.wav", false);
		m_TestSound.Create("Base.rte/GUIs/Sounds/Click01.wav", false);
		m_UserErrorSound.Create("Base.rte/GUIs/Sounds/Error.wav", false);

		return error;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Pause
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Pauses and unpauses the game.

	void MultiplayerGame::Pause(bool pause)
	{
		// Override the pause
		//m_Paused = false;
		Activity::Pause(pause);

		if (pause)
		{
			if (g_AudioMan.IsMusicPlaying())
			{
				m_LastMusic = g_AudioMan.GetMusicPath();
				m_LastMusicPos = g_AudioMan.GetMusicPosition();
			}
		}
		else
		{
			if (m_LastMusic != "")
			{
				g_AudioMan.PlayMusic(m_LastMusic.c_str());
				g_AudioMan.SetMusicPosition(m_LastMusicPos);
			}

			if (m_Mode == GAMEPLAY)
			{
				g_UInputMan.TrapMousePos(true, 0);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          End
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Forces the current game's end.

	void MultiplayerGame::End()
	{
		Activity::End();

		m_ActivityState = OVER;
		g_FrameMan.SetDrawNetworkBackBuffer(false);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Update
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Updates the state of this MultiplayerGame. Supposed to be done every frame
	//                  before drawing.

	void MultiplayerGame::Update()
	{
		Activity::Update();

		/////////////////////////////////////////////////////
		// Update the editor interface
		if (m_Mode == SETUP)
		{
			m_pGUIController->Update();

			////////////////////////////////////////////////////////
			// Handle events for mouse input on the controls

			bool toExit = false;

			GUIEvent anEvent;
			while (m_pGUIController->GetEvent(&anEvent))
			{
				if (anEvent.GetType() == GUIEvent::Command)
				{
					if (anEvent.GetControl() == m_pConnectButton)
					{
						std::string serverName;
						int port;

						string::size_type portPos = string::npos;

						portPos = m_pServerNameTextBox->GetText().find(":");
						if (portPos != string::npos)
						{
							serverName = m_pServerNameTextBox->GetText().substr(0, portPos);
							std::string portStr = m_pServerNameTextBox->GetText().substr(portPos + 1, m_pServerNameTextBox->GetText().length() - 2);

							port = atoi(portStr.c_str());
							if (port == 0)
								port = 8000;
						}
						else 
						{
							serverName = m_pServerNameTextBox->GetText();
							port = 8000;
						}

						std::string playerName = m_pPlayerNameTextBox->GetText();
						if (playerName == "")
							playerName = "Unnamed Player";

						g_NetworkClient.Connect(serverName, port, playerName);
						bool saveSettings = false;
						
						if (g_SettingsMan.GetPlayerNetworkName() != m_pPlayerNameTextBox->GetText())
						{
							g_SettingsMan.SetPlayerNetworkName(m_pPlayerNameTextBox->GetText());
							saveSettings = true;
						}

						if (g_SettingsMan.GetNetworkServerName() != m_pServerNameTextBox->GetText())
						{
							g_SettingsMan.SetNetworkServerName(m_pServerNameTextBox->GetText());
							saveSettings = true;
						}

						if (saveSettings)
						{
							Writer w("Base.rte/Settings.ini");
							g_SettingsMan.Save(w);
						}

						m_pGUIController->EnableMouse(false);
						m_Mode = CONNECTION;
						m_ConnectionWaitTimer.Reset();
						m_ButtonPressSound.Play();
					}


					if (anEvent.GetControl() == m_pConnectNATButton)
					{
						std::string serverName;
						int port;

						string::size_type portPos = string::npos;

						portPos = m_pNATServiceServerNameTextBox->GetText().find(":");
						if (portPos != string::npos)
						{
							serverName = m_pNATServiceServerNameTextBox->GetText().substr(0, portPos);
							std::string portStr = m_pNATServiceServerNameTextBox->GetText().substr(portPos + 1, m_pNATServiceServerNameTextBox->GetText().length() - 2);

							port = atoi(portStr.c_str());
							if (port == 0)
								port = 61111;
						}
						else
						{
							serverName = m_pNATServiceServerNameTextBox->GetText();
							port = 61111;
						}

						std::string playerName = m_pPlayerNameTextBox->GetText();
						if (playerName == "")
							playerName = "Unnamed Player";

						g_NetworkClient.PerformNATPunchThrough(serverName, port, playerName, m_pNATServerNameTextBox->GetText(), m_pNATServerPasswordTextBox->GetText());
						bool saveSettings = false;

						if (g_SettingsMan.GetPlayerNetworkName() != m_pPlayerNameTextBox->GetText())
						{
							g_SettingsMan.SetPlayerNetworkName(m_pPlayerNameTextBox->GetText());
							saveSettings = true;
						}

						if (g_SettingsMan.GetNATServiceAddress() != m_pNATServiceServerNameTextBox->GetText())
						{
							g_SettingsMan.SetNATServiceAddress(m_pNATServiceServerNameTextBox->GetText());
							saveSettings = true;
						}

						if (g_SettingsMan.GetNATServerName() != m_pNATServerNameTextBox->GetText())
						{
							g_SettingsMan.SetNATServerName(m_pNATServerNameTextBox->GetText());
							saveSettings = true;
						}

						if (g_SettingsMan.GetNATServerPassword() != m_pNATServerPasswordTextBox->GetText())
						{
							g_SettingsMan.SetNATServerPassword(m_pNATServerPasswordTextBox->GetText());
							saveSettings = true;
						}

						if (saveSettings)
						{
							Writer w("Base.rte/Settings.ini");
							g_SettingsMan.Save(w);
						}

						m_pGUIController->EnableMouse(false);
						m_Mode = CONNECTION;
						m_ConnectionWaitTimer.Reset();
						m_ButtonPressSound.Play();
					}
				}
				// Notifications
				else if (anEvent.GetType() == GUIEvent::Notification)
				{
				}
			}
		}

		if (m_Mode == CONNECTION)
		{
			if (g_NetworkClient.IsConnectedAndRegistred())
				m_Mode = GAMEPLAY;

			if (m_ConnectionWaitTimer.IsPastRealMS(8000))
			{
				g_NetworkClient.Disconnect();
				m_Mode = SETUP;
				m_pStatusLabel->SetText("Connection failed. Check console for error messages.");
				m_pGUIController->EnableMouse(true);
			}
		}

		if (m_Mode == GAMEPLAY)
		{
			g_UInputMan.TrapMousePos(true, 0);
			g_FrameMan.SetDrawNetworkBackBuffer(true);
			m_pGUIController->EnableMouse(false);

			if (!g_NetworkClient.IsConnectedAndRegistred())
			{
				//g_ActivityMan.EndActivity();
				//g_ResetActivity = true;
				m_Mode = SETUP;
				m_pGUIController->EnableMouse(true);
				g_UInputMan.TrapMousePos(false, 0);
				g_FrameMan.SetDrawNetworkBackBuffer(false);
			}
		}

		/*if (g_UInputMan.ElementHeld(0, UInputMan::INPUT_FIRE))
			g_FrameMan.SetScreenText("FIRE", 0, 0, -1, false);
		else
			g_FrameMan.SetScreenText("-", 0, 0, -1, false);*/

		/*if (g_InActivity)
			g_FrameMan.SetScreenText("IN ACITVITY", 0, 0, -1, false);
		else
			g_FrameMan.SetScreenText("-", 0, 0, -1, false);*/

		g_NetworkClient.Update();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          DrawGUI
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

	void MultiplayerGame::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
	{
		if (m_pGUIController)
		{
			AllegroScreen drawScreen(pTargetBitmap);
			m_pGUIController->Draw(&drawScreen);
			if (m_Mode == SETUP)
				m_pGUIController->DrawMouse();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Draw
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Draws this MultiplayerGame's current graphical representation to a
	//                  BITMAP of choice. This includes all game-related graphics.

	void MultiplayerGame::Draw(BITMAP* pTargetBitmap, const Vector &targetPos)
	{
		Activity::Draw(pTargetBitmap, targetPos);
	}

} // namespace RTE
