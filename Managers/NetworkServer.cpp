#include "NetworkServer.h"

#include "GUI/GUI.h"
#include "GUI/GUIInput.h"
#include "AllegroBitmap.h"

#include "Scene.h"
#include "SLTerrain.h"
#include "GameActivity.h"

#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "UInputMan.h"
#include "TimerMan.h"
#include "AudioMan.h"

#include "RakNetStatistics.h"
#include "RakSleep.h"

#include <lz4.h>
#include <lz4hc.h>

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::BackgroundSendThreadFunction(NetworkServer *server, short player) {
		const int sleepTime = 1000000 / server->m_EncodingFps;
		while (server->IsServerModeEnabled() && server->IsPlayerConnected(player)) {
			if (server->NeedToSendSceneSetupData(player) && server->IsSceneAvailable(player)) {
				server->SendSceneSetupData(player);
			}
			if (server->NeedToSendSceneData(player) && server->IsSceneAvailable(player)) {
				server->ClearTerrainChangeQueue(player);
				server->SendSceneData(player);
			}
			if (server->SendFrameData(player)) {
				server->SendFrame(player);
				std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
			}
			server->UpdateStats(player);
		}
		server->SetThreadExitReason(player, NetworkServer::THREAD_FINISH);

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::Clear() {
		m_SleepWhenIdle = false;
		m_SimSleepWhenIdle = false;

		for (short i = 0; i < c_MaxClients; i++) {
			m_BackBuffer8[i] = 0;
			m_BackBufferGUI8[i] = 0;

			m_LastFrameSentTime[i] = 0;
			m_LastStatResetTime[i] = 0;

			m_DelayedFrames[i] = 0;
			m_MsecPerFrame[i] = 0;
			m_MsecPerSendCall[i] = 0;

			m_LZ4CompressionState[i] = 0;
			m_LZ4FastCompressionState[i] = 0;

			m_MouseState1[i] = 0;
			m_MouseState2[i] = 0;
			m_MouseState3[i] = 0;

			m_MouseEvent1[i] = 0;
			m_MouseEvent2[i] = 0;
			m_MouseEvent3[i] = 0;

			m_SendEven[i] = false;

			m_ThreadExitReason[i] = 0;
			m_MSecsToSleep[i] = 0;

			// Set to send scene setup data by default
			m_SendSceneSetupData[i] = false;
			m_SendSceneData[i] = false;
			m_SceneAvailable[i] = false;
			m_SendFrameData[i] = false;

			m_EndActivityVotes[i] = false;
			m_RestartActivityVotes[i] = false;

			m_FrameNumbers[i] = 0;

			m_Ping[i] = 0;
			m_PingTimer[i].Reset();

			ClearInputMessages(i);
		}

		for (short i = 0; i < MAX_STAT_RECORDS; i++) {
			m_FramesSent[i] = 0;
			m_FramesSkipped[i] = 0;

			for (int j = 0; j < 2; j++) {
				m_DataUncompressedCurrent[i][j] = 0;
				m_DataSentCurrent[i][j] = 0;
				m_FrameDataSentCurrent[i][j] = 0;
				m_PostEffectDataSentCurrent[i][j] = 0;
				m_SoundDataSentCurrent[i][j] = 0;
				m_TerrainDataSentCurrent[i][j] = 0;
				m_OtherDataSentCurrent[i][j] = 0;
			}

			m_FrameDataSentTotal[i] = 0;
			m_PostEffectDataSentTotal[i] = 0;
			m_TerrainDataSentTotal[i] = 0;
			m_OtherDataSentTotal[i] = 0;

			m_DataUncompressedTotal[i] = 0;
			m_DataSentTotal[i] = 0;

			m_SendBufferBytes[i] = 0;
			m_SendBufferMessages[i] = 0;

			m_EmptyBlocks[i] = 0;
			m_FullBlocks[i] = 0;
		}

		m_UseHighCompression = true;
		m_UseFastCompression = false;
		m_HighCompressionLevel = LZ4HC_CLEVEL_OPT_MIN;
		m_FastAccelerationFactor = 1;
		m_UseInterlacing = false;
		m_EncodingFps = 30;
		m_ShowInput = false;
		m_ShowStats = false;
		m_TransmitAsBoxes = true;
		m_BoxWidth = 32;
		m_BoxHeight = 44;
		m_UseNATService = false;
		m_NatServerConnected = false;
		m_LastPackedReceived.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int NetworkServer::Initialize() {
		m_IsInServerMode = false;
		m_ServerPort = "";
		m_Server = RakNet::RakPeerInterface::GetInstance();

		for (int i = 0; i < c_MaxClients; i++) {
			m_ClientConnections[i].InternalId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
			m_ClientConnections[i].ClientId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
			m_ClientConnections[i].IsActive = false;
			m_ClientConnections[i].SendThread = 0;

			m_LZ4CompressionState[i] = malloc(LZ4_sizeofStateHC());
			m_LZ4FastCompressionState[i] = malloc(LZ4_sizeofState());
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::Destroy() {
		//Send a signal that server is going to shutdown
		m_IsInServerMode = false;
		// Wait for thread to shut down
		RakSleep(250);
		m_Server->Shutdown(300);
		// We're done with the network
		RakNet::RakPeerInterface::DestroyInstance(m_Server);

		for (short i = 0; i < c_MaxClients; i++) {
			DestroyBackBuffer(i);

			if (m_LZ4CompressionState[i]) { free(m_LZ4CompressionState[i]); }		
			m_LZ4CompressionState[i] = 0;

			if (m_LZ4FastCompressionState[i]) { free(m_LZ4FastCompressionState[i]); }		
			m_LZ4FastCompressionState[i] = 0;
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool NetworkServer::ReadyForSimulation() {
		short playersReady = 0;
		short playersTotal = 0;

		for (short player = 0; player < c_MaxClients; player++) {
			if (IsPlayerConnected(player)) { playersTotal++; }
			if (SendFrameData(player)) { playersReady++; }
		}

		return playersReady >= playersTotal;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SetServerPort(const std::string &newPort) {
		bool useDefault = false;
		for (const char &stringChar : newPort) {
			if (!std::isdigit(stringChar)) {
				g_ConsoleMan.PrintString("ERROR: Invalid port passed into \"-server\" argument, using default (8000) instead!");
				useDefault = true;
				break;
			}
		}
		m_ServerPort = useDefault ? "8000" : newPort;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::Start() {
		RakNet::SocketDescriptor socketDescriptors[1];
		socketDescriptors[0].port = atoi(m_ServerPort.c_str());
		socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4

		bool ok = m_Server->Startup(4, socketDescriptors, 1) == RakNet::RAKNET_STARTED;

		if (!ok) {
			g_ConsoleMan.PrintString("SERVER: Failed to start. Terminating!");
			exit(1);
		} else {
			m_Server->SetMaximumIncomingConnections(4);
			g_ConsoleMan.PrintString("SERVER: STARTED!");
		}

		if (m_UseNATService) {
			g_ConsoleMan.PrintString("SERVER: Connecting to NAT service");

			std::string serverName;
			int port;

			string::size_type portPos = std::string::npos;
			portPos = g_SettingsMan.GetNATServiceAddress().find(":");

			if (portPos != std::string::npos) {
				serverName = g_SettingsMan.GetNATServiceAddress().substr(0, portPos);
				std::string portStr = g_SettingsMan.GetNATServiceAddress().substr(portPos + 1, g_SettingsMan.GetNATServiceAddress().length() - 2);
				port = atoi(portStr.c_str());
				if (port == 0) { port = 61111; }
			} else {
				serverName = g_SettingsMan.GetNATServiceAddress();
				port = 61111;
			}

			m_NATServiceServerID = ConnectBlocking(m_Server, serverName.c_str(), port);

			if (m_NATServiceServerID != RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
				m_Server->AttachPlugin(&m_NATPunchthroughClient);
				SendNATServerRegistrationMsg(m_NATServiceServerID);
			}
		}
		m_Server->SetOccasionalPing(true);
		m_Server->SetUnreliableTimeout(50);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::LockScene(bool isLocked) {
		for (int i = 0; i < c_MaxClients; i++) {
			if (isLocked) {
				m_SceneLock[i].lock();
			} else {
				m_SceneLock[i].unlock();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ResetScene() {
		m_SceneID++;
		for (int i = 0; i < c_MaxClients; i++) {
			m_SendSceneSetupData[i] = true;
			m_SceneAvailable[i] = true;
			m_SendSceneData[i] = false;
			m_SendFrameData[i] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::RegisterTerrainChange(SceneMan::TerrainChange terrainChange) {
		if (m_IsInServerMode) {
			for (short player = 0; player < c_MaxClients; player++) {
				if (IsPlayerConnected(player)) {
					m_Mutex[player].lock();
					m_PendingTerrainChanges[player].push(terrainChange);
					m_Mutex[player].unlock();
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned char NetworkServer::GetPacketIdentifier(RakNet::Packet *packet) const {
		if (packet == 0) {
			return 255;
		}
		if (packet->data[0] == ID_TIMESTAMP) {
			RakAssert(packet->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
			return packet->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
		} else {
			return packet->data[0];
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ReceiveNewIncomingConnection(RakNet::Packet *packet) {
		std::string msg;
		RakNet::SystemAddress clientID;
		char buf[256];

		// Somebody connected.  We have their IP now
		msg = "SERVER: ID_NEW_INCOMING_CONNECTION from ";
		msg += packet->systemAddress.ToString(true);
		msg += " - ";
		msg += packet->guid.ToString();
		g_ConsoleMan.PrintString(msg);
		clientID = packet->systemAddress; // Record the player ID of the client

		RakNet::SystemAddress internalId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

		//g_ConsoleMan.PrintString("SERVER: Remote internal IDs:\n");
		for (int index = 0; index < MAXIMUM_NUMBER_OF_INTERNAL_IDS; index++) {
			internalId = m_Server->GetInternalID(packet->systemAddress, index);
			if (internalId != RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
				//g_ConsoleMan.PrintString(internalId.ToString(true));
			}
		}

		bool connected = false;

		// Store client connection data
		for (int index = 0; index < c_MaxClients; index++) {
			g_ConsoleMan.PrintString(m_ClientConnections[index].ClientId.ToString());

			if (m_ClientConnections[index].ClientId == RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
				m_ClientConnections[index].ClientId = clientID;
				m_ClientConnections[index].InternalId = internalId;

				connected = true;

				msg = "SERVER: Client connected as #";
				msg += std::to_string(index);
				g_ConsoleMan.PrintString(msg);
				break;
			}
		}
		if (!connected) { g_ConsoleMan.PrintString("SERVER: Could not accept connection"); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendAcceptedMsg(short player) {
		MsgAccepted msg;
		msg.Id = ID_SRV_ACCEPTED;
		m_Server->Send((const char *)&msg, sizeof(MsgAccepted), HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, m_ClientConnections[player].ClientId, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ReceiveDisconnection(RakNet::Packet *packet) {
		std::string msg = "ID_CONNECTION_LOST from";
		msg += packet->systemAddress.ToString(true);
		g_ConsoleMan.PrintString(msg);

		for (int index = 0; index < c_MaxClients; index++) {
			if (m_ClientConnections[index].ClientId == packet->systemAddress) {
				m_ClientConnections[index].IsActive = false;
				m_ClientConnections[index].ClientId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
				m_ClientConnections[index].InternalId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

				//delete m_ClientConnections[index].SendThread;
				m_ClientConnections[index].SendThread = 0;

				m_SendSceneSetupData[index] = true;
				m_SendSceneData[index] = false;
				m_SendFrameData[index] = false;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ReceiveRegisterMsg(RakNet::Packet *packet) {
		std::string msg;
		const MsgRegister *msgReg = (MsgRegister *)packet->data;
		char buf[32];

		msg = "SERVER: CLIENT REGISTRATION: RES ";
		msg += std::to_string(msgReg->ResolutionX);
		msg += " x ";
		msg += std::to_string(msgReg->ResolutionY);
		g_ConsoleMan.PrintString(msg);

		for (short index = 0; index < c_MaxClients; index++) {
			if (m_ClientConnections[index].ClientId == packet->systemAddress) {
				m_ClientConnections[index].ResX = msgReg->ResolutionX;
				m_ClientConnections[index].ResY = msgReg->ResolutionY;
				m_ClientConnections[index].IsActive = true;
				m_ClientConnections[index].PlayerName = msgReg->Name;
				g_FrameMan.CreateNewNetworkPlayerBackBuffer(index, msgReg->ResolutionX, msgReg->ResolutionY);

				m_Server->SetTimeoutTime(5000, m_ClientConnections[index].ClientId);

				m_ClientConnections[index].SendThread = new std::thread(BackgroundSendThreadFunction, this, index);
				SendAcceptedMsg(index);

				m_SendSceneSetupData[index] = true;
				m_SendSceneData[index] = false;
				m_SendFrameData[index] = false;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendNATServerRegistrationMsg(RakNet::SystemAddress address) {
		MsgRegisterServer msg;
		msg.Id = ID_NAT_SERVER_REGISTER_SERVER;

		strncpy(msg.ServerName, g_SettingsMan.GetNATServerName().c_str(), 62);
		strncpy(msg.ServerPassword, g_SettingsMan.GetNATServerPassword().c_str(), 62);
		RakNet::RakNetGUID guid = GetServerGUID();
		strncpy(msg.ServerGuid, guid.ToString(), 62);

		int payloadSize = sizeof(MsgSceneSetup);

		m_Server->Send((const char *)&msg, payloadSize, IMMEDIATE_PRIORITY, RELIABLE, 0, address, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ReceiveInputMsg(RakNet::Packet *packet) {
		const MsgInput *m = (MsgInput *)packet->data;

		int player = -1;

		for (int index = 0; index < c_MaxClients; index++) {
			if (m_ClientConnections[index].ClientId == packet->systemAddress) { player = index; }
		}

		if (player >= 0 && player < c_MaxClients) {
			// Copy message data
			MsgInput msg;

			msg.Id = m->Id;

			msg.MouseX = m->MouseX;
			msg.MouseY = m->MouseY;
			for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
				msg.MouseButtonPressed[i] = m->MouseButtonPressed[i];
				msg.MouseButtonReleased[i] = m->MouseButtonReleased[i];
				msg.MouseButtonHeld[i] = m->MouseButtonHeld[i];
			}
			msg.ResetActivityVote = m->ResetActivityVote;
			msg.RestartActivityVote = m->RestartActivityVote;

			msg.MouseWheelMoved = m->MouseWheelMoved;

			msg.InputElementPressed = m->InputElementPressed;
			msg.InputElementReleased = m->InputElementReleased;
			msg.InputElementHeld = m->InputElementHeld;

			bool skip = true;

			if (!m_InputMessages[player].empty()) {
				MsgInput lastmsg = m_InputMessages[player].back();

				if (msg.MouseX != lastmsg.MouseX) { skip = false; }
				if (msg.MouseY != lastmsg.MouseY) { skip = false; }

				for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
					if (msg.MouseButtonPressed[i] != lastmsg.MouseButtonPressed[i]) { skip = false; }
					if (msg.MouseButtonReleased[i] != lastmsg.MouseButtonReleased[i]) { skip = false; }
					if (msg.MouseButtonHeld[i] != lastmsg.MouseButtonHeld[i]) { skip = false; }
				}
				if (msg.ResetActivityVote != lastmsg.ResetActivityVote) { skip = false; }
				if (msg.RestartActivityVote != lastmsg.RestartActivityVote) { skip = false; }

				if (msg.MouseWheelMoved != lastmsg.MouseWheelMoved) { skip = false; }

				if (msg.InputElementPressed != lastmsg.InputElementPressed) { skip = false; }
				if (msg.InputElementReleased != lastmsg.InputElementReleased) { skip = false; }
				if (msg.InputElementHeld != lastmsg.InputElementHeld) { skip = false; }
			} else {
				skip = false;
			}

			if (!skip) { m_InputMessages[player].push(msg); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ProcessInputMsg(short player, MsgInput msg) {
		if (player >= 0 && player < c_MaxClients) {
			Vector input;
			input.m_X = msg.MouseX;
			input.m_Y = msg.MouseY;
			g_UInputMan.SetNetworkMouseMovement(player, input);

			g_UInputMan.SetNetworkMouseButtonHeldState(player, MOUSE_LEFT, msg.MouseButtonHeld[MOUSE_LEFT]);
			g_UInputMan.SetNetworkMouseButtonPressedState(player, MOUSE_LEFT, msg.MouseButtonPressed[MOUSE_LEFT]);
			g_UInputMan.SetNetworkMouseButtonReleasedState(player, MOUSE_LEFT, msg.MouseButtonReleased[MOUSE_LEFT]);

			m_MouseState1[player] = (msg.MouseButtonPressed[MOUSE_LEFT] || msg.MouseButtonHeld[MOUSE_LEFT]) ? 1 : 0;

			g_UInputMan.SetNetworkMouseButtonHeldState(player, MOUSE_RIGHT, msg.MouseButtonHeld[MOUSE_RIGHT]);
			g_UInputMan.SetNetworkMouseButtonPressedState(player, MOUSE_RIGHT, msg.MouseButtonPressed[MOUSE_RIGHT]);
			g_UInputMan.SetNetworkMouseButtonReleasedState(player, MOUSE_RIGHT, msg.MouseButtonReleased[MOUSE_RIGHT]);

			g_UInputMan.SetNetworkMouseButtonHeldState(player, MOUSE_MIDDLE, msg.MouseButtonHeld[MOUSE_MIDDLE]);
			g_UInputMan.SetNetworkMouseButtonPressedState(player, MOUSE_MIDDLE, msg.MouseButtonPressed[MOUSE_MIDDLE]);
			g_UInputMan.SetNetworkMouseButtonReleasedState(player, MOUSE_MIDDLE, msg.MouseButtonReleased[MOUSE_MIDDLE]);

			GUIInput::SetNetworkMouseButton(player, m_MouseState1[player], m_MouseState2[player], m_MouseState3[player]);

			g_UInputMan.SetNetworkMouseWheelState(player, msg.MouseWheelMoved);

			unsigned int bitMask = 0x1;

			// Store element states as bit flags
			for (int i = 0; i < INPUT_COUNT; i++) {
				bool val = (msg.InputElementHeld & bitMask) > 0;

				g_UInputMan.SetNetworkInputElementHeldState(player, i, val);
				g_UInputMan.SetNetworkInputElementPressedState(player, i, (msg.InputElementPressed & bitMask) > 0);
				g_UInputMan.SetNetworkInputElementReleasedState(player, i, (msg.InputElementReleased & bitMask) > 0);

				bitMask <<= 1;
			}

			m_EndActivityVotes[player] = msg.ResetActivityVote;
			m_RestartActivityVotes[player] = msg.RestartActivityVote;

			// We need to replace mouse input obtained from the allegro with mouse input obtained from network clients
			GUIInput::SetNetworkMouseMovement(player, msg.MouseX, msg.MouseY);
		} else {
			//g_ConsoleMan.PrintString("SERVER: Input for unknown client. Ignored.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ClearInputMessages(short player) {
		if (player >= 0 && player < c_MaxClients) {
			while (!m_InputMessages[player].empty()) {
				m_InputMessages[player].pop();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendSoundData(short player) {
		std::list<AudioMan::NetworkSoundData> events;
		g_AudioMan.GetSoundEvents(player, events);

		if (events.empty()) {
			return;
		}

		MsgSoundEvents *msg = (MsgSoundEvents *)m_PixelLineBuffer[player];
		AudioMan::NetworkSoundData *soundDataPointer = (AudioMan::NetworkSoundData *)((char *)msg + sizeof(MsgSoundEvents));
		msg->Id = ID_SRV_SOUND_EVENTS;
		msg->FrameNumber = m_FrameNumbers[player];
		msg->SoundEventsCount = 0;

		for (const AudioMan::NetworkSoundData &soundEvent : events) {
			if (sizeof(MsgSoundEvents) + (msg->SoundEventsCount * sizeof(AudioMan::NetworkSoundData)) <= c_MaxPixelLineBufferSize) {
				soundDataPointer->State = soundEvent.State;
				soundDataPointer->SoundFileHash = soundEvent.SoundFileHash;
				soundDataPointer->Channel = soundEvent.Channel;
				soundDataPointer->Immobile = soundEvent.Immobile;
				soundDataPointer->AttenuationStartDistance = soundEvent.AttenuationStartDistance;
				soundDataPointer->Loops = soundEvent.Loops;
				soundDataPointer->Priority = soundEvent.Priority;
				soundDataPointer->AffectedByGlobalPitch = soundEvent.AffectedByGlobalPitch;
				soundDataPointer->Position[0] = soundEvent.Position[0];
				soundDataPointer->Position[1] = soundEvent.Position[1];
				soundDataPointer->Volume = soundEvent.Volume;
				soundDataPointer->Pitch = soundEvent.Pitch;
				soundDataPointer->FadeOutTime = soundEvent.FadeOutTime;

				msg->SoundEventsCount++;
				soundDataPointer++;
			} else {
				int payloadSize = sizeof(MsgSoundEvents) + (msg->SoundEventsCount * sizeof(AudioMan::NetworkSoundData));
				m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

				m_SoundDataSentCurrent[player][STAT_CURRENT] += payloadSize;
				m_SoundDataSentTotal[player] += payloadSize;
				m_DataSentTotal[player] += payloadSize;

				soundDataPointer = (AudioMan::NetworkSoundData *)((char *)msg + sizeof(MsgSoundEvents));
				msg->SoundEventsCount = 0;
			}
		}

		if (msg->SoundEventsCount > 0) {
			int payloadSize = sizeof(MsgSoundEvents) + (msg->SoundEventsCount * sizeof(AudioMan::NetworkSoundData));
			m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

			m_SoundDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_SoundDataSentTotal[player] += payloadSize;
			m_DataSentTotal[player] += payloadSize;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendMusicData(short player) {
		std::list<AudioMan::NetworkMusicData> events;
		g_AudioMan.GetMusicEvents(player, events);

		if (events.empty()) {
			return;
		}

		MsgMusicEvents *msg = (MsgMusicEvents *)m_PixelLineBuffer[player];
		AudioMan::NetworkMusicData *musicDataPointer = (AudioMan::NetworkMusicData *)((char *)msg + sizeof(MsgMusicEvents));

		msg->Id = ID_SRV_MUSIC_EVENTS;
		msg->FrameNumber = m_FrameNumbers[player];
		msg->MusicEventsCount = 0;

		for (const AudioMan::NetworkMusicData &musicEvent : events) {
			musicDataPointer->State = musicEvent.State;
			musicDataPointer->LoopsOrSilence = musicEvent.LoopsOrSilence;
			musicDataPointer->Pitch = musicEvent.Pitch;
			musicDataPointer->Position = musicEvent.Position;
			strncpy(musicDataPointer->Path, musicEvent.Path, 255);

			msg->MusicEventsCount++;
			musicDataPointer++;

			if (msg->MusicEventsCount >= 4) {
				int payloadSize = sizeof(MsgMusicEvents) + sizeof(AudioMan::NetworkMusicData) * msg->MusicEventsCount;
				m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);
				msg->MusicEventsCount = 0;
				musicDataPointer = (AudioMan::NetworkMusicData *)((char *)msg + sizeof(MsgMusicEvents));

				m_SoundDataSentCurrent[player][STAT_CURRENT] += payloadSize;
				m_SoundDataSentTotal[player] += payloadSize;

				m_DataSentTotal[player] += payloadSize;

			}
		}

		if (msg->MusicEventsCount > 0) {
			int payloadSize = sizeof(MsgMusicEvents) + sizeof(AudioMan::NetworkMusicData) * msg->MusicEventsCount;
			m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

			m_SoundDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_SoundDataSentTotal[player] += payloadSize;

			m_DataSentTotal[player] += payloadSize;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendSceneSetupData(short player) {
		MsgSceneSetup msgSceneSetup;
		msgSceneSetup.Id = ID_SRV_SCENE_SETUP;
		msgSceneSetup.SceneId = m_SceneID;
		msgSceneSetup.Width = static_cast<short>(g_SceneMan.GetSceneWidth());
		msgSceneSetup.Height = static_cast<short>(g_SceneMan.GetSceneHeight());
		msgSceneSetup.SceneWrapsX = g_SceneMan.SceneWrapsX();

		Scene *scene = g_SceneMan.GetScene();

		std::list<SceneLayer *> sceneLayers = scene->GetBackLayers();
		short index = 0;

		for (SceneLayer * &layer : sceneLayers) {
			// Recalculate layers internal values for this player
			layer->UpdateScrollRatiosForNetworkPlayer(player);
			msgSceneSetup.BackgroundLayers[index].BitmapHash = layer->GetBitmapHash();

			msgSceneSetup.BackgroundLayers[index].DrawTrans = layer->m_DrawTrans;
			msgSceneSetup.BackgroundLayers[index].OffsetX = layer->m_Offset.m_X;
			msgSceneSetup.BackgroundLayers[index].OffsetY = layer->m_Offset.m_Y;

			msgSceneSetup.BackgroundLayers[index].ScrollInfoX = layer->m_ScrollInfo.m_X;
			msgSceneSetup.BackgroundLayers[index].ScrollInfoY = layer->m_ScrollInfo.m_Y;
			msgSceneSetup.BackgroundLayers[index].ScrollRatioX = layer->m_ScrollRatio.m_X;
			msgSceneSetup.BackgroundLayers[index].ScrollRatioY = layer->m_ScrollRatio.m_Y;
			msgSceneSetup.BackgroundLayers[index].ScaleFactorX = layer->m_ScaleFactor.m_X;
			msgSceneSetup.BackgroundLayers[index].ScaleFactorY = layer->m_ScaleFactor.m_Y;
			msgSceneSetup.BackgroundLayers[index].ScaleInverseX = layer->m_ScaleInverse.m_X;
			msgSceneSetup.BackgroundLayers[index].ScaleInverseY = layer->m_ScaleInverse.m_Y;
			msgSceneSetup.BackgroundLayers[index].ScaledDimensionsX = layer->m_ScaledDimensions.m_X;
			msgSceneSetup.BackgroundLayers[index].ScaledDimensionsY = layer->m_ScaledDimensions.m_Y;

			msgSceneSetup.BackgroundLayers[index].WrapX = layer->m_WrapX;
			msgSceneSetup.BackgroundLayers[index].WrapY = layer->m_WrapY;

			msgSceneSetup.BackgroundLayers[index].FillLeftColor = layer->m_FillLeftColor;
			msgSceneSetup.BackgroundLayers[index].FillRightColor = layer->m_FillRightColor;
			msgSceneSetup.BackgroundLayers[index].FillUpColor = layer->m_FillUpColor;
			msgSceneSetup.BackgroundLayers[index].FillDownColor = layer->m_FillDownColor;

			index++;
			// Set everything back to what it was just in case
			layer->InitScrollRatios();
		}
		msgSceneSetup.BackgroundLayerCount = index;

		int payloadSize = sizeof(MsgSceneSetup);

		m_Server->Send((const char *)&msgSceneSetup, payloadSize, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, m_ClientConnections[player].ClientId, false);

		m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
		m_DataSentTotal[player] += payloadSize;

		m_OtherDataSentCurrent[player][STAT_CURRENT] += payloadSize;
		m_OtherDataSentTotal[player] += payloadSize;

		m_DataUncompressedCurrent[player][STAT_CURRENT] += payloadSize;
		m_DataUncompressedTotal[player] += payloadSize;

		m_SendSceneSetupData[player] = false;

		// While we're on the same thread with freshly connected player, send current music being played
		if (g_AudioMan.IsMusicPlaying()) {
			std::string currentMusic = g_AudioMan.GetMusicPath();
			if (!currentMusic.empty()) { g_AudioMan.RegisterMusicEvent(player, AudioMan::MUSIC_PLAY, currentMusic.c_str(), -1, g_AudioMan.GetMusicPosition(), 1.0F); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ReceiveSceneSetupDataAccepted(RakNet::Packet *packet) {
		short player = -1;

		for (short index = 0; index < c_MaxClients; index++) {
			if (m_ClientConnections[index].ClientId == packet->systemAddress) { player = index; }
		}

		if (player > -1) {
			m_SendSceneSetupData[player] = false;
			m_SendSceneData[player] = true;
			m_SendFrameData[player] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendSceneData(short player) {
		// Check for congestion
		RakNet::RakNetStatistics rns;

		MsgSceneLine *sceneData = (MsgSceneLine *)m_PixelLineBuffer[player];

		// Save message ID
		sceneData->Id = ID_SRV_SCENE;

		int lineX = 0;
		int lineY = 0;
		int lineWidth = 1280;

		Scene *scene = g_SceneMan.GetScene();
		SLTerrain *terrain = 0;

		if (scene) {
			terrain = scene->GetTerrain();
			if (!terrain) {
				return;
			}
		} else {
			return;
		}

		// Lock the scene until current bitmap is fully transfered
		m_SceneLock[player].lock();

		for (int layer = 0; layer < 2; layer++) {
			BITMAP *bmp = 0;
			if (layer == 0) {
				bmp = terrain->GetBGColorBitmap();
			} else if (layer == 1) {
				bmp = terrain->GetFGColorBitmap();
			}

			lock_bitmap(bmp);

			for (lineX = 0; ; lineX += lineWidth) {
				int width = lineWidth;
				if (lineX + width >= g_SceneMan.GetSceneWidth()) { width = g_SceneMan.GetSceneWidth() - lineX; }

				for (lineY = 0; lineY < g_SceneMan.GetSceneHeight(); lineY++) {
					// Save scene fragment data
					sceneData->DataSize = width;
					sceneData->UncompressedSize = width;
					sceneData->Layer = layer;
					sceneData->X = lineX;
					sceneData->Y = lineY;
					sceneData->SceneId = m_SceneID;

					// Compression section
					int result = 0;
					//bool lineIsEmpty = false;

					result = LZ4_compress_HC_extStateHC(m_LZ4CompressionState[player], (char *)bmp->line[lineY] + lineX, (char *)(m_PixelLineBuffer[player] + sizeof(MsgSceneLine)), width, width, LZ4HC_CLEVEL_MAX);

					// Compression failed or ineffective, send as is
					if (result == 0 || result == width) {
#ifdef _WIN32
						memcpy_s(m_PixelLineBuffer[player] + sizeof(MsgSceneLine), c_MaxPixelLineBufferSize, bmp->line[lineY] + lineX, width);
#else
						memcpy(m_PixelLineBuffer[player] + sizeof(MsgSceneLine), bmp->line[lineY] + lineX, width);
#endif
					} else {
						sceneData->DataSize = result;
					}

					int payloadSize = sceneData->DataSize + sizeof(MsgSceneLine);

					m_Server->Send((const char *)sceneData, payloadSize, HIGH_PRIORITY, RELIABLE, 0, m_ClientConnections[player].ClientId, false);

					m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
					m_DataSentTotal[player] += payloadSize;

					m_TerrainDataSentCurrent[player][STAT_CURRENT] += payloadSize;
					m_TerrainDataSentTotal[player] += payloadSize;

					m_DataUncompressedCurrent[player][STAT_CURRENT] += sceneData->UncompressedSize;
					m_DataUncompressedTotal[player] += sceneData->UncompressedSize;

					// Wait for the  messages to leave to avoid congestion
					if (lineY % 250 == 0) {
						do {
							m_Server->GetStatistics(m_ClientConnections[player].ClientId, &rns);

							m_SendBufferBytes[player] = (int)rns.bytesInSendBuffer[MEDIUM_PRIORITY] + (int)rns.bytesInSendBuffer[HIGH_PRIORITY];
							m_SendBufferMessages[player] = (int)rns.messageInSendBuffer[MEDIUM_PRIORITY] + (int)rns.messageInSendBuffer[HIGH_PRIORITY];

							RakSleep(25);
						} while (rns.messageInSendBuffer[HIGH_PRIORITY] > 1000 && IsPlayerConnected(player));

						if (!IsPlayerConnected(player)) {
							break;
						}
					}
				}
				if (lineX + lineWidth >= g_SceneMan.GetSceneWidth()) {
					break;
				}
			}
			release_bitmap(bmp);
		}

		m_SceneLock[player].unlock();

		m_SendSceneSetupData[player] = false;
		m_SendSceneData[player] = false;
		m_SendFrameData[player] = false;

		SendSceneEndMsg(player);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ClearTerrainChangeQueue(short player) {
		m_Mutex[player].lock();
		while (!m_PendingTerrainChanges[player].empty()) {
			m_PendingTerrainChanges[player].pop();
		}
		while (!m_CurrentTerrainChanges[player].empty()) {
			m_CurrentTerrainChanges[player].pop();
		}
		m_Mutex[player].unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool NetworkServer::NeedToProcessTerrainChanges(short player) {
		bool result;

		m_Mutex[player].lock();
		result = !m_PendingTerrainChanges[player].empty();
		m_Mutex[player].unlock();

		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ProcessTerrainChanges(short player) {
		m_Mutex[player].lock();
		while (!m_PendingTerrainChanges[player].empty()) {
			m_CurrentTerrainChanges[player].push(m_PendingTerrainChanges[player].front());
			m_PendingTerrainChanges[player].pop();
		}
		m_Mutex[player].unlock();

		while (!m_CurrentTerrainChanges[player].empty()) {
			int maxSize = 1280;

			SceneMan::TerrainChange terrainChange = m_CurrentTerrainChanges[player].front();
			m_CurrentTerrainChanges[player].pop();

			// Fragment region if it does not fit one packet
			if (terrainChange.w * terrainChange.h > maxSize) {
				int size = 0;
				int height = 1;
				int heightStart = 0;

				for (int y = 0; y < terrainChange.h; y++) {
					height++;
					size += terrainChange.w;

					// Store changed block if the size is over the MTU or if it's the last block.
					if (size + terrainChange.w >= maxSize || y == terrainChange.h - 1) {
						SceneMan::TerrainChange tcf;
						tcf.x = terrainChange.x;
						tcf.y = terrainChange.y + heightStart;
						tcf.w = terrainChange.w;
						tcf.h = height;
						tcf.back = terrainChange.back;
						tcf.color = terrainChange.color;

						SendTerrainChangeMsg(player, tcf);

						size = 0;
						heightStart = y;
						height = 1;
					}
				}
			} else {
				SendTerrainChangeMsg(player, terrainChange);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendTerrainChangeMsg(short player, SceneMan::TerrainChange terrainChange) {
		if (terrainChange.w == 1 && terrainChange.h == 1) {
			MsgTerrainChange msg;
			msg.Id = ID_SRV_TERRAIN;
			msg.X = terrainChange.x;
			msg.Y = terrainChange.y;
			msg.W = terrainChange.w;
			msg.H = terrainChange.h;
			msg.DataSize = 0;
			msg.UncompressedSize = 0;
			msg.SceneId = m_SceneID;
			msg.Color = terrainChange.color;
			msg.Back = terrainChange.back;

			int payloadSize = sizeof(MsgTerrainChange);

			m_Server->Send((const char *)&msg, payloadSize, MEDIUM_PRIORITY, RELIABLE, 0, m_ClientConnections[player].ClientId, false);

			m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_DataSentTotal[player] += payloadSize;

			m_TerrainDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_TerrainDataSentTotal[player] += payloadSize;

			m_DataUncompressedCurrent[player][STAT_CURRENT] += payloadSize;
			m_DataUncompressedTotal[player] += payloadSize;
		} else {
			MsgTerrainChange *msg = (MsgTerrainChange *)m_PixelLineBuffer[player];
			msg->Id = ID_SRV_TERRAIN;
			msg->X = terrainChange.x;
			msg->Y = terrainChange.y;
			msg->W = terrainChange.w;
			msg->H = terrainChange.h;
			int size = msg->W * msg->H;
			msg->DataSize = size;
			msg->UncompressedSize = size;
			msg->SceneId = m_SceneID;
			msg->Color = terrainChange.color;
			msg->Back = terrainChange.back;

			Scene * scene = g_SceneMan.GetScene();
			SLTerrain * terrain = scene->GetTerrain();

			const BITMAP *bmp = 0;
			bmp = msg->Back ? terrain->GetBGColorBitmap() : terrain->GetFGColorBitmap();

			unsigned char *dest = (unsigned char *)(m_TerrainChangeBuffer[player]);

			// Copy bitmap data
			for (int y = 0; y < msg->H && msg->Y + y < bmp->h; y++) {
				memcpy(dest, bmp->line[msg->Y + y] + msg->X, msg->W);
				dest += msg->W;
			}

			int result = 0;

			result = LZ4_compress_HC_extStateHC(m_LZ4CompressionState[player], (char *)m_TerrainChangeBuffer[player], (char *)(m_PixelLineBuffer[player] + sizeof(MsgTerrainChange)), size, size, LZ4HC_CLEVEL_OPT_MIN);

			// Compression failed or ineffective, send as is
			if (result == 0 || result == size) {
#ifdef _WIN32
				memcpy_s(m_PixelLineBuffer[player] + sizeof(MsgTerrainChange), c_MaxPixelLineBufferSize, m_TerrainChangeBuffer[player], size);
#else
				memcpy(m_PixelLineBuffer[player] + sizeof(MsgTerrainChange), m_TerrainChangeBuffer[player], size);
#endif
			} else {
				msg->DataSize = result;
			}

			int payloadSize = sizeof(MsgTerrainChange) + msg->DataSize;

			m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE, 0, m_ClientConnections[player].ClientId, false);

			m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_DataSentTotal[player] += payloadSize;

			m_TerrainDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_TerrainDataSentTotal[player] += payloadSize;

			m_DataUncompressedCurrent[player][STAT_CURRENT] += msg->UncompressedSize;
			m_DataUncompressedTotal[player] += msg->UncompressedSize;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::ReceiveSceneAcceptedMsg(RakNet::Packet *packet) {
		for (short player = 0; player < c_MaxClients; player++) {
			if (m_ClientConnections[player].ClientId == packet->systemAddress) { m_SendFrameData[player] = true; }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendSceneEndMsg(short player) {
		MsgSceneEnd msg;
		msg.Id = ID_SRV_SCENE_END;
		m_Server->Send((const char *)&msg, sizeof(MsgSceneSetup), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::CreateBackBuffer(short player, int w, int h) {
		m_BackBuffer8[player] = create_bitmap_ex(8, w, h);
		m_BackBufferGUI8[player] = create_bitmap_ex(8, w, h);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::DestroyBackBuffer(short player) {
		if (m_BackBuffer8) { destroy_bitmap(m_BackBuffer8[player]); }
		m_BackBuffer8[player] = 0;

		if (m_BackBufferGUI8) { destroy_bitmap(m_BackBufferGUI8[player]); }
		m_BackBufferGUI8[player] = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendFrameSetupMsg(short player) {
		MsgFrameSetup msgFrameSetup;
		msgFrameSetup.Id = ID_SRV_FRAME_SETUP;
		msgFrameSetup.FrameNumber = m_FrameNumbers[player];
		msgFrameSetup.TargetPosX = g_FrameMan.GetTargetPos(player).m_X;
		msgFrameSetup.TargetPosY = g_FrameMan.GetTargetPos(player).m_Y;

		for (int i = 0; i < c_MaxLayersStoredForNetwork; i++) {
			msgFrameSetup.OffsetX[i] = g_FrameMan.SLOffset[player][i].m_X;
			msgFrameSetup.OffsetY[i] = g_FrameMan.SLOffset[player][i].m_Y;
		}

		int payloadSize = sizeof(MsgSceneSetup);

		m_Server->Send((const char *)&msgFrameSetup, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

		m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
		m_DataSentTotal[player] += payloadSize;

		m_FrameDataSentCurrent[player][STAT_CURRENT] += payloadSize;
		m_FrameDataSentTotal[player] += payloadSize;

		m_DataUncompressedCurrent[player][STAT_CURRENT] += payloadSize;
		m_DataUncompressedTotal[player] += payloadSize;

		m_SendSceneData[player] = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::SendPostEffectData(short player) {
		std::list<PostEffect> effects;
		g_PostProcessMan.GetNetworkPostEffectsList(player, effects);

		if (effects.empty()) {
			return;
		}

		MsgPostEffects *msg = (MsgPostEffects *)m_PixelLineBuffer[player];
		PostEffectNetworkData *effDataPtr = (PostEffectNetworkData *)((char *)msg + sizeof(MsgPostEffects));

		msg->Id = ID_SRV_POST_EFFECTS;
		msg->FrameNumber = m_FrameNumbers[player];
		msg->PostEffectsCount = 0;

		for (const PostEffect postEffectEvent : effects) {
			effDataPtr->X = postEffectEvent.m_Pos.GetX();
			effDataPtr->Y = postEffectEvent.m_Pos.GetY();
			effDataPtr->BitmapHash = postEffectEvent.m_BitmapHash;
			effDataPtr->Strength = postEffectEvent.m_Strength;
			effDataPtr->Angle = postEffectEvent.m_Angle;

			msg->PostEffectsCount++;
			effDataPtr++;

			if (msg->PostEffectsCount >= 75) {
				int payloadSize = sizeof(MsgPostEffects) + sizeof(PostEffectNetworkData) * msg->PostEffectsCount;
				m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);
				msg->PostEffectsCount = 0;
				effDataPtr = (PostEffectNetworkData *)((char *)msg + sizeof(MsgPostEffects));

				m_PostEffectDataSentCurrent[player][STAT_CURRENT] += payloadSize;
				m_PostEffectDataSentTotal[player] += payloadSize;

				m_DataSentTotal[player] += payloadSize;
			}
		}

		if (msg->PostEffectsCount > 0) {
			//int header = sizeof(MsgPostEffects);
			//int data = sizeof(PostEffectNetworkData);
			//int total = header + data * msg->PostEffectsCount;
			//int sz = sizeof(size_t);

			int payloadSize = sizeof(MsgPostEffects) + sizeof(PostEffectNetworkData) * msg->PostEffectsCount;
			m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

			m_PostEffectDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_PostEffectDataSentTotal[player] += payloadSize;

			m_DataSentTotal[player] += payloadSize;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int NetworkServer::SendFrame(short player) {
		long long currentTicks = g_TimerMan.GetRealTickCount();
		double fps = static_cast<double>(m_EncodingFps);
		double secsPerFrame = 1.0 / fps;
		double secsSinceLastFrame = static_cast<double>(currentTicks - m_LastFrameSentTime[player]) / static_cast<double>(g_TimerMan.GetTicksPerSecond());

		// Fix for an overflow which may happen if server lags for a few seconds when loading activities
		if (secsSinceLastFrame < 0) { secsSinceLastFrame = secsPerFrame; }

		m_MsecPerFrame[player] = static_cast<int>(secsSinceLastFrame * 1000.0);

		m_LastFrameSentTime[player] = g_TimerMan.GetRealTickCount();

		// Wait till FrameMan releases bitmap
		SetThreadExitReason(player, NetworkServer::LOCKED);
		SetThreadExitReason(player, NetworkServer::NORMAL);

		// Get backbuffer bitmap for this player
		BITMAP *frameManBmp = g_FrameMan.GetNetworkBackBuffer8Ready(player);
		BITMAP *frameManGUIBmp = g_FrameMan.GetNetworkBackBufferGUI8Ready(player);

		if (!m_BackBuffer8[player]) {
			CreateBackBuffer(player, frameManBmp->w, frameManBmp->h);
		} else {
			// If for whatever reasons frameMans back buffer changed dimensions, recreate our internal backbuffer
			if (m_BackBuffer8[player]->w != frameManBmp->w || m_BackBuffer8[player]->h != frameManBmp->h) {
				DestroyBackBuffer(player);
				CreateBackBuffer(player, frameManBmp->w, frameManBmp->h);
				//g_ConsoleMan.PrintString("SERVER: Backbuffer recreated");
			}
		}

		m_FrameNumbers[player]++;
		if (m_FrameNumbers[player] >= c_FramesToRemember) { m_FrameNumbers[player] = 0; }

		// Save a copy of buffer to avoid tearing when the original is updated by frame man
		blit(frameManBmp, m_BackBuffer8[player], 0, 0, 0, 0, frameManBmp->w, frameManBmp->h);
		blit(frameManGUIBmp, m_BackBufferGUI8[player], 0, 0, 0, 0, frameManGUIBmp->w, frameManGUIBmp->h);

		SendFrameSetupMsg(player);
		SendPostEffectData(player);
		SendSoundData(player);
		SendMusicData(player);

		m_FramesSent[player]++;

		// Compression section
		int compressionMethod = m_HighCompressionLevel;
		int accelerationFactor = m_FastAccelerationFactor;

		m_SendEven[player] = !m_SendEven[player];

		if (m_TransmitAsBoxes) {
			MsgFrameBox *frameData = (MsgFrameBox *)m_PixelLineBuffer[player];
			frameData->FrameNumber = m_FrameNumbers[player];

			// Save message ID
			frameData->Id = ID_SRV_FRAME_BOX;
			frameData->BoxWidth = m_BoxWidth;
			frameData->BoxHeight = m_BoxHeight;

			int bw = m_BackBuffer8[player]->w / m_BoxWidth;
			int bh = m_BackBuffer8[player]->h / m_BoxHeight;

			for (int by = 0; by <= bh; by++) {
				int step = 1;
				int startLine = 0;

				if (m_UseInterlacing) {
					step = 2;
					if (m_SendEven[player]) {
						startLine = (by % 2 == 0) ? 1 : 0;
					} else {
						startLine = (by % 2 == 0) ? 0 : 1;
					}
				}

				for (int bx = startLine; bx <= bw; bx += step) {
					int bpx = bx * m_BoxWidth;
					int bpy = by * m_BoxHeight;

					if (bpx >= m_BackBuffer8[player]->w || bpy >= m_BackBuffer8[player]->h) {
						break;
					}

					frameData->BoxX = bpx;
					frameData->BoxY = bpy;

					int maxWidth = m_BoxWidth;
					if (bpx + m_BoxWidth >= m_BackBuffer8[player]->w) {
						maxWidth = m_BackBuffer8[player]->w - bpx;
						frameData->BoxWidth = maxWidth;
					}

					int maxHeight = m_BoxHeight;
					if (bpy + m_BoxHeight >= m_BackBuffer8[player]->h) {
						maxHeight = m_BackBuffer8[player]->h - bpy;
						frameData->BoxHeight = maxHeight;
					}

					int size = maxWidth * maxHeight;
					frameData->UncompressedSize = size;
					frameData->DataSize = size;

					for (int layer = 0; layer < 2; layer++) {
						bool boxIsEmpty = true;
						int line = 0;

						const BITMAP *backBuffer = 0;
						if (layer == 0) {
							backBuffer = m_BackBuffer8[player];
						} else if (layer == 1) {
							backBuffer = m_BackBufferGUI8[player];
						}

						frameData->Layer = layer;

						unsigned char *dest = (unsigned char *)(m_TerrainChangeBuffer[player]);

						// Copy block to line buffer and also check if block is empty
						for (line = 0; line < maxHeight; line++) {
							// Copy bitmap data
							memcpy(dest, backBuffer->line[bpy + line] + bpx, maxWidth);
							dest += maxWidth;
						}

						// Check if block is empty
						unsigned long *pixelInt = (unsigned long *)m_TerrainChangeBuffer[player];
						int counter = 0;

						for (counter = 0; counter < size; counter += sizeof(unsigned long)) {
							if (*pixelInt > 0) {
								boxIsEmpty = false;
								break;
							}
							pixelInt++;
						}
						if (boxIsEmpty && counter > size) {
							pixelInt--;
							counter -= sizeof(unsigned long);

							const unsigned char *pixelChr = (unsigned char *)pixelInt;
							for (; counter < size; counter++) {
								if (*pixelChr > 0) {
									boxIsEmpty = false;
									break;
								}
								pixelChr++;
							}
						}

						if (!boxIsEmpty) {
							int result = 0;

							if (m_UseHighCompression) {
								result = LZ4_compress_HC_extStateHC(m_LZ4CompressionState[player], (char *)m_TerrainChangeBuffer[player], (char *)(m_PixelLineBuffer[player] + sizeof(MsgFrameBox)), size, size, compressionMethod);
							} else if (m_UseFastCompression) {
								result = LZ4_compress_fast_extState(m_LZ4FastCompressionState[player], (char *)m_TerrainChangeBuffer[player], (char *)(m_PixelLineBuffer[player] + sizeof(MsgFrameBox)), size, size, accelerationFactor);
							}

							// Compression failed or ineffective, send as is
							if (result == 0 || result == backBuffer->w) {
#ifdef _WIN32
								memcpy_s(m_PixelLineBuffer[player] + sizeof(MsgFrameBox), c_MaxPixelLineBufferSize, m_TerrainChangeBuffer[player], size);
#else
								memcpy(m_PixelLineBuffer[player] + sizeof(MsgFrameBox), m_TerrainChangeBuffer[player], size);
#endif
							} else {
								frameData->DataSize = result;
							}

							m_FullBlocks[player]++;
						} else {
							frameData->DataSize = 0;
							m_EmptyBlocks[player]++;
						}

						int payloadSize = frameData->DataSize + sizeof(MsgFrameBox);

						m_Server->Send((const char *)frameData, payloadSize, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, m_ClientConnections[player].ClientId, false);

						m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
						m_DataSentTotal[player] += payloadSize;

						m_FrameDataSentCurrent[player][STAT_CURRENT] += payloadSize;
						m_FrameDataSentTotal[player] += payloadSize;

						m_DataUncompressedCurrent[player][STAT_CURRENT] += frameData->UncompressedSize;
						m_DataUncompressedTotal[player] += frameData->UncompressedSize;
					}
				}
			}
		} else {
			MsgFrameLine *frameData = (MsgFrameLine *)m_PixelLineBuffer[player];
			frameData->FrameNumber = m_FrameNumbers[player];

			// Save message ID
			frameData->Id = ID_SRV_FRAME_LINE;

			int startLine = 0;
			int step = 1;

			if (m_UseInterlacing) {
				step = 2;
				m_SendEven[player] = !m_SendEven[player];
				startLine = m_SendEven[player] ? 0 : 1;
			}

			for (int m_CurrentFrameLine = startLine; m_CurrentFrameLine < m_BackBuffer8[player]->h; m_CurrentFrameLine += step) {
				for (int layer = 0; layer < 2; layer++) {
					const BITMAP *backBuffer = 0;

					if (layer == 0) {
						backBuffer = m_BackBuffer8[player];
					} else if (layer == 1) {
						backBuffer = m_BackBufferGUI8[player];
					}

					// Save line number
					frameData->LineNumber = m_CurrentFrameLine;

					frameData->Layer = layer;

					// Save line length
					frameData->DataSize = backBuffer->w;
					frameData->UncompressedSize = backBuffer->w;

					int result = 0;
					bool lineIsEmpty = true;

					// Check if line is empty
					{
						unsigned long *pixelInt = (unsigned long *)backBuffer->line[m_CurrentFrameLine];
						int counter = 0;
						for (counter = 0; counter < backBuffer->w; counter += sizeof(unsigned long)) {
							if (*pixelInt > 0) {
								lineIsEmpty = false;
								break;
							}
							pixelInt++;
						}
						if (lineIsEmpty) {
							pixelInt--;
							counter -= sizeof(unsigned long);

							const unsigned char *pixelChr = (unsigned char *)pixelInt;
							for (; counter < backBuffer->w; counter++) {
								if (*pixelChr > 0) {
									lineIsEmpty = false;
									break;
								}
								pixelChr++;
							}
						}
					}

					if (!lineIsEmpty) {
						if (m_UseHighCompression) {
							result = LZ4_compress_HC_extStateHC(m_LZ4CompressionState[player], (char *)backBuffer->line[m_CurrentFrameLine], (char *)(m_PixelLineBuffer[player] + sizeof(MsgFrameLine)), backBuffer->w, backBuffer->w, compressionMethod);
						} else if (m_UseFastCompression) {
							result = LZ4_compress_fast_extState(m_LZ4FastCompressionState[player], (char *)backBuffer->line[m_CurrentFrameLine], (char *)(m_PixelLineBuffer[player] + sizeof(MsgFrameLine)), backBuffer->w, backBuffer->w, accelerationFactor);
						}

						// Compression failed or ineffective, send as is
						if (result == 0 || result == m_BackBuffer8[player]->w) {
#ifdef _WIN32
							memcpy_s(m_PixelLineBuffer[player] + sizeof(MsgFrameLine), c_MaxPixelLineBufferSize, backBuffer->line[m_CurrentFrameLine], backBuffer->w);
#else
							memcpy(m_PixelLineBuffer[player] + sizeof(MsgFrameLine), backBuffer->line[m_CurrentFrameLine], backBuffer->w);
#endif
						} else {
							frameData->DataSize = result;
						}

						m_FullBlocks[player]++;
					} else {
						frameData->DataSize = 0;
						m_EmptyBlocks[player]++;
					}

					int payloadSize = frameData->DataSize + sizeof(MsgFrameLine);

					m_Server->Send((const char *)frameData, payloadSize, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, m_ClientConnections[player].ClientId, false);

					m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
					m_DataSentTotal[player] += payloadSize;

					m_FrameDataSentCurrent[player][STAT_CURRENT] += payloadSize;
					m_FrameDataSentTotal[player] += payloadSize;

					m_DataUncompressedCurrent[player][STAT_CURRENT] += frameData->UncompressedSize;
					m_DataUncompressedTotal[player] += frameData->UncompressedSize;
				}
			}
		}
		ProcessTerrainChanges(player);

		double secsSinceSendStart = static_cast<double>(g_TimerMan.GetRealTickCount() - currentTicks) / static_cast<double>(g_TimerMan.GetTicksPerSecond());
		m_MsecPerSendCall[player] = static_cast<int>(secsSinceSendStart * 1000.0);

		SetThreadExitReason(player, NetworkServer::NORMAL);
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::UpdateStats(short player) {
		long long currentTicks = g_TimerMan.GetRealTickCount();

		if (currentTicks - m_LastStatResetTime[player] > g_TimerMan.GetTicksPerSecond() || currentTicks - m_LastStatResetTime[player] < -g_TimerMan.GetTicksPerSecond()) {
			m_LastStatResetTime[player] = currentTicks;

			m_DataUncompressedCurrent[player][STAT_SHOWN] = m_DataUncompressedCurrent[player][STAT_CURRENT];
			m_DataSentCurrent[player][STAT_SHOWN] = m_DataSentCurrent[player][STAT_CURRENT];
			m_FrameDataSentCurrent[player][STAT_SHOWN] = m_FrameDataSentCurrent[player][STAT_CURRENT];
			m_PostEffectDataSentCurrent[player][STAT_SHOWN] = m_PostEffectDataSentCurrent[player][STAT_CURRENT];
			m_SoundDataSentCurrent[player][STAT_SHOWN] = m_SoundDataSentCurrent[player][STAT_CURRENT];
			m_TerrainDataSentCurrent[player][STAT_SHOWN] = m_TerrainDataSentCurrent[player][STAT_CURRENT];
			m_OtherDataSentCurrent[player][STAT_SHOWN] = m_OtherDataSentCurrent[player][STAT_CURRENT];

			m_DataUncompressedCurrent[player][STAT_CURRENT] = 0;
			m_DataSentCurrent[player][STAT_CURRENT] = 0;
			m_FrameDataSentCurrent[player][STAT_CURRENT] = 0;
			m_PostEffectDataSentCurrent[player][STAT_CURRENT] = 0;
			m_SoundDataSentCurrent[player][STAT_CURRENT] = 0;
			m_TerrainDataSentCurrent[player][STAT_CURRENT] = 0;
			m_OtherDataSentCurrent[player][STAT_CURRENT] = 0;
		}

		if (m_PingTimer[player].IsPastRealMS(500)) {
			m_Ping[player] = m_Server->GetLastPing(m_ClientConnections[player].ClientId);
			m_PingTimer[player].Reset();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::DrawStatisticsData() {
		int midX = g_FrameMan.GetResX() / 2;

		BITMAP *bmp = g_FrameMan.GetBackBuffer8();
		AllegroBitmap guiBMP(bmp);
		clear_to_color(bmp, g_BlackColor);

		// Print server GUID to connect via NAT
		std::string guid = "Server GUID: ";
		guid += GetServerGUID().ToString();
		g_FrameMan.GetLargeFont()->DrawAligned(&guiBMP, midX, 5, guid, GUIFont::Centre);

		char buf[256];

		if (m_NatServerConnected) {
			std::snprintf(buf, sizeof(buf), "NAT SERVICE CONNECTED\nName: %s  Pass: %s", g_SettingsMan.GetNATServerName().c_str(), g_SettingsMan.GetNATServerPassword().c_str());
			g_FrameMan.GetLargeFont()->DrawAligned(&guiBMP, midX, 20, buf, GUIFont::Centre);
		} else {
			g_FrameMan.GetLargeFont()->DrawAligned(&guiBMP, midX, 20, "NOT CONNECTED TO NAT SERVICE", GUIFont::Centre);
		}

		if (g_ActivityMan.IsInActivity()) {
			const GameActivity *gameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetActivity());
			if (gameActivity) {
				std::snprintf(buf, sizeof(buf), "Activity: %s   Players: %d", gameActivity->GetPresetName().c_str(), gameActivity->GetPlayerCount());
				g_FrameMan.GetLargeFont()->DrawAligned(&guiBMP, midX, 50, buf, GUIFont::Centre);
			}
		} else {
			g_FrameMan.GetLargeFont()->DrawAligned(&guiBMP, midX, 50, "NOT IN GAME ACTIVITY", GUIFont::Centre);
		}

		m_FramesSent[c_MaxClients] = 0;
		m_FramesSkipped[c_MaxClients] = 0;

		m_DataUncompressedCurrent[c_MaxClients][STAT_SHOWN] = 0;
		m_DataSentCurrent[c_MaxClients][STAT_SHOWN] = 0;
		m_FrameDataSentCurrent[c_MaxClients][STAT_SHOWN] = 0;
		m_TerrainDataSentCurrent[c_MaxClients][STAT_SHOWN] = 0;
		m_OtherDataSentCurrent[c_MaxClients][STAT_SHOWN] = 0;

		m_FrameDataSentTotal[c_MaxClients] = 0;
		m_TerrainDataSentTotal[c_MaxClients] = 0;
		m_OtherDataSentTotal[c_MaxClients] = 0;

		m_DataUncompressedTotal[c_MaxClients] = 0;
		m_DataSentTotal[c_MaxClients] = 0;

		m_SendBufferBytes[c_MaxClients] = 0;
		m_SendBufferMessages[c_MaxClients] = 0;

		m_FullBlocks[c_MaxClients] = 0;
		m_EmptyBlocks[c_MaxClients] = 0;


		for (short i = 0; i < MAX_STAT_RECORDS; i++) {
			// Update sum
			if (i < c_MaxClients) {
				m_FramesSent[c_MaxClients] += m_FramesSent[i];
				m_FramesSkipped[c_MaxClients] += m_FramesSkipped[i];

				m_DataUncompressedCurrent[c_MaxClients][STAT_SHOWN] += m_DataUncompressedCurrent[i][STAT_SHOWN];
				m_DataSentCurrent[c_MaxClients][STAT_SHOWN] += m_DataSentCurrent[i][STAT_SHOWN];
				m_FrameDataSentCurrent[c_MaxClients][STAT_SHOWN] += m_FrameDataSentCurrent[i][STAT_SHOWN];
				m_TerrainDataSentCurrent[c_MaxClients][STAT_SHOWN] += m_TerrainDataSentCurrent[i][STAT_SHOWN];
				m_OtherDataSentCurrent[c_MaxClients][STAT_SHOWN] += m_OtherDataSentCurrent[i][STAT_SHOWN];

				m_FrameDataSentTotal[c_MaxClients] += m_FrameDataSentTotal[i];
				m_TerrainDataSentTotal[c_MaxClients] += m_TerrainDataSentTotal[i];
				m_OtherDataSentTotal[c_MaxClients] += m_OtherDataSentTotal[i];

				m_DataUncompressedTotal[c_MaxClients] += m_DataUncompressedTotal[i];
				m_DataSentTotal[c_MaxClients] += m_DataSentTotal[i];

				m_SendBufferBytes[c_MaxClients] += m_SendBufferBytes[i];
				m_SendBufferMessages[c_MaxClients] += m_SendBufferMessages[i];

				m_FullBlocks[c_MaxClients] += m_FullBlocks[i];
				m_EmptyBlocks[c_MaxClients] += m_EmptyBlocks[i];
			}

			// Update compression ratio
			double compressionRatio = (m_DataUncompressedTotal[i] > 0) ? static_cast<double>(m_DataSentTotal[i]) / static_cast<double>(m_DataUncompressedTotal[i]) : 0;
			double emptyRatio = (m_EmptyBlocks[i] > 0) ? static_cast<double>(m_FullBlocks[i]) / static_cast<double>(m_EmptyBlocks[i]) : 0;

			int fps = 0;
			if (m_MsecPerFrame[i] > 0) { fps = 1000 / m_MsecPerFrame[i]; }
			std::string playerName = IsPlayerConnected(i) ? GetPlayerName(i) : "- NO PLAYER -";

			// Jesus christ
			std::snprintf(buf, sizeof(buf),
					  "%s\nPing %u\nCmp Mbit: %.1f\nUnc Mbit: %.1f\nR: %.2f\nFrame Kbit: %lu\nGlow Kbit: %lu\nSound Kbit: %lu\nScene Kbit: %lu\nFrames sent: %uK\nFrame skipped: %uK\nBlocks full: %uK\nBlocks empty: %uK\nBlk Ratio: %.2f\nFPS: %d\nSend Ms %d\nTotal Data %lu MB",
					  (i == c_MaxClients) ? "- TOTALS - " : playerName.c_str(),
					  (i < c_MaxClients) ? m_Ping[i] : 0,
					  static_cast<double>(m_DataSentCurrent[i][STAT_SHOWN]) / 125000,
					  static_cast<double>(m_DataUncompressedCurrent[i][STAT_SHOWN]) / 125000,
					  compressionRatio,
					  m_FrameDataSentCurrent[i][STAT_SHOWN] / 125,
					  m_PostEffectDataSentCurrent[i][STAT_SHOWN] / 125,
					  m_SoundDataSentCurrent[i][STAT_SHOWN] / 125,
					  m_TerrainDataSentCurrent[i][STAT_SHOWN] / 125,
					  m_FramesSent[i] / 1000,
					  m_FramesSkipped[i] / 1000,
					  m_FullBlocks[i] / 1000,
					  m_EmptyBlocks[i] / 1000,
					  emptyRatio,
					  (i < c_MaxClients) ? fps : 0,
					  (i < c_MaxClients) ? m_MsecPerSendCall[i] : 0,
					  m_DataSentTotal[i] / (1024 * 1024)
			);

			g_FrameMan.GetLargeFont()->DrawAligned(&guiBMP, 10 + i * g_FrameMan.GetResX() / 5, 75, buf, GUIFont::Left);

			if (i < c_MaxClients) {
				int lines = 2;
				std::snprintf(buf, sizeof(buf), "Thread: %d\nBuffer: %d / %d", m_ThreadExitReason[i], m_SendBufferMessages[i], m_SendBufferBytes[i] / 1024);
				g_FrameMan.GetLargeFont()->DrawAligned(&guiBMP, 10 + i * g_FrameMan.GetResX() / 5, g_FrameMan.GetResY() - lines * 15, buf, GUIFont::Left);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	RakNet::SystemAddress NetworkServer::ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, unsigned short port) {
		if (rakPeer->Connect(address, port, 0, 0) != RakNet::CONNECTION_ATTEMPT_STARTED) {
			return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		}

		RakNet::Packet *packet;
		while (true) {
			for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive()) {
				if (packet->data[0] == ID_CONNECTION_REQUEST_ACCEPTED) {
					return packet->systemAddress;
				} else if (packet->data[0] == ID_NO_FREE_INCOMING_CONNECTIONS) {
					return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
				} else {
					return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
				}
				RakSleep(100);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::Update(bool processInput) {
		HandleNetworkPackets();

		if (processInput) {
			for (short player = 0; player < c_MaxClients; player++) {
				if (!m_InputMessages[player].empty()) {
					MsgInput msg = m_InputMessages[player].front();
					m_InputMessages[player].pop();
					ProcessInputMsg(player, msg);
				}
			}

			// Process reset votes
			// Only reset gameplay activities, and not server lobby
			if (g_ActivityMan.IsInActivity() && g_ActivityMan.GetActivity()->GetPresetName() != "Multiplayer Lobby") {

				int votesNeeded = 0;
				int endActivityVotes = 0;
				int restartVotes = 0;

				for (short player = 0; player < c_MaxClients; player++) {
					if (IsPlayerConnected(player)) {
						votesNeeded++;
						if (m_EndActivityVotes[player]) { endActivityVotes++; }
						if (m_RestartActivityVotes[player]) { restartVotes++; }
					}
				}

				std::string displayMsg = "";

				if (endActivityVotes > 0) {
					displayMsg = "Voting to end activity: " + std::to_string(endActivityVotes) + " of " + std::to_string(votesNeeded);
				}
				if (restartVotes > 0) {
					if (!displayMsg.empty()) { displayMsg += "\n"; }
					displayMsg += "Voting to restart activity: " + std::to_string(restartVotes) + " of " + std::to_string(votesNeeded);
				}

				if (votesNeeded > 0 && (endActivityVotes > 0 || restartVotes > 0)) {
					for (short i = 0; i < c_MaxClients; i++) {
						g_FrameMan.SetScreenText(displayMsg, g_ActivityMan.GetActivity()->ScreenOfPlayer(i), 0, 500);
						g_ActivityMan.GetActivity()->ResetMessageTimer(i);
					}

					// establish timer so restarts can only occur once per 5 seconds
					long long currentTicks = g_TimerMan.GetRealTickCount();
					int minRestartInterval = 5;

					if (endActivityVotes >= votesNeeded) {
						g_ActivityMan.EndActivity();
						g_ActivityMan.SetRestartActivity();
						g_ActivityMan.SetInActivity(false);
					} else if (restartVotes >= votesNeeded && ((currentTicks - m_LatestRestartTime > (g_TimerMan.GetTicksPerSecond() * minRestartInterval) || m_LatestRestartTime == 0))) {
						m_LatestRestartTime = currentTicks;
						g_ActivityMan.RestartActivity();
					}

					for (short player = 0; player < c_MaxClients; player++) {
						m_EndActivityVotes[player] = false;
						m_RestartActivityVotes[player] = false;
					}
				}
			}
		}

		DrawStatisticsData();

		// Clear sound events for unconnected players because AudioMan does not know about their state and stores broadcast sounds to their event lists
		for (short player = 0; player < c_MaxClients; player++) {
			if (!IsPlayerConnected(player)) {
				g_AudioMan.ClearSoundEvents(player);
				g_AudioMan.ClearMusicEvents(player);
			}
		}

		if (m_SleepWhenIdle && m_LastPackedReceived.IsPastRealMS(10000)) {
			short playersConnected = 0;
			for (short player = 0; player < c_MaxClients; player++)
				if (IsPlayerConnected(player)) {
					playersConnected++;
					break;
				}

			if (playersConnected == 0) { RakSleep(250); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkServer::HandleNetworkPackets() {
		RakNet::Packet *packet;

		for (packet = m_Server->Receive(); packet; m_Server->DeallocatePacket(packet), packet = m_Server->Receive()) {
			m_LastPackedReceived.Reset();

			// We got a packet, get the identifier with our handy function
			unsigned char packetIdentifier = GetPacketIdentifier(packet);
			std::string msg;
			RakNet::SystemAddress clientID;

			// Check if this is a network message packet
			switch (packetIdentifier) {
				case ID_DISCONNECTION_NOTIFICATION:
					// Connection lost normally
					msg = "ID_DISCONNECTION_NOTIFICATION from ";
					msg += packet->systemAddress.ToString(true);
					g_ConsoleMan.PrintString(msg);
					ReceiveDisconnection(packet);
					break;
				case ID_NEW_INCOMING_CONNECTION:
					ReceiveNewIncomingConnection(packet);
					break;
				case ID_INCOMPATIBLE_PROTOCOL_VERSION:
					g_ConsoleMan.PrintString("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
					break;
				case ID_CONNECTED_PING:
				case ID_UNCONNECTED_PING:
					g_ConsoleMan.PrintString(packet->systemAddress.ToString(true));
					break;
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally terminated
				case ID_CONNECTION_LOST:
				// Manual disconnection
				case ID_CLT_DISCONNECT:
					ReceiveDisconnection(packet);
					break;
				case ID_CLT_SCENE_SETUP_ACCEPTED:
					ReceiveSceneSetupDataAccepted(packet);
					break;
				case ID_CLT_REGISTER:
					ReceiveRegisterMsg(packet);
					break;
				case ID_CLT_INPUT:
					ReceiveInputMsg(packet);
					break;
				case ID_CLT_SCENE_ACCEPTED:
					ReceiveSceneAcceptedMsg(packet);
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
					break;
				case ID_NAT_SERVER_REGISTER_ACCEPTED:
					m_NatServerConnected = true;
					break;
				case ID_NAT_TARGET_NOT_CONNECTED:
					g_ConsoleMan.PrintString("Failed: ID_NAT_TARGET_NOT_CONNECTED");
					break;
				case ID_NAT_TARGET_UNRESPONSIVE:
					g_ConsoleMan.PrintString("Failed: ID_NAT_TARGET_UNRESPONSIVE");
					break;
				case ID_NAT_CONNECTION_TO_TARGET_LOST:
					g_ConsoleMan.PrintString("Failed: ID_NAT_CONNECTION_TO_TARGET_LOST");
					break;
				case ID_NAT_PUNCHTHROUGH_FAILED:
					g_ConsoleMan.PrintString("Failed: ID_NAT_PUNCHTHROUGH_FAILED");
					break;
				case ID_NAT_PUNCHTHROUGH_SUCCEEDED:
					g_ConsoleMan.PrintString("SERVER: ID_NAT_PUNCHTHROUGH_SUCCEEDED");
					break;
				default:
					break;
			}
		}
	}
}
