//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// File:            NetworkServer.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the NetworkServer class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "GUI/GUIInput.h"

#include "NetworkClient.h"
#include "NetworkServer.h"

#include "RakNetStatistics.h"
#include "RakSleep.h"
#include "GetTime.h"
#include "BitStream.h"

#include "LZ4/lz4.h"
#include "LZ4/lz4hc.h"

#include "Scene.h"
#include "SLTerrain.h"
#include "TimerMan.h"
#include "AudioMan.h"
#include "GameActivity.h"

#include "GUI/GUI.h"
#include "AllegroBitmap.h"


extern bool g_ResetActivity;
extern bool g_InActivity;

namespace RTE
{
	const std::string NetworkServer::m_ClassName = "NetworkServer";

	void BackgroundSendThreadFunction(NetworkServer * ns, int player)
	{
		while (ns->IsServerModeEnabled() && ns->IsPlayerConnected(player))
		{
			if (ns->NeedToSendSceneSetupData(player) && ns->IsSceneAvailable(player))
			{
				ns->SendSceneSetupData(player);
			}
			if (ns->NeedToSendSceneData(player) && ns->IsSceneAvailable(player))
			{
				ns->ClearTerrainChangeQueue(player);
				ns->SendSceneData(player);
			}
			if (ns->SendFrameData(player))
			{
				int ret = ns->SendFrame(player);
				ns->SetMSecsToSleep(player, ret / 1000);
				if (ret > 0)
					std::this_thread::sleep_for(std::chrono::microseconds(ret));
			}
			ns->UpdateStats(player);

		}
		ns->SetThreadExitReason(player, NetworkServer::THREAD_FINISH);

//#ifdef DEBUG_BUILD
		// Not thread safe at all to do this, just for debugging purposes
		//g_ConsoleMan.PrintString("Thread exited " + player);
//#endif

	}

	void NetworkServer::LockScene(bool isLocked)
	{
		for (int i = 0; i < c_MaxClients; i++)
		{
			if (isLocked)
				m_SceneLock[i].lock();
			else
				m_SceneLock[i].unlock();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Clear
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Clears all the member variables of this NetworkServer, effectively
	//                  resetting the members of this abstraction level only.

	void NetworkServer::Clear()
	{

		for (int i = 0; i < c_MaxClients; i++)
		{
			m_pBackBuffer8[i] = 0;
			m_pBackBufferGUI8[i] = 0;

			m_LastFrameSentTime[i] = 0;
			m_LastStatResetTime[i] = 0;

			m_DelayedFrames[i] = 0;
			m_MsecPerFrame[i] = 0;
			m_MsecPerSendCall[i] = 0;

			m_pLZ4CompressionState[i] = 0;
			m_pLZ4FastCompressionState[i] = 0;

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

			m_ResetActivityVotes[i] = false;

			m_FrameNumbers[i] = 0;

			m_Ping[i] = 0;
			m_PingTimer[i].Reset();

			ClearInputMessages(i);
		}

		for (int i = 0; i < MAX_STAT_RECORDS; i++)
		{
			m_FramesSent[i] = 0;
			m_FramesSkipped[i] = 0;

			for (int j = 0; j < 2; j++)
			{
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
		m_NatServerConnected = false;
		m_LastPackedReceived.Reset();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the NetworkServer object ready for use.

	int NetworkServer::Create()
	{
		m_IsInServerMode = false;
		m_ServerPort = "";
		m_Server = RakNet::RakPeerInterface::GetInstance();

		for (int i = 0; i < c_MaxClients; i++)
		{
			m_ClientConnections[i].InternalId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
			m_ClientConnections[i].ClientId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
			m_ClientConnections[i].IsActive = false;
			m_ClientConnections[i].pSendThread = 0;

			m_pLZ4CompressionState[i] = malloc(LZ4_sizeofStateHC());
			m_pLZ4FastCompressionState[i] = malloc(LZ4_sizeofState());
		}

		m_UseHighCompression = g_SettingsMan.GetServerUseHighCompression();
		m_UseFastCompression = g_SettingsMan.GetServerUseFastCompression();
		m_HighCompressionLevel = g_SettingsMan.GetServerHighCompressionLevel();
		m_FastAccelerationFactor = g_SettingsMan.GetServerFastAccelerationFactor();
		m_UseInterlacing = g_SettingsMan.GetServerUseInterlacing();
		m_EncodingFps = g_SettingsMan.GetServerEncodingFps();

		m_TransmitAsBoxes = g_SettingsMan.GetServerTransmitAsBoxes();
		m_BoxWidth = g_SettingsMan.GetServerBoxWidth();
		m_BoxHeight = g_SettingsMan.GetServerBoxHeight();

		return 0;
	}

	void NetworkServer::UpdateStats(int player)
	{
		int64_t currentTicks = g_TimerMan.GetRealTickCount();

		if (currentTicks - m_LastStatResetTime[player] > g_TimerMan.GetTicksPerSecond() || currentTicks - m_LastStatResetTime[player] < -g_TimerMan.GetTicksPerSecond())
		{
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

		if (m_PingTimer[player].IsPastRealMS(500))
		{ 
			m_Ping[player] = m_Server->GetLastPing(m_ClientConnections[player].ClientId);
			m_PingTimer[player].Reset();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Destroy
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Destroys and resets (through Clear()) the NetworkServer object.

	void NetworkServer::Destroy()
	{
		//Send a signal that srver is going to shutdown
		m_IsInServerMode = false;
		// Wait for thread to shut down
		Sleep(250);
		m_Server->Shutdown(300);
		// We're done with the network
		RakNet::RakPeerInterface::DestroyInstance(m_Server);

		for (int i = 0; i < c_MaxClients; i++)
		{
			DestroyBackBuffer(i);

			if (m_pLZ4CompressionState[i])
				free(m_pLZ4CompressionState[i]);
			m_pLZ4CompressionState[i] = 0;

			if (m_pLZ4FastCompressionState[i])
				free(m_pLZ4FastCompressionState[i]);
			m_pLZ4FastCompressionState[i] = 0;
		}

		Clear();
	}

	void NetworkServer::SendNATServerRegistrationMsg(RakNet::SystemAddress addr)
	{
		RTE::MsgRegisterServer msg;
		msg.Id = ID_NAT_SERVER_REGISTER_SERVER;

		strncpy(msg.ServerName, g_SettingsMan.GetNATServerName().c_str(), 62);
		strncpy(msg.ServerPassword, g_SettingsMan.GetNATServerPassword().c_str(), 62);
		RakNet::RakNetGUID guid = GetServerGuid();
		strncpy(msg.ServerGuid, guid.ToString(), 62);
		
		int payloadSize = sizeof(RTE::MsgSceneSetup);

		m_Server->Send((const char *)&msg, payloadSize, IMMEDIATE_PRIORITY, RELIABLE, 0, addr, false);
	}

	void NetworkServer::Start()
	{
		RakNet::SocketDescriptor socketDescriptors[1];
		socketDescriptors[0].port = atoi(m_ServerPort.c_str());
		socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4

		bool ok = m_Server->Startup(4, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
		if (!ok)
		{
			g_ConsoleMan.PrintString("Server: Failed to start.Terminating.\n");
			exit(1);
		}
		else
		{
			m_Server->SetMaximumIncomingConnections(4);
			g_ConsoleMan.PrintString("Server: STARTED!");
		}

		if (g_SettingsMan.GetUseNATService())
		{
			g_ConsoleMan.PrintString("Server: Connecting to NAT service.\n");

			std::string serverName;
			int port;

			string::size_type portPos = string::npos;

			portPos = g_SettingsMan.GetNATServiceAddress().find(":");
			if (portPos != string::npos)
			{
				serverName = g_SettingsMan.GetNATServiceAddress().substr(0, portPos);
				std::string portStr = g_SettingsMan.GetNATServiceAddress().substr(portPos + 1, g_SettingsMan.GetNATServiceAddress().length() - 2);

				port = atoi(portStr.c_str());
				if (port == 0)
					port = 61111;
			}
			else
			{
				serverName = g_SettingsMan.GetNATServiceAddress();
				port = 61111;
			}

			m_NATServiceServerID = ConnectBlocking(m_Server, serverName.c_str(), port);

			if (m_NATServiceServerID != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				m_Server->AttachPlugin(&m_NatPunchthroughClient);
				SendNATServerRegistrationMsg(m_NATServiceServerID);
			}
		}

		m_Server->SetOccasionalPing(true);
		m_Server->SetUnreliableTimeout(50);
	}

	void NetworkServer::Update(bool processInput)
	{
		RakNet::Packet *p;

		for (p = m_Server->Receive(); p; m_Server->DeallocatePacket(p), p = m_Server->Receive())
		{
			m_LastPackedReceived.Reset();

			// We got a packet, get the identifier with our handy function
			unsigned char packetIdentifier = GetPacketIdentifier(p);
			std::string msg;
			RakNet::SystemAddress clientID;

			// Check if this is a network message packet
			switch (packetIdentifier)
			{
			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				msg = "ID_DISCONNECTION_NOTIFICATION from ";
				msg += p->systemAddress.ToString(true);
				g_ConsoleMan.PrintString(msg);
				ReceiveDisconnection(p);
				break;


			case ID_NEW_INCOMING_CONNECTION:
				ReceiveNewIncomingConnection(p);
				break;

			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				g_ConsoleMan.PrintString("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;

			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				g_ConsoleMan.PrintString(p->systemAddress.ToString(true));
				break;

			// Couldn't deliver a reliable packet - i.e. the other system was abnormally terminated
			case ID_CONNECTION_LOST:
			// Manual disconnection
			case ID_CLT_DISCONNECT:
				ReceiveDisconnection(p);
				break;

			case ID_CLT_SCENE_SETUP_ACCEPTED:
				ReceiveSceneSetupDataAccepted(p);
				break;

			case ID_CLT_REGISTER:
				ReceiveRegisterMsg(p);
				break;

			case ID_CLT_INPUT:
				ReceiveInputMsg(p);
				break;

			case ID_CLT_SCENE_ACCEPTED:
				ReceiveSceneAcceptedMsg(p);
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
				g_ConsoleMan.PrintString("Server: ID_NAT_PUNCHTHROUGH_SUCCEEDED");
				break;

			/*case ID_UNCONNECTED_PONG:
			case ID_CONNECTED_PONG:
			{
				g_ConsoleMan.PrintString("PONG");
				int player = -1;

				for (int index = 0; index < c_MaxClients; index++)
					if (m_ClientConnections[index].ClientId == p->systemAddress)
						player = index;

				if (player > -1 && player < c_MaxClients)
				{
					unsigned int dataLength;
					RakNet::TimeMS time;
					RakNet::BitStream bsIn(p->data, p->length, false);
					bsIn.IgnoreBytes(1);
					bsIn.Read(time);
					dataLength = p->length - sizeof(unsigned char) - sizeof(RakNet::TimeMS);
					m_Ping[player] = (unsigned int)(RakNet::GetTimeMS() - time);
				}
				break;
			}*/


			default:
				break;
			}
		}

		if (processInput)
		{
			// Process input messages
			for (int p = 0; p < c_MaxClients; p++)
			{
				if (!m_InputMessages[p].empty())
				{
					NetworkClient::MsgInput msg = m_InputMessages[p].front();
					m_InputMessages[p].pop();
					ProcessInputMessage(p, msg);
				}
			}

			// Process reset votes
			int votesNeeded = 0;
			int votes = 0;
			for (int p = 0; p < c_MaxClients; p++)
			{
				if (IsPlayerConnected(p))
				{
					votesNeeded++;
					if (m_ResetActivityVotes[p])
						votes++;
				}
			}
			if (votes > 0)
			{
				char buf[128];

				sprintf_s(buf, sizeof(buf), "Voting to end activity %d of %d", votes, votesNeeded);
				for (int i = 0; i < c_MaxClients; i++)
				{
					g_FrameMan.SetScreenText(buf, i, 0, -1, false);
				}

			}
			if (votes == votesNeeded && votesNeeded > 0)
			{
				for (int p = 0; p < c_MaxClients; p++)
				{
					m_ResetActivityVotes[p] = false;
				}
				if (g_InActivity)
				{
					// Only reset gameplay activities, and not server lobby
					if (g_ActivityMan.GetActivity()->GetPresetName() != "Multiplayer Lobby")
					{
						g_ActivityMan.EndActivity();
						g_ResetActivity = true;
						g_InActivity = false;
					}
				}
			}
		}

		DrawStatisticsData();

		// Clear sound events for unconnected players because AudioMan does not know about their state and stores broadcast sounds to their event lists
		{
			std::list<AudioMan::NetworkSoundData> ls;
			std::list<AudioMan::NetworkMusicData> lm;

			for (int i = 0; i < c_MaxClients; i++)
			{
				if (!IsPlayerConnected(i))
				{
					g_AudioMan.GetSoundEvents(i, ls);
					g_AudioMan.GetMusicEvents(i, lm);
				}
			}
		}


		if (g_SettingsMan.GetServerSleepWhenIdle() && m_LastPackedReceived.IsPastRealMS(10000))
		{
			int playersConnected = 0;
			for (int player = 0; player < c_MaxClients; player++)
				if (IsPlayerConnected(player))
				{
					playersConnected++;
					break;
				}

			if (playersConnected == 0)
				RakSleep(250);
		}
	}

	RakNet::RakNetGUID NetworkServer::GetServerGuid()
	{
		return m_Server->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	}

	void NetworkServer::DrawStatisticsData()
	{
		int midX = g_FrameMan.GetResX() / 2;

		BITMAP * bmp = g_FrameMan.GetBackBuffer8();
		AllegroBitmap pGUIBitmap(bmp);
		clear_to_color(bmp, g_BlackColor);

		// Print server GUID to connect via NAT
		std::string guid = "Server GUID: ";
		guid += GetServerGuid().ToString();
		g_FrameMan.GetLargeFont()->DrawAligned(&pGUIBitmap, midX, 5, guid, GUIFont::Centre);

		char buf[256];

		if (m_NatServerConnected)
		{
			sprintf_s(buf, sizeof(buf), "NAT SERVICE CONNECTED\nName: %s  Pass: %s", g_SettingsMan.GetNATServerName().c_str(), g_SettingsMan.GetNATServerPassword().c_str());
			g_FrameMan.GetLargeFont()->DrawAligned(&pGUIBitmap, midX, 20, buf, GUIFont::Centre);
		}
		else
		{
			g_FrameMan.GetLargeFont()->DrawAligned(&pGUIBitmap, midX, 20, "NOT CONNECTED TO NAT SERVICE", GUIFont::Centre);
		}

		if (g_InActivity)
		{
			GameActivity * pGameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetActivity());
			if (pGameActivity)
			{
				sprintf_s(buf, sizeof(buf), "Activity: %s   Players: %d", pGameActivity->GetPresetName().c_str(), pGameActivity->GetPlayerCount());
				g_FrameMan.GetLargeFont()->DrawAligned(&pGUIBitmap, midX, 50, buf, GUIFont::Centre);
			}
		}
		else 
		{
			g_FrameMan.GetLargeFont()->DrawAligned(&pGUIBitmap, midX, 50, "NOT IN GAME ACTIVITY", GUIFont::Centre);
		}


		m_FramesSent[STATS_SUM] = 0;
		m_FramesSkipped[STATS_SUM] = 0;

		m_DataUncompressedCurrent[STATS_SUM][STAT_SHOWN] = 0;
		m_DataSentCurrent[STATS_SUM][STAT_SHOWN] = 0;
		m_FrameDataSentCurrent[STATS_SUM][STAT_SHOWN] = 0;
		m_TerrainDataSentCurrent[STATS_SUM][STAT_SHOWN] = 0;
		m_OtherDataSentCurrent[STATS_SUM][STAT_SHOWN] = 0;

		m_FrameDataSentTotal[STATS_SUM] = 0;
		m_TerrainDataSentTotal[STATS_SUM] = 0;
		m_OtherDataSentTotal[STATS_SUM] = 0;

		m_DataUncompressedTotal[STATS_SUM] = 0;
		m_DataSentTotal[STATS_SUM] = 0;

		m_SendBufferBytes[STATS_SUM] = 0;
		m_SendBufferMessages[STATS_SUM] = 0;

		m_FullBlocks[STATS_SUM] = 0;
		m_EmptyBlocks[STATS_SUM] = 0;


		for (int i = 0; i < MAX_STAT_RECORDS; i++)
		{
			// Update sum
			if (i < STATS_SUM)
			{
				m_FramesSent[STATS_SUM] += m_FramesSent[i];
				m_FramesSkipped[STATS_SUM] += m_FramesSkipped[i];

				m_DataUncompressedCurrent[STATS_SUM][STAT_SHOWN] += m_DataUncompressedCurrent[i][STAT_SHOWN];
				m_DataSentCurrent[STATS_SUM][STAT_SHOWN] += m_DataSentCurrent[i][STAT_SHOWN];
				m_FrameDataSentCurrent[STATS_SUM][STAT_SHOWN] += m_FrameDataSentCurrent[i][STAT_SHOWN];
				m_TerrainDataSentCurrent[STATS_SUM][STAT_SHOWN] += m_TerrainDataSentCurrent[i][STAT_SHOWN];
				m_OtherDataSentCurrent[STATS_SUM][STAT_SHOWN] += m_OtherDataSentCurrent[i][STAT_SHOWN];

				m_FrameDataSentTotal[STATS_SUM] += m_FrameDataSentTotal[i];
				m_TerrainDataSentTotal[STATS_SUM] += m_TerrainDataSentTotal[i];
				m_OtherDataSentTotal[STATS_SUM] += m_OtherDataSentTotal[i];

				m_DataUncompressedTotal[STATS_SUM] += m_DataUncompressedTotal[i];
				m_DataSentTotal[STATS_SUM] += m_DataSentTotal[i];

				m_SendBufferBytes[STATS_SUM] += m_SendBufferBytes[i];
				m_SendBufferMessages[STATS_SUM] += m_SendBufferMessages[i];

				m_FullBlocks[STATS_SUM] += m_FullBlocks[i];
				m_EmptyBlocks[STATS_SUM] += m_EmptyBlocks[i];
			}

			// Update compression ratio
			double compressionRatio = m_DataUncompressedTotal[i] > 0 ? (double)m_DataSentTotal[i] / (double)m_DataUncompressedTotal[i] : 0;
			double emptyRatio = m_EmptyBlocks[i] > 0 ? (double)m_FullBlocks[i] / (double)m_EmptyBlocks[i] : 0;

			int fps = 0;
			if (m_MsecPerFrame[i] > 0)
				fps = 1000 / m_MsecPerFrame[i];

			sprintf_s(buf, sizeof(buf), "%s\nPing %u\nCmp Mbit: %.1f\nUnc Mbit: %.1f\nR: %.2f\nFrame Kbit: %lu\nGlow Kbit: %lu\nSound Kbit: %lu\nScene Kbit: %lu\nFrames sent: %uK\nFrame skipped: %uK\nBlocks full: %uK\nBlocks empty: %uK\nBlk Ratio: %.2f\nFPS: %d\nSend Ms %d\nTotal Data %lu MB",
				i == STATS_SUM ? "- TOTALS - " : IsPlayerConnected(i) ? GetPlayerName(i).c_str() : "- NO PLAYER -",
				i < c_MaxClients ? m_Ping[i] : 0,
				(double)m_DataSentCurrent[i][STAT_SHOWN] / (125000),
				(double)m_DataUncompressedCurrent[i][STAT_SHOWN] / (125000),
				compressionRatio,
				m_FrameDataSentCurrent[i][STAT_SHOWN] / (125),
				m_PostEffectDataSentCurrent[i][STAT_SHOWN] / (125),
				m_SoundDataSentCurrent[i][STAT_SHOWN] / (125),
				m_TerrainDataSentCurrent[i][STAT_SHOWN] / (125),
				m_FramesSent[i] / 1000,
				m_FramesSkipped[i] / 1000,
				m_FullBlocks[i] / 1000,
				m_EmptyBlocks[i] / 1000,
				emptyRatio,
				i < c_MaxClients ? fps : 0,
				i < c_MaxClients ? m_MsecPerSendCall[i] : 0,
				m_DataSentTotal[i] / (1024 * 1024));

				g_FrameMan.GetLargeFont()->DrawAligned(&pGUIBitmap, 10 + i * g_FrameMan.GetResX() / 5, 75, buf, GUIFont::Left);


				if (i < c_MaxClients)
				{
					int lines = 2;
					sprintf_s(buf, sizeof(buf), "Thread: %d\nBuffer: %d / %d",
						m_ThreadExitReason[i], m_SendBufferMessages[i], m_SendBufferBytes[i] / 1024);
					g_FrameMan.GetLargeFont()->DrawAligned(&pGUIBitmap, 10 + i * g_FrameMan.GetResX() / 5, g_FrameMan.GetResY() - lines * 15, buf, GUIFont::Left);
				}
		}
	}

	bool NetworkServer::ReadyForSimulation()
	{
		int playersReady = 0;
		int playersTotal = 0;

		for (int i = 0; i < c_MaxClients; i++)
		{
			if (IsPlayerConnected(i))
				playersTotal++;
			if (SendFrameData(i))
				playersReady++;
		}

		return playersReady >= playersTotal;
	}


	void NetworkServer::CreateBackBuffer(int player, int w, int h)
	{
		m_pBackBuffer8[player] = create_bitmap_ex(8, w, h);
		m_pBackBufferGUI8[player] = create_bitmap_ex(8, w, h);
	}

	void NetworkServer::DestroyBackBuffer(int player)
	{
		if (m_pBackBuffer8)
			destroy_bitmap(m_pBackBuffer8[player]);
		m_pBackBuffer8[player] = 0;

		if (m_pBackBufferGUI8)
			destroy_bitmap(m_pBackBufferGUI8[player]);
		m_pBackBufferGUI8[player] = 0;
	}

	void NetworkServer::SendSceneSetupData(int player)
	{
		RTE::MsgSceneSetup msgSceneSetup;
		msgSceneSetup.Id = ID_SRV_SCENE_SETUP;
		msgSceneSetup.SceneId = m_SceneId;
		msgSceneSetup.Width = g_SceneMan.GetSceneWidth();
		msgSceneSetup.Height = g_SceneMan.GetSceneHeight();
		msgSceneSetup.SceneWrapsX = g_SceneMan.SceneWrapsX();

		Scene * pScene = g_SceneMan.GetScene();

		std::list<SceneLayer *> layers = pScene->GetBackLayers();
		int index = 0;

		for (std::list<SceneLayer *>::iterator itr = layers.begin(); itr != layers.end(); ++itr)
		{
			// Recalculate layers internal values for this player
			(*itr)->UpdateScrollRatiosForNetworkPlayer(player);
			msgSceneSetup.BackgroundLayers[index].BitmapHash = (*itr)->GetBitmapHash();

			msgSceneSetup.BackgroundLayers[index].DrawTrans = (*itr)->m_DrawTrans;
			msgSceneSetup.BackgroundLayers[index].OffsetX = (*itr)->m_Offset.m_X;
			msgSceneSetup.BackgroundLayers[index].OffsetY = (*itr)->m_Offset.m_Y;

			msgSceneSetup.BackgroundLayers[index].ScrollInfoX = (*itr)->m_ScrollInfo.m_X;
			msgSceneSetup.BackgroundLayers[index].ScrollInfoY = (*itr)->m_ScrollInfo.m_Y;
			msgSceneSetup.BackgroundLayers[index].ScrollRatioX = (*itr)->m_ScrollRatio.m_X;
			msgSceneSetup.BackgroundLayers[index].ScrollRatioY = (*itr)->m_ScrollRatio.m_Y;
			msgSceneSetup.BackgroundLayers[index].ScaleFactorX = (*itr)->m_ScaleFactor.m_X;
			msgSceneSetup.BackgroundLayers[index].ScaleFactorY = (*itr)->m_ScaleFactor.m_Y;
			msgSceneSetup.BackgroundLayers[index].ScaleInverseX = (*itr)->m_ScaleInverse.m_X;
			msgSceneSetup.BackgroundLayers[index].ScaleInverseY = (*itr)->m_ScaleInverse.m_Y;
			msgSceneSetup.BackgroundLayers[index].ScaledDimensionsX = (*itr)->m_ScaledDimensions.m_X;
			msgSceneSetup.BackgroundLayers[index].ScaledDimensionsY = (*itr)->m_ScaledDimensions.m_Y;

			msgSceneSetup.BackgroundLayers[index].WrapX = (*itr)->m_WrapX;
			msgSceneSetup.BackgroundLayers[index].WrapY = (*itr)->m_WrapY;

			msgSceneSetup.BackgroundLayers[index].FillLeftColor = (*itr)->m_FillLeftColor;
			msgSceneSetup.BackgroundLayers[index].FillRightColor = (*itr)->m_FillRightColor;
			msgSceneSetup.BackgroundLayers[index].FillUpColor = (*itr)->m_FillUpColor;
			msgSceneSetup.BackgroundLayers[index].FillDownColor = (*itr)->m_FillDownColor;

			index++;
			// Set everything back to what it was just in case
			(*itr)->InitScrollRatios();
		}
		msgSceneSetup.BackgroundLayerCount = index;

		int payloadSize = sizeof(RTE::MsgSceneSetup);

		m_Server->Send((const char *)&msgSceneSetup, payloadSize, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, m_ClientConnections[player].ClientId, false);

		m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
		m_DataSentTotal[player] += payloadSize;

		m_OtherDataSentCurrent[player][STAT_CURRENT] += payloadSize;
		m_OtherDataSentTotal[player] += payloadSize;

		m_DataUncompressedCurrent[player][STAT_CURRENT] += payloadSize;
		m_DataUncompressedTotal[player] += payloadSize;

		m_SendSceneSetupData[player] = false;

		// While we're on the same thread with freshly connected player, send current music being played
		if (g_AudioMan.IsMusicPlaying())
		{
			std::string currentMusic = g_AudioMan.GetMusicPath();
			if (currentMusic != "")
			{
				g_AudioMan.RegisterMusicEvent(player, AudioMan::MUSIC_PLAY, currentMusic.c_str(), -1, g_AudioMan.GetMusicPosition(), 1.0);
			}
		}
	}

	void NetworkServer::ResetScene()
	{
		m_SceneId++;
		for (int i = 0; i < c_MaxClients; i++)
		{
			m_SendSceneSetupData[i] = true;
			m_SceneAvailable[i] = true;
			m_SendSceneData[i] = false;
			m_SendFrameData[i] = false;
		}
	}

	void NetworkServer::ReceiveSceneSetupDataAccepted(RakNet::Packet * p)
	{
		int player = -1;

		for (int index = 0; index < c_MaxClients; index++)
			if (m_ClientConnections[index].ClientId == p->systemAddress)
				player = index;

		if (player > -1)
		{
			m_SendSceneSetupData[player] = false;
			m_SendSceneData[player] = true;
			m_SendFrameData[player] = false;
		}
	}

	void NetworkServer::RegisterTerrainChange(SceneMan::TerrainChange tc)
	{
		if (m_IsInServerMode)
		{
			for (int p = 0; p < c_MaxClients; p++)
			{
				if (IsPlayerConnected(p))
				{
					m_Mutex[p].lock();
					m_PendingTerrainChanges[p].push(tc);
					m_Mutex[p].unlock();
				}
			}
		}
	}

	bool NetworkServer::NeedToProcessTerrainChanges(int player)
	{
		bool result;

		m_Mutex[player].lock();
		result = !m_PendingTerrainChanges[player].empty();
		m_Mutex[player].unlock();

		return result;
	}

	void NetworkServer::ProcessTerrainChanges(int player)
	{
		m_Mutex[player].lock();
		while (!m_PendingTerrainChanges[player].empty())
		{
			m_CurrentTerrainChanges[player].push(m_PendingTerrainChanges[player].front());
			m_PendingTerrainChanges[player].pop();
		}
		m_Mutex[player].unlock();

		while (!m_CurrentTerrainChanges[player].empty())
		{
			int maxSize = 1280;

			SceneMan::TerrainChange tc = m_CurrentTerrainChanges[player].front();
			m_CurrentTerrainChanges[player].pop();

			// Fragment region if it does not fit one packet
			if (tc.w * tc.h > maxSize)
			{
				int size = 0;
				int height = 1;
				int heightStart = 0;

				for (int y = 0; y < tc.h; y++)
				{
					height++;
					size += tc.w;

					// Store changed block if the size is over the MTU or if it's the last block.
					if (size + tc.w >= maxSize || y == tc.h - 1)
					{
						SceneMan::TerrainChange tcf;
						tcf.x = tc.x;
						tcf.y = tc.y + heightStart;
						tcf.w = tc.w;
						tcf.h = height;
						tcf.back = tc.back;
						tcf.color = tc.color;

						SendTerrainChangeMsg(player, tcf);

						size = 0;
						heightStart = y;
						height = 1;
					}
				}
			}
			else 
			{
				SendTerrainChangeMsg(player, tc);
			}
		}
	}

	void NetworkServer::SendTerrainChangeMsg(int player, SceneMan::TerrainChange tc)
	{
		if (tc.w == 1 && tc.h == 1)
		{
			RTE::MsgTerrainChange msg;
			msg.Id = ID_SRV_TERRAIN;
			msg.X = tc.x;
			msg.Y = tc.y;
			msg.W = tc.w;
			msg.H = tc.h;
			msg.DataSize = 0;
			msg.UncompressedSize = 0;
			msg.SceneId = m_SceneId;
			msg.Color = tc.color;
			msg.Back = tc.back;

			int payloadSize = sizeof(RTE::MsgTerrainChange);

			m_Server->Send((const char *)&msg, payloadSize, MEDIUM_PRIORITY, RELIABLE, 0, m_ClientConnections[player].ClientId, false);

			m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_DataSentTotal[player] += payloadSize;

			m_TerrainDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_TerrainDataSentTotal[player] += payloadSize;

			m_DataUncompressedCurrent[player][STAT_CURRENT] += payloadSize;
			m_DataUncompressedTotal[player] += payloadSize;
		}
		else 
		{
			RTE::MsgTerrainChange * msg = (RTE::MsgTerrainChange *)m_aPixelLineBuffer[player];
			msg->Id = ID_SRV_TERRAIN;
			msg->X = tc.x;
			msg->Y = tc.y;
			msg->W = tc.w;
			msg->H = tc.h;
			int size = msg->W * msg->H;
			msg->DataSize = size;
			msg->UncompressedSize = size;
			msg->SceneId = m_SceneId;
			msg->Color = tc.color;
			msg->Back = tc.back;

			Scene * pScene = g_SceneMan.GetScene();
			SLTerrain * pTerrain = pScene->GetTerrain();

			BITMAP * bmp = 0;
			if (msg->Back)
				bmp = pTerrain->GetBGColorBitmap();
			else 
				bmp = pTerrain->GetFGColorBitmap();

			unsigned char * pDest = (unsigned char *)(m_aTerrainChangeBuffer[player]);

			// Copy bitmap data
			for (int y = 0; y < msg->H && msg->Y + y < bmp->h; y++)
			{
				memcpy(pDest, bmp->line[msg->Y + y] + msg->X, msg->W);
				pDest += msg->W;
			}

			int result = 0;

			result = LZ4_compress_HC_extStateHC(m_pLZ4CompressionState[player], (char *)m_aTerrainChangeBuffer[player] , (char *)(m_aPixelLineBuffer[player] + sizeof(RTE::MsgTerrainChange)), size, size, LZ4HC_CLEVEL_OPT_MIN);

			// Compression failed or ineffective, send as is
			if (result == 0 || result == size)
				memcpy_s(m_aPixelLineBuffer[player] + sizeof(RTE::MsgTerrainChange), MAX_PIXEL_LINE_BUFFER_SIZE, m_aTerrainChangeBuffer[player], size);
			else
				msg->DataSize = result;

			int payloadSize = sizeof(RTE::MsgTerrainChange) + msg->DataSize;

			m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE, 0, m_ClientConnections[player].ClientId, false);

			m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_DataSentTotal[player] += payloadSize;

			m_TerrainDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_TerrainDataSentTotal[player] += payloadSize;

			m_DataUncompressedCurrent[player][STAT_CURRENT] += msg->UncompressedSize;
			m_DataUncompressedTotal[player] += msg->UncompressedSize;
		}
	}

	void NetworkServer::ClearTerrainChangeQueue(int player)
	{
		m_Mutex[player].lock();
		while (!m_PendingTerrainChanges[player].empty())
			m_PendingTerrainChanges[player].pop();
		while (!m_CurrentTerrainChanges[player].empty())
			m_CurrentTerrainChanges[player].pop();
		m_Mutex[player].unlock();
	}


	void NetworkServer::SendSceneData(int player)
	{
		// Check for congestion
		RakNet::RakNetStatistics rns;

		RTE::MsgSceneLine * sceneData = (RTE::MsgSceneLine *)m_aPixelLineBuffer[player];

		// Save msg ID
		sceneData->Id = ID_SRV_SCENE;

		int linex = 0;
		int liney = 0;
		int lineWidth = 1280;

		Scene * pScene = g_SceneMan.GetScene();
		SLTerrain * pTerrain = 0;
		if (pScene)
		{
			pTerrain = pScene->GetTerrain();
			if (!pTerrain)
				return;
		}
		else
			return;

		// Lock the scene until current bitmap is fully transfered
		m_SceneLock[player].lock();

		for (int layer = 0; layer < 2; layer++)
		{
			BITMAP * bmp = 0;
			if (layer == 0)
				bmp = pTerrain->GetBGColorBitmap();
			else if (layer == 1)
				bmp = pTerrain->GetFGColorBitmap();

			lock_bitmap(bmp);

			for (int linex = 0; ; linex += lineWidth)
			{
				int width = lineWidth;
				if (linex + width >= g_SceneMan.GetSceneWidth())
					width = g_SceneMan.GetSceneWidth() - linex;

				for (liney = 0; liney < g_SceneMan.GetSceneHeight(); liney++)
				{
					// Save scene fragment data
					sceneData->DataSize = width;
					sceneData->UncompressedSize = width;
					sceneData->Layer = layer;
					sceneData->X = linex;
					sceneData->Y = liney;
					sceneData->SceneId = m_SceneId;

					// Compression section
					int result = 0;
					bool lineIsEmpty = false;

					result = LZ4_compress_HC_extStateHC(m_pLZ4CompressionState[player], (char *)bmp->line[liney] + linex, (char *)(m_aPixelLineBuffer[player] + sizeof(RTE::MsgSceneLine)), width, width, LZ4HC_CLEVEL_MAX);

					// Compression failed or ineffective, send as is
					if (result == 0 || result == width)
						memcpy_s(m_aPixelLineBuffer[player] + sizeof(RTE::MsgSceneLine), MAX_PIXEL_LINE_BUFFER_SIZE, bmp->line[liney] + linex, width);
					else
						sceneData->DataSize = result;

					int payloadSize = sceneData->DataSize + sizeof(RTE::MsgSceneLine);

					m_Server->Send((const char *)sceneData, payloadSize, HIGH_PRIORITY, RELIABLE, 0, m_ClientConnections[player].ClientId, false);

					m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
					m_DataSentTotal[player] += payloadSize;

					m_TerrainDataSentCurrent[player][STAT_CURRENT] += payloadSize;
					m_TerrainDataSentTotal[player] += payloadSize;

					m_DataUncompressedCurrent[player][STAT_CURRENT] += sceneData->UncompressedSize;
					m_DataUncompressedTotal[player] += sceneData->UncompressedSize;

					// Wait for the  messages to leave to avoid congestion
					if (liney % 250 == 0)
					{
						do
						{
							m_Server->GetStatistics(m_ClientConnections[player].ClientId, &rns);

							m_SendBufferBytes[player] = (int)rns.bytesInSendBuffer[MEDIUM_PRIORITY] + (int)rns.bytesInSendBuffer[HIGH_PRIORITY];
							m_SendBufferMessages[player] = (int)rns.messageInSendBuffer[MEDIUM_PRIORITY] + (int)rns.messageInSendBuffer[HIGH_PRIORITY];

							Sleep(25);
						} while (rns.messageInSendBuffer[HIGH_PRIORITY] > 1000 && IsPlayerConnected(player));

						if (!IsPlayerConnected(player))
							break;
					}
				} // next liney

				if (linex + lineWidth >= g_SceneMan.GetSceneWidth())
					break;
			}// next linex

			release_bitmap(bmp);
		}// next layer

		m_SceneLock[player].unlock();

		m_SendSceneSetupData[player] = false;
		m_SendSceneData[player] = false;
		m_SendFrameData[player] = false;

		SendSceneEndMsg(player);
	}

	void NetworkServer::SendSceneEndMsg(int player)
	{
		RTE::MsgSceneEnd msg;
		msg.Id = ID_SRV_SCENE_END;

		m_Server->Send((const char *)&msg, sizeof(RTE::MsgSceneSetup), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);
	}

	void NetworkServer::ReceiveSceneAcceptedMsg(RakNet::Packet * p)
	{
		for (int player = 0; player < c_MaxClients; player++)
			if (m_ClientConnections[player].ClientId == p->systemAddress)
				m_SendFrameData[player] = true;
	}

	void NetworkServer::SendPostEffectData(int player)
	{
		std::list<PostEffect> effects;
		g_FrameMan.GetPostEffectsList(player, effects);

		if (effects.empty())
			return;

		MsgPostEffects * msg = (MsgPostEffects * )m_aPixelLineBuffer[player];
		PostEffectNetworkData * effDataPtr = (PostEffectNetworkData *)((char *)msg + sizeof(MsgPostEffects));

		msg->Id = ID_SRV_POST_EFFECTS;
		msg->FrameNumber = m_FrameNumbers[player];
		msg->PostEffectsCount = 0;

		for (std::list<PostEffect>::iterator eItr = effects.begin(); eItr != effects.end(); ++eItr)
		{
			effDataPtr->X = (*eItr).m_Pos.GetX();
			effDataPtr->Y = (*eItr).m_Pos.GetY();
			effDataPtr->BitmapHash = (*eItr).m_BitmapHash;
			effDataPtr->Strength = (*eItr).m_Strength;
			effDataPtr->Angle = (*eItr).m_Angle;

			msg->PostEffectsCount++;
			effDataPtr++;

			if (msg->PostEffectsCount >= 75)
			{
				//char buf[128];
				//sprintf_s(buf, sizeof(buf), "%d %d", msg->FrameNumber, msg->PostEffectsCount);
				//g_ConsoleMan.PrintString(buf);

				int payloadSize = sizeof(RTE::MsgPostEffects) + sizeof(PostEffectNetworkData) * msg->PostEffectsCount;
				m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);
				msg->PostEffectsCount = 0;
				effDataPtr = (PostEffectNetworkData *)((char *)msg + sizeof(MsgPostEffects));

				m_PostEffectDataSentCurrent[player][STAT_CURRENT] += payloadSize;
				m_PostEffectDataSentTotal[player] += payloadSize;

				m_DataSentTotal[player] += payloadSize;

			}
		}

		if (msg->PostEffectsCount > 0)
		{
			//char buf[128];
			//sprintf_s(buf, sizeof(buf), "%d %d", msg->FrameNumber, msg->PostEffectsCount);
			//g_ConsoleMan.PrintString(buf);

			int header = sizeof(RTE::MsgPostEffects);
			int data = sizeof(PostEffectNetworkData);
			int toal = header + data * msg->PostEffectsCount;
			int sz = sizeof(size_t);

			int payloadSize = sizeof(RTE::MsgPostEffects) + sizeof(PostEffectNetworkData) * msg->PostEffectsCount;
			m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

			m_PostEffectDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_PostEffectDataSentTotal[player] += payloadSize;

			m_DataSentTotal[player] += payloadSize;

		}
	}

	void NetworkServer::SendSoundData(int player)
	{
		std::list<AudioMan::NetworkSoundData> events;
		g_AudioMan.GetSoundEvents(player, events);

		if (events.empty())
			return;

		MsgSoundEvents * msg = (MsgSoundEvents *)m_aPixelLineBuffer[player];
		AudioMan::NetworkSoundData * sndDataPtr = (AudioMan::NetworkSoundData *)((char *)msg + sizeof(MsgSoundEvents));

		msg->Id = ID_SRV_SOUND_EVENTS;
		msg->FrameNumber = m_FrameNumbers[player];
		msg->SoundEventsCount = 0;

		for (std::list<AudioMan::NetworkSoundData>::iterator eItr = events.begin(); eItr != events.end(); ++eItr)
		{
			sndDataPtr->State = (*eItr).State;
			sndDataPtr->Channels = (*eItr).Channels;
			sndDataPtr->Distance = (*eItr).Distance;
			sndDataPtr->SoundHash = (*eItr).SoundHash;
			sndDataPtr->Loops = (*eItr).Loops;
			sndDataPtr->Pitch = (*eItr).Pitch;
			sndDataPtr->AffectedByPitch = (*eItr).AffectedByPitch;

			msg->SoundEventsCount++;
			sndDataPtr++;

			if (msg->SoundEventsCount >= 50)
			{
				//char buf[128];
				//sprintf_s(buf, sizeof(buf), "%d %d", msg->FrameNumber, msg->PostEffectsCount);
				//g_ConsoleMan.PrintString(buf);

				int payloadSize = sizeof(RTE::MsgSoundEvents) + sizeof(AudioMan::NetworkSoundData) * msg->SoundEventsCount;
				m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);
				msg->SoundEventsCount = 0;
				sndDataPtr = (AudioMan::NetworkSoundData *)((char *)msg + sizeof(MsgSoundEvents));

				m_SoundDataSentCurrent[player][STAT_CURRENT] += payloadSize;
				m_SoundDataSentTotal[player] += payloadSize;

				m_DataSentTotal[player] += payloadSize;

			}
		}

		if (msg->SoundEventsCount > 0)
		{
			//char buf[128];
			//sprintf_s(buf, sizeof(buf), "%d %d", msg->FrameNumber, msg->PostEffectsCount);
			//g_ConsoleMan.PrintString(buf);

			int header = sizeof(RTE::MsgSoundEvents);
			int data = sizeof(AudioMan::NetworkSoundData);
			int toal = header + data * msg->SoundEventsCount;
			int sz = sizeof(size_t);

			int payloadSize = sizeof(RTE::MsgSoundEvents) + sizeof(AudioMan::NetworkSoundData) * msg->SoundEventsCount;
			m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

			m_SoundDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_SoundDataSentTotal[player] += payloadSize;

			m_DataSentTotal[player] += payloadSize;
		}
	}

	void NetworkServer::SendMusicData(int player)
	{
		std::list<AudioMan::NetworkMusicData> events;
		g_AudioMan.GetMusicEvents(player, events);

		if (events.empty())
			return;

		MsgMusicEvents * msg = (MsgMusicEvents *)m_aPixelLineBuffer[player];
		AudioMan::NetworkMusicData * sndDataPtr = (AudioMan::NetworkMusicData *)((char *)msg + sizeof(MsgMusicEvents));

		msg->Id = ID_SRV_MUSIC_EVENTS;
		msg->FrameNumber = m_FrameNumbers[player];
		msg->MusicEventsCount = 0;

		for (std::list<AudioMan::NetworkMusicData>::iterator eItr = events.begin(); eItr != events.end(); ++eItr)
		{
			sndDataPtr->State = (*eItr).State;
			sndDataPtr->Loops = (*eItr).Loops;
			sndDataPtr->Pitch = (*eItr).Pitch;
			sndDataPtr->Position = (*eItr).Position;
			strncpy(sndDataPtr->Path, (*eItr).Path, 255);

			msg->MusicEventsCount++;
			sndDataPtr++;

			if (msg->MusicEventsCount >= 4)
			{
				//char buf[128];
				//sprintf_s(buf, sizeof(buf), "%d %d", msg->FrameNumber, msg->PostEffectsCount);
				//g_ConsoleMan.PrintString(buf);

				int payloadSize = sizeof(RTE::MsgMusicEvents) + sizeof(AudioMan::NetworkMusicData) * msg->MusicEventsCount;
				m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);
				msg->MusicEventsCount = 0;
				sndDataPtr = (AudioMan::NetworkMusicData *)((char *)msg + sizeof(MsgMusicEvents));

				m_SoundDataSentCurrent[player][STAT_CURRENT] += payloadSize;
				m_SoundDataSentTotal[player] += payloadSize;

				m_DataSentTotal[player] += payloadSize;

			}
		}

		if (msg->MusicEventsCount > 0)
		{
			//char buf[128];
			//sprintf_s(buf, sizeof(buf), "%d %d", msg->FrameNumber, msg->PostEffectsCount);
			//g_ConsoleMan.PrintString(buf);

			int header = sizeof(RTE::MsgMusicEvents);
			int data = sizeof(AudioMan::NetworkMusicData);
			int toal = header + data * msg->MusicEventsCount;
			int sz = sizeof(size_t);

			int payloadSize = sizeof(RTE::MsgMusicEvents) + sizeof(AudioMan::NetworkMusicData) * msg->MusicEventsCount;
			m_Server->Send((const char *)msg, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

			m_SoundDataSentCurrent[player][STAT_CURRENT] += payloadSize;
			m_SoundDataSentTotal[player] += payloadSize;

			m_DataSentTotal[player] += payloadSize;
		}
	}


	void NetworkServer::SendFrameSetupMsg(int player)
	{
		RTE::MsgFrameSetup msgFrameSetup;
		msgFrameSetup.Id = ID_SRV_FRAME_SETUP;
		msgFrameSetup.FrameNumber = m_FrameNumbers[player];
		msgFrameSetup.TargetPosX = g_FrameMan.GetTargetPos(player).m_X;
		msgFrameSetup.TargetPosY = g_FrameMan.GetTargetPos(player).m_Y;

		for (int i = 0; i < MAX_BACKGROUND_LAYERS_TRANSMITTED; i++)
		{
			msgFrameSetup.OffsetX[i] = g_FrameMan.SLOffset[player][i].m_X;
			msgFrameSetup.OffsetY[i] = g_FrameMan.SLOffset[player][i].m_Y;
		}

		int payloadSize = sizeof(RTE::MsgSceneSetup);

		m_Server->Send((const char *)&msgFrameSetup, payloadSize, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_ClientConnections[player].ClientId, false);

		m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
		m_DataSentTotal[player] += payloadSize;

		m_FrameDataSentCurrent[player][STAT_CURRENT] += payloadSize;
		m_FrameDataSentTotal[player] += payloadSize;

		m_DataUncompressedCurrent[player][STAT_CURRENT] += payloadSize;
		m_DataUncompressedTotal[player] += payloadSize;

		m_SendSceneData[player] = false;
	}

	int NetworkServer::SendFrame(int player)
	{
		// Calc timing stuff
		int64_t currentTicks = g_TimerMan.GetRealTickCount();
		double fps = (double)m_EncodingFps;
		double secsPerFrame = 1.0 / fps;
		double secsSinceLastFrame = (double)(currentTicks - m_LastFrameSentTime[player]) / g_TimerMan.GetTicksPerSecond();

		// Fix for an overflow which may happen if server lags for a few seconds whn loading activities
		if (secsSinceLastFrame < 0)
		{
			secsSinceLastFrame = secsPerFrame;
		}

		// Is it time to send frame?
		// Return time to sleep till next frame in microsecs

		m_MSecsSinceLastUpdate[player] = secsSinceLastFrame * 1000;

		if (secsSinceLastFrame < secsPerFrame)
		{
			SetThreadExitReason(player, NetworkServer::TOO_EARLY_TO_SEND);
			return (secsPerFrame - secsSinceLastFrame) * m_MicroSecs;
		}

		// Accaumulate delayed frames counter for stats
		if (secsSinceLastFrame > secsPerFrame * 1.5f)
			m_DelayedFrames[player]++;

		m_MsecPerFrame[player] = secsSinceLastFrame * 1000;

		m_LastFrameSentTime[player] = g_TimerMan.GetRealTickCount();

		// Check for congestion
		RakNet::RakNetStatistics rns;

		m_Server->GetStatistics(m_ClientConnections[player].ClientId, &rns);

		m_SendBufferBytes[player] = (int)rns.bytesInSendBuffer[MEDIUM_PRIORITY] + (int)rns.bytesInSendBuffer[HIGH_PRIORITY];
		m_SendBufferMessages[player] = (int)rns.messageInSendBuffer[MEDIUM_PRIORITY] + (int)rns.messageInSendBuffer[HIGH_PRIORITY];

		if (rns.isLimitedByCongestionControl)
		{
			SetThreadExitReason(player, NetworkServer::SEND_BUFFER_IS_LIMITED_BY_CONGESTION);
			m_FramesSkipped[player]++;
			return (1 / fps) * m_MicroSecs;
		}

		if (rns.messageInSendBuffer[MEDIUM_PRIORITY] > 1000)
		{
			SetThreadExitReason(player, NetworkServer::SEND_BUFFER_IS_FULL);
			m_FramesSkipped[player]++;
			return 0;
		}

		// Wait till frameman releases bitmap
		SetThreadExitReason(player, NetworkServer::LOCKED);
		//while (g_FrameMan.IsNetworkBitmapLocked(player));
		SetThreadExitReason(player, NetworkServer::NORMAL);

		// Get backbuffer bitmap for this player
		BITMAP * frameManBmp = g_FrameMan.GetNetworkBackBuffer8Ready(player);
		BITMAP * frameManGUIBmp = g_FrameMan.GetNetworkBackBufferGUI8Ready(player);

		if (!m_pBackBuffer8[player])
			CreateBackBuffer(player, frameManBmp->w, frameManBmp->h);
		else
		{
			// If for whatever reasons frameMans back buffer changed dimensions, recreate our internal backbuffer
			if (m_pBackBuffer8[player]->w != frameManBmp->w || m_pBackBuffer8[player]->h != frameManBmp->h)
			{
				DestroyBackBuffer(player);
				CreateBackBuffer(player, frameManBmp->w, frameManBmp->h);
				//g_ConsoleMan.PrintString("Server: Backbuffer recreated");
			}
		}

		m_FrameNumbers[player]++;
		if (m_FrameNumbers[player] >= FRAMES_TO_REMEMBER)
			m_FrameNumbers[player] = 0;

		// Save a copy of buffer to avoid tearing when the original is updated by frame man
		blit(frameManBmp, m_pBackBuffer8[player], 0, 0, 0, 0, frameManBmp->w, frameManBmp->h);
		blit(frameManGUIBmp, m_pBackBufferGUI8[player], 0, 0, 0, 0, frameManGUIBmp->w, frameManGUIBmp->h);

		SendFrameSetupMsg(player);
		SendPostEffectData(player);
		SendSoundData(player);
		SendMusicData(player);

		m_FramesSent[player]++;

		// Compression section
		int compressionMethod = m_HighCompressionLevel;
		int accelerationFactor = m_FastAccelerationFactor;

		m_SendEven[player] = !m_SendEven[player];

		if (m_TransmitAsBoxes)
		{
			RTE::MsgFrameBox * frameData = (RTE::MsgFrameBox *)m_aPixelLineBuffer[player];
			frameData->FrameNumber = m_FrameNumbers[player];

			// Save msg ID
			frameData->Id = ID_SRV_FRAME_BOX;
			frameData->BoxWidth = m_BoxWidth;
			frameData->BoxHeight = m_BoxHeight;

			int bw = m_pBackBuffer8[player]->w / m_BoxWidth;
			int bh = m_pBackBuffer8[player]->h / m_BoxHeight;

			for (int by = 0; by <= bh; by++)
			{
				int step = 1;
				int startLine = 0;

				if (m_UseInterlacing)
				{
					step = 2;
					if (m_SendEven[player])
						startLine = by % 2 == 0 ? 1 : 0;
					else
						startLine = by % 2 == 0 ? 0 : 1;
				}

				for (int bx = startLine; bx <= bw; bx += step)
				{
					int bpx = bx * m_BoxWidth;
					int bpy = by * m_BoxHeight;

					if (bpx >= m_pBackBuffer8[player]->w || bpy >= m_pBackBuffer8[player]->h)
						break;

					frameData->BoxX = bpx;
					frameData->BoxY = bpy;

					int maxWidth = m_BoxWidth;
					if (bpx + m_BoxWidth >= m_pBackBuffer8[player]->w)
					{
						maxWidth = m_pBackBuffer8[player]->w - bpx;
						frameData->BoxWidth = maxWidth;
					}

					int maxHeight = m_BoxHeight;
					if (bpy + m_BoxHeight >= m_pBackBuffer8[player]->h)
					{
						maxHeight = m_pBackBuffer8[player]->h - bpy;
						frameData->BoxHeight = maxHeight;
					}

					int size = maxWidth * maxHeight;
					frameData->UncompressedSize = size;
					frameData->DataSize = size;

					for (int layer = 0; layer < 2; layer++)
					{
						bool boxIsEmpty = true;
						int line = 0;

						BITMAP * backBuffer = 0;
						if (layer == 0)
							backBuffer = m_pBackBuffer8[player];
						if (layer == 1)
							backBuffer = m_pBackBufferGUI8[player];

						frameData->Layer = layer;

						unsigned char * pDest = (unsigned char *)(m_aTerrainChangeBuffer[player]);

						// Copy block to line buffer and aso check if block is empty
						for (line = 0; line < maxHeight; line++)
						{
							// Copy bitmap data
							memcpy(pDest, backBuffer->line[bpy + line] + bpx, maxWidth);
							pDest += maxWidth;
						}
						// Check if block is empty
						unsigned long int * pixelInt = (unsigned long int *)m_aTerrainChangeBuffer[player];
						int counter = 0;
						for (counter = 0; counter < size; counter += sizeof(unsigned long int))
						{
							if (*pixelInt > 0)
							{
								boxIsEmpty = false;
								break;
							}
							pixelInt++;
						}
						if (boxIsEmpty && counter > size)
						{
							pixelInt--;
							counter -= sizeof(unsigned long int);

							unsigned char * pixelChr = (unsigned char *)pixelInt;
							for (; counter < size; counter++)
							{
								if (*pixelChr > 0)
								{
									boxIsEmpty = false;
									break;
								}
								pixelChr++;
							}
						}

						if (!boxIsEmpty)
						{
							int result = 0;

							if (m_UseHighCompression)
								result = LZ4_compress_HC_extStateHC(m_pLZ4CompressionState[player], (char *)m_aTerrainChangeBuffer[player], (char *)(m_aPixelLineBuffer[player] + sizeof(RTE::MsgFrameBox)), size, size, compressionMethod);
							else if (m_UseFastCompression)
								result = LZ4_compress_fast_extState(m_pLZ4FastCompressionState[player], (char *)m_aTerrainChangeBuffer[player], (char *)(m_aPixelLineBuffer[player] + sizeof(RTE::MsgFrameBox)), size, size, accelerationFactor);

							// Compression failed or ineffective, send as is
							if (result == 0 || result == backBuffer->w)
							{
								memcpy_s(m_aPixelLineBuffer[player] + sizeof(RTE::MsgFrameBox), MAX_PIXEL_LINE_BUFFER_SIZE, m_aTerrainChangeBuffer[player], size);
							}
							else
							{
								frameData->DataSize = result;
							}

							m_FullBlocks[player]++;
						}
						else
						{
							frameData->DataSize = 0;
							m_EmptyBlocks[player]++;
						}

						int payloadSize = frameData->DataSize + sizeof(RTE::MsgFrameBox);

						//if (!boxIsEmpty)
							m_Server->Send((const char *)frameData, payloadSize, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, m_ClientConnections[player].ClientId, false);
						//else
						//	payloadSize = 0;
						
						m_DataSentCurrent[player][STAT_CURRENT] += payloadSize;
						m_DataSentTotal[player] += payloadSize;

						m_FrameDataSentCurrent[player][STAT_CURRENT] += payloadSize;
						m_FrameDataSentTotal[player] += payloadSize;

						m_DataUncompressedCurrent[player][STAT_CURRENT] += frameData->UncompressedSize;
						m_DataUncompressedTotal[player] += frameData->UncompressedSize;
					}
				}
			}
		}
		else
		{
			RTE::MsgFrameLine * frameData = (RTE::MsgFrameLine *)m_aPixelLineBuffer[player];
			frameData->FrameNumber = m_FrameNumbers[player];

			// Save msg ID
			frameData->Id = ID_SRV_FRAME_LINE;

			int startLine = 0;
			int step = 1;

			if (m_UseInterlacing)
			{
				step = 2;
				m_SendEven[player] = !m_SendEven[player];
				if (m_SendEven[player])
					startLine = 0;
				else
					startLine = 1;
			}

			for (int m_CurrentFrameLine = startLine; m_CurrentFrameLine < m_pBackBuffer8[player]->h; m_CurrentFrameLine += step)
			{
				for (int layer = 0; layer < 2; layer++)
				{
					BITMAP * backBuffer = 0;

					if (layer == 0)
						backBuffer = m_pBackBuffer8[player];
					if (layer == 1)
						backBuffer = m_pBackBufferGUI8[player];

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
						unsigned long int * pixelInt = (unsigned long int *)backBuffer->line[m_CurrentFrameLine];
						int counter = 0;
						for (counter = 0; counter < backBuffer->w; counter += sizeof(unsigned long int))
						{
							if (*pixelInt > 0)
							{
								lineIsEmpty = false;
								break;
							}
							pixelInt++;
						}
						if (lineIsEmpty)
						{
							pixelInt--;
							counter -= sizeof(unsigned long int);

							unsigned char * pixelChr = (unsigned char *)pixelInt;
							for (; counter < backBuffer->w; counter++)
							{
								if (*pixelChr > 0)
								{
									lineIsEmpty = false;
									break;
								}
								pixelChr++;
							}
						}
					}

					if (!lineIsEmpty)
					{
						if (m_UseHighCompression)
							result = LZ4_compress_HC_extStateHC(m_pLZ4CompressionState[player], (char *)backBuffer->line[m_CurrentFrameLine], (char *)(m_aPixelLineBuffer[player] + sizeof(RTE::MsgFrameLine)), backBuffer->w, backBuffer->w, compressionMethod);
						else if (m_UseFastCompression)
							result = LZ4_compress_fast_extState(m_pLZ4FastCompressionState[player], (char *)backBuffer->line[m_CurrentFrameLine], (char *)(m_aPixelLineBuffer[player] + sizeof(RTE::MsgFrameLine)), backBuffer->w, backBuffer->w, accelerationFactor);

						// Compression failed or ineffective, send as is
						if (result == 0 || result == m_pBackBuffer8[player]->w)
						{
							memcpy_s(m_aPixelLineBuffer[player] + sizeof(RTE::MsgFrameLine), MAX_PIXEL_LINE_BUFFER_SIZE, backBuffer->line[m_CurrentFrameLine], backBuffer->w);
						}
						else
						{
							frameData->DataSize = result;
						}

						m_FullBlocks[player]++;
					}
					else
					{
						frameData->DataSize = 0;
						m_EmptyBlocks[player]++;
					}

					int payloadSize = frameData->DataSize + sizeof(RTE::MsgFrameLine);

					//if (!lineIsEmpty)
						m_Server->Send((const char *)frameData, payloadSize, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, m_ClientConnections[player].ClientId, false);
					//else
					//	payloadSize = 0;

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

		double secsSinceSendStart = (double)(g_TimerMan.GetRealTickCount() - currentTicks) / g_TimerMan.GetTicksPerSecond();
		m_MsecPerSendCall[player] = secsSinceSendStart * 1000;

		SetThreadExitReason(player, NetworkServer::NORMAL);
		return 0;
	}

	void NetworkServer::ReceiveDisconnection(RakNet::Packet * p)
	{
		std::string msg = "ID_CONNECTION_LOST from";
		msg += p->systemAddress.ToString(true);
		g_ConsoleMan.PrintString(msg);

		for (int index = 0; index < c_MaxClients; index++)
		{
			if (m_ClientConnections[index].ClientId == p->systemAddress)
			{
				m_ClientConnections[index].IsActive = false;
				m_ClientConnections[index].ClientId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
				m_ClientConnections[index].InternalId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

				//delete m_ClientConnections[index].pSendThread;
				m_ClientConnections[index].pSendThread = 0;

				m_SendSceneSetupData[index] = true;
				m_SendSceneData[index] = false;
				m_SendFrameData[index] = false;
			}
		}
	}

	void NetworkServer::ReceiveNewIncomingConnection(RakNet::Packet * p)
	{
		std::string msg;
		RakNet::SystemAddress clientID;
		char buf[256];

		// Somebody connected.  We have their IP now
		msg = "Server: ID_NEW_INCOMING_CONNECTION from ";
		msg += p->systemAddress.ToString(true);
		msg += " - ";
		msg += p->guid.ToString();
		g_ConsoleMan.PrintString(msg);
		clientID = p->systemAddress; // Record the player ID of the client

		RakNet::SystemAddress internalId = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

		//g_ConsoleMan.PrintString("Server: Remote internal IDs:\n");
		for (int index = 0; index < MAXIMUM_NUMBER_OF_INTERNAL_IDS; index++)
		{
			internalId = m_Server->GetInternalID(p->systemAddress, index);
			if (internalId != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				//g_ConsoleMan.PrintString(internalId.ToString(true));
			}
		}

		bool connected = false;

		// Store client connection data
		for (int index = 0; index < c_MaxClients; index++)
		{
			g_ConsoleMan.PrintString(m_ClientConnections[index].ClientId.ToString());

			if (m_ClientConnections[index].ClientId == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				m_ClientConnections[index].ClientId = clientID;
				m_ClientConnections[index].InternalId = internalId;

				connected = true;

				msg = "Server: Client connected as #";
				itoa(index, buf, 10);
				msg += buf;
				g_ConsoleMan.PrintString(msg);
				break;
			}
		}

		if (!connected)
			g_ConsoleMan.PrintString("Server: Could not accept connection");
	}

	void NetworkServer::ReceiveRegisterMsg(RakNet::Packet * p)
	{
		std::string msg;
		RTE::MsgRegister * m = (RTE::MsgRegister *)p->data;
		char buf[32];

		msg = "Server: CLIENT REGISTRATION: RES ";
		itoa(m->ResolutionX, buf, 10);
		msg += buf;
		msg += " x ";
		itoa(m->ResolutionY, buf, 10);
		msg += buf;
		g_ConsoleMan.PrintString(msg);

		for (int index = 0; index < c_MaxClients; index++)
		{
			if (m_ClientConnections[index].ClientId == p->systemAddress)
			{
				m_ClientConnections[index].ResX = m->ResolutionX;
				m_ClientConnections[index].ResY = m->ResolutionY;
				m_ClientConnections[index].IsActive = true;
				m_ClientConnections[index].PlayerName = m->Name;
				g_FrameMan.CreateNewPlayerBackBuffer(index, m->ResolutionX, m->ResolutionY);

				m_Server->SetTimeoutTime(5000, m_ClientConnections[index].ClientId);

				m_ClientConnections[index].pSendThread = new boost::thread(BackgroundSendThreadFunction, this, index);
				SendAcceptedMsg(index);

				m_SendSceneSetupData[index] = true;
				m_SendSceneData[index] = false;
				m_SendFrameData[index] = false;
			}
		}

	}

	void NetworkServer::SendAcceptedMsg(int player)
	{
		RTE::MsgAccepted msg;
		msg.Id = ID_SRV_ACCEPTED;

		m_Server->Send((const char *)&msg, sizeof(RTE::MsgAccepted), HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, m_ClientConnections[player].ClientId, false);
	}

	std::string & NetworkServer::GetPlayerName(int player)
	{
		return m_ClientConnections[player].PlayerName;
	}

	void NetworkServer::ClearInputMessages(int player)
	{
		if (player >= 0 && player < c_MaxClients)
		{
			while (!m_InputMessages[player].empty())
				m_InputMessages[player].pop();
		}
	}

	void NetworkServer::ProcessInputMessage(int player, NetworkClient::MsgInput msg)
	{
		if (player >= 0 && player < c_MaxClients)
		{
			Vector input;
			input.m_X = msg.MouseX;
			input.m_Y = msg.MouseY;
			g_UInputMan.SetNetworkMouseInput(player, input);

			g_UInputMan.SetNetworkMouseButtonHeldState(player, UInputMan::MOUSE_LEFT, msg.MouseButtonHeld[UInputMan::MOUSE_LEFT]);
			g_UInputMan.SetNetworkMouseButtonPressedState(player, UInputMan::MOUSE_LEFT, msg.MouseButtonPressed[UInputMan::MOUSE_LEFT]);
			g_UInputMan.SetNetworkMouseButtonReleasedState(player, UInputMan::MOUSE_LEFT, msg.MouseButtonReleased[UInputMan::MOUSE_LEFT]);

			if (msg.MouseButtonPressed[UInputMan::MOUSE_LEFT] || msg.MouseButtonHeld[UInputMan::MOUSE_LEFT])
				m_MouseState1[player] = 1;
			else
				m_MouseState1[player] = 0;

			g_UInputMan.SetNetworkMouseButtonHeldState(player, UInputMan::MOUSE_RIGHT, msg.MouseButtonHeld[UInputMan::MOUSE_RIGHT]);
			g_UInputMan.SetNetworkMouseButtonPressedState(player, UInputMan::MOUSE_RIGHT, msg.MouseButtonPressed[UInputMan::MOUSE_RIGHT]);
			g_UInputMan.SetNetworkMouseButtonReleasedState(player, UInputMan::MOUSE_RIGHT, msg.MouseButtonReleased[UInputMan::MOUSE_RIGHT]);

			g_UInputMan.SetNetworkMouseButtonHeldState(player, UInputMan::MOUSE_MIDDLE, msg.MouseButtonHeld[UInputMan::MOUSE_MIDDLE]);
			g_UInputMan.SetNetworkMouseButtonPressedState(player, UInputMan::MOUSE_MIDDLE, msg.MouseButtonPressed[UInputMan::MOUSE_MIDDLE]);
			g_UInputMan.SetNetworkMouseButtonReleasedState(player, UInputMan::MOUSE_MIDDLE, msg.MouseButtonReleased[UInputMan::MOUSE_MIDDLE]);


			GUIInput::SetNetworkMouseButton(player, m_MouseState1[player], m_MouseState2[player], m_MouseState3[player]);


			g_UInputMan.SetNetworkMouseWheelState(player, msg.MouseWheelMoved);

			unsigned int mask = 0x1;

			// Store element states as bit flags
			for (int i = 0; i < UInputMan::INPUT_COUNT; i++)
			{
				bool val = (msg.InputElementHeld & mask) > 0;

				g_UInputMan.SetNetworkInputElementHeldState(player, i, val);
				g_UInputMan.SetNetworkInputElementPressedState(player, i, (msg.InputElementPressed & mask) > 0);
				g_UInputMan.SetNetworkInputElementReleasedState(player, i, (msg.InputElementReleased & mask) > 0);

				mask <<= 1;
			}

			m_ResetActivityVotes[player] = msg.ResetActivityVote;

			// We need to replace mouse input obained from the allegro with mouse input obtained from network clients
			GUIInput::SetNetworkMouseMovement(player, msg.MouseX, msg.MouseY);
		}
		else 
		{
			//g_ConsoleMan.PrintString("Server: Input for unknown client. Ignored.");
		}
	}

	void NetworkServer::ReceiveInputMsg(RakNet::Packet * p)
	{
		NetworkClient::MsgInput * m = (NetworkClient::MsgInput *)p->data;
		//char buf[256];

		int player = -1;

		for (int index = 0; index < c_MaxClients; index++)
			if (m_ClientConnections[index].ClientId == p->systemAddress)
				player = index;

		if (player >= 0 && player < c_MaxClients)
		{
			// Copy message data
			NetworkClient::MsgInput msg;

			msg.Id = m->Id;

			msg.MouseX = m->MouseX;
			msg.MouseY = m->MouseY;
			for (int i = 0; i < UInputMan::MAX_MOUSE_BUTTONS; i++)
			{
				msg.MouseButtonPressed[i] = m->MouseButtonPressed[i];
				msg.MouseButtonReleased[i] = m->MouseButtonReleased[i];
				msg.MouseButtonHeld[i] = m->MouseButtonHeld[i];
			}
			msg.ResetActivityVote = m->ResetActivityVote;

			msg.MouseWheelMoved = m->MouseWheelMoved;

			msg.InputElementPressed = m->InputElementPressed;
			msg.InputElementReleased = m->InputElementReleased;
			msg.InputElementHeld = m->InputElementHeld;


			bool skip = true;

			if (!m_InputMessages[player].empty())
			{
				NetworkClient::MsgInput lastmsg = m_InputMessages[player].back();

				if (msg.MouseX != lastmsg.MouseX) skip = false;
				if (msg.MouseY != lastmsg.MouseY) skip = false;;
				for (int i = 0; i < UInputMan::MAX_MOUSE_BUTTONS; i++)
				{
					if (msg.MouseButtonPressed[i] != lastmsg.MouseButtonPressed[i]) skip = false;;
					if (msg.MouseButtonReleased[i] != lastmsg.MouseButtonReleased[i]) skip = false;;
					if (msg.MouseButtonHeld[i] != lastmsg.MouseButtonHeld[i]) skip = false;;
				}
				if (msg.ResetActivityVote != lastmsg.ResetActivityVote) skip = false;;

				if (msg.MouseWheelMoved != lastmsg.MouseWheelMoved) skip = false;;

				if (msg.InputElementPressed != lastmsg.InputElementPressed) skip = false;;
				if (msg.InputElementReleased != lastmsg.InputElementReleased) skip = false;;
				if (msg.InputElementHeld != lastmsg.InputElementHeld) skip = false;;
			}
			else 
			{
				skip = false;
			}

			if (!skip)
				m_InputMessages[player].push(msg);
		}
	}

	unsigned char NetworkServer::GetPacketIdentifier(RakNet::Packet *p)
	{
		if (p == 0)
			return 255;

		if ((unsigned char)p->data[0] == ID_TIMESTAMP)
		{
			RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
			return (unsigned char)p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
		}
		else
			return (unsigned char)p->data[0];
	}

	RakNet::SystemAddress NetworkServer::ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, int port)
	{
		if (rakPeer->Connect(address, port, 0, 0) != RakNet::CONNECTION_ATTEMPT_STARTED)
		{
			//printf("Failed connect call for %s.\n", hostName);
			return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		}
		//printf("Connecting...\n");
		RakNet::Packet *packet;
		while (1)
		{
			for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
			{
				if (packet->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
				{
					return packet->systemAddress;
				}
				else if (packet->data[0] == ID_NO_FREE_INCOMING_CONNECTIONS)
				{
					//printf("ID_NO_FREE_INCOMING_CONNECTIONS");
					return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
				}
				else
				{
					return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
				}
				Sleep(100);
			}
		}
	}



} // namespace RTE
