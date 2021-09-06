#include "ConsoleMan.h"
#include "FrameMan.h"
#include "PostProcessMan.h"
#include "AudioMan.h"
#include "SettingsMan.h"
#include "PerformanceMan.h"
#include "UInputMan.h"

#include "RakSleep.h"

#include "NetworkClient.h"
#include <lz4.h>

namespace RTE {

	// Data structure for constructing the draw boxes we'll need to use for drawing SceneLayers.
	struct SLDrawBox {
		int sourceX;
		int sourceY;
		int sourceW;
		int sourceH;
		int destX;
		int destY;
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::Clear() {
		m_LastInputSentTime = 0;
		m_ReceivedData = 0;
		m_CompressedData = 0;
		m_IsConnected = false;
		m_IsRegistered = false;
		m_ClientInputFps = 30;
		m_SceneBackgroundBitmap = 0;
		m_SceneForegroundBitmap = 0;
		m_CurrentSceneLayerReceived = -1;
		m_CurrentFrame = 0;
		m_UseNATPunchThroughService = false;
		m_ServerGUID = RakNet::UNASSIGNED_RAKNET_GUID;
		m_NATServiceServerID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		m_ServerID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		m_IsNATPunched = false;
		m_ActiveBackgroundLayers = 0;
		m_SceneWrapsX = false;

		for (int f = 0; f < c_FramesToRemember; f++) {
			m_TargetPos[f].Reset();
		}
		for (int i = 0; i < c_MaxLayersStoredForNetwork; i++) {
			m_BackgroundBitmaps[i] = 0;
		}
		for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
			m_MouseButtonPressedState[i] = -1;
			m_MouseButtonReleasedState[i] = -1;
		}
		// Stop all sounds received from server
		for (const std::pair<unsigned short, SoundContainer *> &soundEntry : m_ServerSounds) {
			soundEntry.second->Stop();
			delete soundEntry.second;
		}
		m_ServerSounds.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int NetworkClient::Initialize() {
		// Record the first client that connects to us so we can pass it to the ping function
		m_ClientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		m_Client = RakNet::RakPeerInterface::GetInstance();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::Connect(std::string serverName, unsigned short serverPort, std::string playerName) {
		g_ConsoleMan.PrintString("CLIENT: Connecting to " + serverName);

		RakNet::SocketDescriptor socketDescriptor;
		socketDescriptor.socketFamily = AF_INET;
		m_Client->Startup(8, &socketDescriptor, 1);
		m_Client->SetOccasionalPing(true);
		m_PlayerName = playerName;
		RakNet::ConnectionAttemptResult connectionAttempt = m_Client->Connect(serverName.c_str(), serverPort, NULL, 0);

		g_ConsoleMan.PrintString((connectionAttempt == RakNet::CONNECTION_ATTEMPT_STARTED) ? "CLIENT: Connect request sent" : "CLIENT: Unable to connect");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ConnectNAT(RakNet::SystemAddress address) {
		g_ConsoleMan.PrintString("CLIENT: Connecting to server through NAT");
		g_ConsoleMan.PrintString(address.ToString());

		m_ServerID = address;
		RakNet::ConnectionAttemptResult connectionAttempt = m_Client->Connect(address.ToString(false), address.GetPort(), NULL, 0);

		g_ConsoleMan.PrintString((connectionAttempt == RakNet::CONNECTION_ATTEMPT_STARTED) ? "CLIENT: Connect request sent" : "CLIENT: Unable to connect");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::Disconnect() {
		if (m_IsConnected || m_IsRegistered) { SendDisconnectMsg(); }

		m_IsRegistered = false;
		m_IsConnected = false;
		RakSleep(250);
		RakNet::AddressOrGUID addr = m_Client->GetSystemAddressFromIndex(0);
		m_Client->CloseConnection(addr, true);
		g_ConsoleMan.PrintString("CLIENT: Disconnect");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::PerformNATPunchThrough(std::string serviceServerName, unsigned short serviceServerPort, std::string playerName, std::string serverName, std::string serverPassword) {
		m_UseNATPunchThroughService = true;

		g_ConsoleMan.PrintString("CLIENT: Connecting to NAT service" + serverName);

		RakNet::SocketDescriptor socketDescriptor;
		socketDescriptor.socketFamily = AF_INET;
		m_Client->Startup(8, &socketDescriptor, 1);
		m_Client->SetOccasionalPing(true);
		m_PlayerName = playerName;
		g_ConsoleMan.PrintString("CLIENT: Connect request sent");
		m_NATServiceServerID = ConnectBlocking(m_Client, serviceServerName.c_str(), serviceServerPort);

		if (m_NATServiceServerID == RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
			g_ConsoleMan.PrintString("CLIENT: Unable to connect");
		} else {
			m_Client->SetTimeoutTime(5000, m_NATServiceServerID);
			m_Client->AttachPlugin(&m_NATPunchthroughClient);
			SendServerGUIDRequest(m_NATServiceServerID, serverName, serverPassword);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	RakNet::SystemAddress NetworkClient::ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, unsigned short port) {
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
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned char NetworkClient::GetPacketIdentifier(RakNet::Packet *packet) const {
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

	void NetworkClient::SendRegisterMsg() {
		MsgRegister msg;
		msg.Id = ID_CLT_REGISTER;
		msg.ResolutionX = g_FrameMan.GetResX();
		msg.ResolutionY = g_FrameMan.GetResY();
		strncpy(msg.Name, m_PlayerName.c_str(), c_PlayerNameCharLimit);
		m_Client->Send((const char *)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);
		g_ConsoleMan.PrintString("CLIENT: Registration Sent");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveAcceptedMsg() {
		g_ConsoleMan.PrintString("CLIENT: Registration accepted.");
		m_IsRegistered = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::SendDisconnectMsg() {
		MsgRegister msg;
		msg.Id = ID_CLT_DISCONNECT;
		m_Client->Send((const char *)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);
		g_ConsoleMan.PrintString("CLIENT: Disconnection Sent");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::SendServerGUIDRequest(RakNet::SystemAddress address, std::string serverName, std::string serverPassword) {
		MsgGetServerRequest msg;
		msg.Id = ID_NAT_SERVER_GET_SERVER_GUID;
		strncpy(msg.ServerName, serverName.c_str(), 62);
		strncpy(msg.ServerPassword, serverPassword.c_str(), 62);
		m_Client->Send((const char *)&msg, sizeof(RTE::MsgGetServerRequest), IMMEDIATE_PRIORITY, RELIABLE, 0, address, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveServerGUIDAnswer(RakNet::Packet *packet) {
		const MsgGetServerAnswer *msg = (MsgGetServerAnswer *)packet->data;
		m_ServerGUID.FromString(msg->ServerGuid);
		m_NATPunchthroughClient.OpenNAT(m_ServerGUID, m_NATServiceServerID);
		g_ConsoleMan.PrintString("CLIENT: Open NAT to server");
		g_ConsoleMan.PrintString(m_ServerGUID.ToString());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::SendInputMsg() {
		MsgInput msg;
		msg.Id = ID_CLT_INPUT;

		Vector mouse = g_UInputMan.GetNetworkAccumulatedRawMouseMovement(0);
		msg.MouseX = static_cast<int>(mouse.GetX());
		msg.MouseY = static_cast<int>(mouse.GetY());

		// Those are update in Update every frame to catch short events like clicks and releases
		msg.MouseButtonPressed[MOUSE_LEFT] = m_MouseButtonPressedState[MOUSE_LEFT] == 1 ? true : false;
		msg.MouseButtonPressed[MOUSE_MIDDLE] = m_MouseButtonPressedState[MOUSE_MIDDLE] == 1 ? true : false;
		msg.MouseButtonPressed[MOUSE_RIGHT] = m_MouseButtonPressedState[MOUSE_RIGHT] == 1 ? true : false;

		msg.MouseButtonReleased[MOUSE_LEFT] = m_MouseButtonReleasedState[MOUSE_LEFT] == 1 ? true : false;
		msg.MouseButtonReleased[MOUSE_MIDDLE] = m_MouseButtonReleasedState[MOUSE_MIDDLE] == 1 ? true : false;
		msg.MouseButtonReleased[MOUSE_RIGHT] = m_MouseButtonReleasedState[MOUSE_RIGHT] == 1 ? true : false;

		msg.MouseButtonHeld[MOUSE_LEFT] = g_UInputMan.MouseButtonHeld(MOUSE_LEFT, -1);
		msg.MouseButtonHeld[MOUSE_MIDDLE] = g_UInputMan.MouseButtonHeld(MOUSE_MIDDLE, -1);
		msg.MouseButtonHeld[MOUSE_RIGHT] = g_UInputMan.MouseButtonHeld(MOUSE_RIGHT, -1);

		for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
			m_MouseButtonPressedState[i] = -1;
			m_MouseButtonReleasedState[i] = -1;
		}

		msg.MouseWheelMoved = g_UInputMan.MouseWheelMoved();

		msg.InputElementHeld = 0;
		msg.InputElementPressed = 0;
		msg.InputElementReleased = 0;

		msg.ResetActivityVote = g_UInputMan.KeyHeld(KEY_BACKSPACE) ? true : false;
		msg.RestartActivityVote = g_UInputMan.KeyHeld(KEY_BACKSLASH) ? true : false;

		unsigned int bitMask = 0x1;

		// Store element states as bit flags
		for (int i = 0; i < INPUT_COUNT; i++) {
			if (g_UInputMan.ElementHeld(0, i)) { msg.InputElementHeld = msg.InputElementHeld | bitMask; }
			if (g_UInputMan.NetworkAccumulatedElementPressed(i)) { msg.InputElementPressed = msg.InputElementPressed | bitMask; }
			if (g_UInputMan.NetworkAccumulatedElementReleased(i)) { msg.InputElementReleased = msg.InputElementReleased | bitMask; }

			bitMask <<= 1;
		}

		g_UInputMan.ClearNetworkAccumulatedStates();
		m_Client->Send((const char *)&msg, sizeof(msg), IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveFrameSetupMsg(RakNet::Packet *packet) {
		const MsgFrameSetup *frameData = (MsgFrameSetup *)packet->data;
		if (frameData->FrameNumber < 0 || frameData->FrameNumber >= c_FramesToRemember) {
			return;
		}

		DrawFrame();

		m_PostEffects[m_CurrentFrame].clear();
		m_CurrentFrame = frameData->FrameNumber;

		m_TargetPos[m_CurrentFrame].m_X = frameData->TargetPosX;
		m_TargetPos[m_CurrentFrame].m_Y = frameData->TargetPosY;

		for (int i = 0; i < c_MaxLayersStoredForNetwork; i++) {
			m_BackgroundLayers[m_CurrentFrame][i].OffsetX = frameData->OffsetX[i];
			m_BackgroundLayers[m_CurrentFrame][i].OffsetY = frameData->OffsetY[i];
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveFrameLineMsg(RakNet::Packet *packet) {
		const MsgFrameLine *frameData = (MsgFrameLine *)packet->data;
		int lineNumber = frameData->LineNumber;
		m_CurrentSceneLayerReceived = -1;

		BITMAP *bmp = 0;

		if (frameData->Layer == 0) {
			bmp = g_FrameMan.GetNetworkBackBufferIntermediate8Ready(0);
		} else if (frameData->Layer == 1) {
			bmp = g_FrameMan.GetNetworkBackBufferIntermediateGUI8Ready(0);
		}

		acquire_bitmap(bmp);

		int width = frameData->DataSize;
		int pixels = std::min(bmp->w, width);

		m_ReceivedData += frameData->DataSize;
		m_CompressedData += frameData->UncompressedSize;

		if (lineNumber < bmp->h) {
			if (frameData->DataSize == 0) {
				memset(bmp->line[lineNumber], g_MaskColor, bmp->w);
			} else {
				if (frameData->DataSize == frameData->UncompressedSize) {
#ifdef _WIN32
					memcpy_s(bmp->line[lineNumber], bmp->w, packet->data + sizeof(MsgFrameLine), pixels);
#else
					//Fallback to non safe memcpy
					memcpy(bmp->line[lineNumber], packet->data + sizeof(MsgFrameLine), pixels);
#endif
				} else {
					LZ4_decompress_safe((char *)(packet->data + sizeof(MsgFrameLine)), (char *)(bmp->line[lineNumber]), frameData->DataSize, bmp->w);
				}
			}
		}
		release_bitmap(bmp);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveFrameBoxMsg(RakNet::Packet *packet) {
		const MsgFrameBox *frameData = (MsgFrameBox *)packet->data;
		int bpx = frameData->BoxX;
		int bpy = frameData->BoxY;
		m_CurrentSceneLayerReceived = -1;

		BITMAP *bmp = 0;

		if (frameData->Layer == 0) {
			bmp = g_FrameMan.GetNetworkBackBufferIntermediate8Ready(0);
		} else if (frameData->Layer == 1) {
			bmp = g_FrameMan.GetNetworkBackBufferIntermediateGUI8Ready(0);
		}

		acquire_bitmap(bmp);

		int maxWidth = frameData->BoxWidth;
		int maxHeight = frameData->BoxHeight;
		int size = frameData->DataSize;

		m_ReceivedData += frameData->DataSize;
		m_CompressedData += frameData->UncompressedSize;

		if (bpx + maxWidth - 1 < bmp->w && bpy + maxHeight - 1 < bmp->h && bpx >= 0 && bpy >= 0) {
			// Unpack box
			if (frameData->DataSize == 0) {
				//memset(bmp->line[lineNumber], g_MaskColor, bmp->w);
				rectfill(bmp, bpx, bpy, bpx + maxWidth - 1, bpy + maxHeight - 1, g_MaskColor);
			} else {
				if (frameData->DataSize == frameData->UncompressedSize) {
#ifdef _WIN32
					memcpy_s(m_PixelLineBuffer, size, packet->data + sizeof(MsgFrameBox), size);
#else
					// Fallback to unsafe memcpy
					memcpy(m_PixelLineBuffer, packet->data + sizeof(MsgFrameBox), size);
#endif

				} else {
					LZ4_decompress_safe((char *)(packet->data + sizeof(MsgFrameBox)), (char *)(m_PixelLineBuffer), size, frameData->UncompressedSize);
				}
				// Copy box to bitmap line by line
				const unsigned char *lineAddr = m_PixelLineBuffer;
				for (int y = 0; y < maxHeight; y++) {
#ifdef _WIN32
					memcpy_s(bmp->line[bpy + y] + bpx, maxWidth, lineAddr, maxWidth);
#else
					memcpy(bmp->line[bpy + y] + bpx, lineAddr, maxWidth);
#endif

					lineAddr += maxWidth;
				}

#ifndef RELEASE_BUILD
				if (g_UInputMan.KeyHeld(KEY_0)) { rect(bmp, bpx, bpy, bpx + maxWidth - 1, bpy + maxHeight - 1, g_BlackColor); }
#endif
			}
		}
		release_bitmap(bmp);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::SendSceneAcceptedMsg() {
		MsgRegister msg;
		msg.Id = ID_CLT_SCENE_ACCEPTED;
		m_Client->Send((const char *)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);
		g_ConsoleMan.PrintString("CLIENT: Scene ACK Sent");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveSceneMsg(RakNet::Packet *packet) {
		const MsgSceneLine *frameData = (MsgSceneLine *)packet->data;
		if (frameData->SceneId != m_SceneID) {
			return;
		}

		int linex = frameData->X;
		int liney = frameData->Y;

		const BITMAP *bmp = 0;

		if (frameData->Layer == 0) {
			bmp = m_SceneBackgroundBitmap;
		} else if (frameData->Layer == 1) {
			bmp = m_SceneForegroundBitmap;
		}

		int width = frameData->UncompressedSize;
		int pixels = bmp ? std::min(bmp->w, width) : 0;

		m_CurrentSceneLayerReceived = frameData->Layer;

		if (bmp && liney < bmp->h) {
			if (frameData->DataSize == 0) {
				memset(bmp->line[liney] + linex, g_MaskColor, width);
			} else {
				if (frameData->DataSize == frameData->UncompressedSize) {
#ifdef _WIN32
					memcpy_s(bmp->line[liney] + linex, width, packet->data + sizeof(MsgSceneLine), pixels);
#else
					memcpy(bmp->line[liney] + linex, packet->data + sizeof(MsgSceneLine), pixels);
#endif
				} else {
					LZ4_decompress_safe((char *)(packet->data + sizeof(MsgSceneLine)), (char *)(bmp->line[liney] + linex), frameData->DataSize, width);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveSceneEndMsg() {
		g_ConsoleMan.PrintString("CLIENT: Scene received.");
		SendSceneAcceptedMsg();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveSceneSetupMsg(RakNet::Packet *packet) {
		clear_to_color(g_FrameMan.GetNetworkBackBufferIntermediateGUI8Ready(0), g_MaskColor);
		clear_to_color(g_FrameMan.GetNetworkBackBufferGUI8Ready(0), g_MaskColor);

		const MsgSceneSetup *frameData = (MsgSceneSetup *)packet->data;

		m_SceneID = frameData->SceneId;

		if (m_SceneBackgroundBitmap) { destroy_bitmap(m_SceneBackgroundBitmap); }
		if (m_SceneForegroundBitmap) { destroy_bitmap(m_SceneForegroundBitmap); }

		m_SceneBackgroundBitmap = create_bitmap_ex(8, frameData->Width, frameData->Height);
		m_SceneForegroundBitmap = create_bitmap_ex(8, frameData->Width, frameData->Height);

		// This is purely for aesthetic reasons to draw bitmap during level loading
		clear_to_color(m_SceneForegroundBitmap, g_MaskColor);

		m_SceneWrapsX = frameData->SceneWrapsX;
		m_SceneWidth = frameData->Width;
		m_SceneHeight = frameData->Height;

		m_ActiveBackgroundLayers = frameData->BackgroundLayerCount;

		for (int i = 0; i < m_ActiveBackgroundLayers; i++) {
			size_t hash = frameData->BackgroundLayers[i].BitmapHash;
			std::string bitmapPath = ContentFile::GetPathFromHash(hash);
			if (!bitmapPath.empty()) {
				ContentFile fl(bitmapPath.c_str());
				m_BackgroundBitmaps[i] = fl.GetAsBitmap();
			} else {
				m_BackgroundBitmaps[i] = 0;
			}

			for (int f = 0; f < c_FramesToRemember; f++) {
				m_BackgroundLayers[f][i].DrawTrans = frameData->BackgroundLayers[i].DrawTrans;
				m_BackgroundLayers[f][i].OffsetX = frameData->BackgroundLayers[i].OffsetX;
				m_BackgroundLayers[f][i].OffsetY = frameData->BackgroundLayers[i].OffsetY;

				m_BackgroundLayers[f][i].ScrollInfoX = frameData->BackgroundLayers[i].ScrollInfoX;
				m_BackgroundLayers[f][i].ScrollInfoY = frameData->BackgroundLayers[i].ScrollInfoY;
				m_BackgroundLayers[f][i].ScrollRatioX = frameData->BackgroundLayers[i].ScrollRatioX;
				m_BackgroundLayers[f][i].ScrollRatioY = frameData->BackgroundLayers[i].ScrollRatioY;
				m_BackgroundLayers[f][i].ScaleFactorX = frameData->BackgroundLayers[i].ScaleFactorX;
				m_BackgroundLayers[f][i].ScaleFactorY = frameData->BackgroundLayers[i].ScaleFactorY;
				m_BackgroundLayers[f][i].ScaleInverseX = frameData->BackgroundLayers[i].ScaleInverseX;
				m_BackgroundLayers[f][i].ScaleInverseY = frameData->BackgroundLayers[i].ScaleInverseY;
				m_BackgroundLayers[f][i].ScaledDimensionsX = frameData->BackgroundLayers[i].ScaledDimensionsX;
				m_BackgroundLayers[f][i].ScaledDimensionsY = frameData->BackgroundLayers[i].ScaledDimensionsY;

				m_BackgroundLayers[f][i].WrapX = frameData->BackgroundLayers[i].WrapX;
				m_BackgroundLayers[f][i].WrapY = frameData->BackgroundLayers[i].WrapY;

				m_BackgroundLayers[f][i].FillLeftColor = frameData->BackgroundLayers[i].FillLeftColor;
				m_BackgroundLayers[f][i].FillRightColor = frameData->BackgroundLayers[i].FillRightColor;
				m_BackgroundLayers[f][i].FillUpColor = frameData->BackgroundLayers[i].FillUpColor;
				m_BackgroundLayers[f][i].FillDownColor = frameData->BackgroundLayers[i].FillDownColor;
			}
		}
		SendSceneSetupAcceptedMsg();
		g_ConsoleMan.PrintString("CLIENT: Scene setup accepted");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::SendSceneSetupAcceptedMsg() {
		MsgRegister msg;
		msg.Id = ID_CLT_SCENE_SETUP_ACCEPTED;
		m_Client->Send((const char *)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);
		g_ConsoleMan.PrintString("CLIENT: Scene setup ACK Sent");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveTerrainChangeMsg(RakNet::Packet *packet) {
		const MsgTerrainChange *frameData = (MsgTerrainChange *)packet->data;
		if (frameData->SceneId != m_SceneID) {
			return;
		}

		if (frameData->W == 1 && frameData->H == 1) {
			BITMAP *bmp = 0;
			bmp = frameData->Back ? m_SceneBackgroundBitmap : m_SceneForegroundBitmap;
			putpixel(bmp, frameData->X, frameData->Y, frameData->Color);
		} else {
			int size = frameData->UncompressedSize;

			if (frameData->DataSize == frameData->UncompressedSize) {
#ifdef _WIN32
				memcpy_s(m_PixelLineBuffer, size, packet->data + sizeof(MsgTerrainChange), size);
#else
				memcpy(m_PixelLineBuffer, packet->data + sizeof(MsgTerrainChange), size);
#endif
			} else {
				LZ4_decompress_safe((char *)(packet->data + sizeof(MsgTerrainChange)), (char *)m_PixelLineBuffer, frameData->DataSize, size);
			}

			// Copy bitmap data to scene bitmap
			const BITMAP *bmp = 0;
			bmp = (frameData->Back) ? m_SceneBackgroundBitmap : m_SceneForegroundBitmap;

			const unsigned char *src = m_PixelLineBuffer;
			for (int y = 0; y < frameData->H && frameData->Y + y < bmp->h; y++) {
				memcpy(bmp->line[frameData->Y + y] + frameData->X, src, frameData->W);
				src += frameData->W;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceivePostEffectsMsg(RakNet::Packet *packet) {
		MsgPostEffects *msg = (MsgPostEffects *)packet->data;
		const PostEffectNetworkData *effDataPtr = (PostEffectNetworkData *)((char *)msg + sizeof(MsgPostEffects));

		for (int i = 0; i < msg->PostEffectsCount; i++) {
			BITMAP *bmp = 0;
			std::string bitmapPath = ContentFile::GetPathFromHash(effDataPtr->BitmapHash);
			if (!bitmapPath.empty()) {
				ContentFile fl(bitmapPath.c_str());
				bmp = fl.GetAsBitmap();
			}
			if (bmp) { m_PostEffects[msg->FrameNumber].push_back(PostEffect(Vector(effDataPtr->X, effDataPtr->Y), bmp, 0, effDataPtr->Strength, effDataPtr->Angle)); }
			effDataPtr++;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveSoundEventsMsg(RakNet::Packet *packet) {
		MsgSoundEvents *msg = (MsgSoundEvents *)packet->data;
		const AudioMan::NetworkSoundData *soundDataPointer = (AudioMan::NetworkSoundData *)((char *)msg + sizeof(MsgSoundEvents));
		std::unordered_multimap<SoundContainer *, unsigned char> alreadyHandledSoundContainers;

		for (int msgIndex = 0; msgIndex < msg->SoundEventsCount; msgIndex++) {
			if (soundDataPointer->State == AudioMan::SOUND_SET_GLOBAL_PITCH) {
				g_AudioMan.SetGlobalPitch(soundDataPointer->Pitch);
			} else {
				int serverSoundChannelIndex = soundDataPointer->Channel;
				std::unordered_map<int, SoundContainer *>::iterator serverSoundEntryForChannel = m_ServerSounds.find(serverSoundChannelIndex);
				if (soundDataPointer->State == AudioMan::SOUND_PLAY || serverSoundEntryForChannel != m_ServerSounds.end()) {
					SoundContainer *soundContainerToHandle = (serverSoundEntryForChannel == m_ServerSounds.end()) ? nullptr : m_ServerSounds.at(serverSoundChannelIndex);

					auto alreadyHandledSoundStates = alreadyHandledSoundContainers.equal_range(soundContainerToHandle);
					bool alreadyHandled = soundDataPointer->State != AudioMan::SOUND_PLAY && std::any_of(alreadyHandledSoundStates.first, alreadyHandledSoundStates.second, [&soundDataPointer](const std::pair<SoundContainer *, unsigned char> &alreadyHandledSoundStateEntry) { return static_cast<const AudioMan::NetworkSoundState>(alreadyHandledSoundStateEntry.second) == soundDataPointer->State; });
					if (!alreadyHandled) {
						switch (soundDataPointer->State) {
							case AudioMan::SOUND_PLAY:
								if (soundContainerToHandle == nullptr) {
									soundContainerToHandle = new SoundContainer;
								} else {
									soundContainerToHandle->Stop();
									soundContainerToHandle->Reset();
								}
								soundContainerToHandle->GetTopLevelSoundSet().AddSound(ContentFile::GetPathFromHash(soundDataPointer->SoundFileHash), false);
								soundContainerToHandle->SetImmobile(soundDataPointer->Immobile);
								soundContainerToHandle->SetAttenuationStartDistance(soundDataPointer->AttenuationStartDistance);
								soundContainerToHandle->SetLoopSetting(soundDataPointer->Loops);
								soundContainerToHandle->SetPriority(soundDataPointer->Priority);
								soundContainerToHandle->SetAffectedByGlobalPitch(soundDataPointer->AffectedByGlobalPitch);
								soundContainerToHandle->SetPosition(Vector(soundDataPointer->Position[0], soundDataPointer->Position[1]));
								soundContainerToHandle->SetVolume(soundDataPointer->Volume);
								soundContainerToHandle->SetPitch(soundDataPointer->Pitch);
								soundContainerToHandle->Play();
								break;
							case AudioMan::SOUND_STOP:
								soundContainerToHandle->Stop();
								break;
							case AudioMan::SOUND_SET_POSITION:
								soundContainerToHandle->SetPosition(Vector(soundDataPointer->Position[0], soundDataPointer->Position[1]));
								break;
							case AudioMan::SOUND_SET_VOLUME:
								soundContainerToHandle->SetVolume(soundDataPointer->Volume);
								break;
							case AudioMan::SOUND_SET_PITCH:
								soundContainerToHandle->SetPitch(soundDataPointer->Pitch);
								break;
							case AudioMan::SOUND_FADE_OUT:
								soundContainerToHandle->FadeOut(soundDataPointer->FadeOutTime);
								break;
							default:
								RTEAbort("Multiplayer client tried to receive unhandled Sound Event, of state " + soundDataPointer->State);
						}
						alreadyHandledSoundContainers.insert({soundContainerToHandle, soundDataPointer->State});
					}
					if (soundDataPointer->State == AudioMan::SOUND_PLAY) { m_ServerSounds.insert({ serverSoundChannelIndex, soundContainerToHandle }); }
				}
			}
			soundDataPointer++;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::ReceiveMusicEventsMsg(RakNet::Packet *packet) {
		MsgMusicEvents *msg = (MsgMusicEvents *)packet->data;
		const AudioMan::NetworkMusicData *musicDataPointer = (AudioMan::NetworkMusicData *)((char *)msg + sizeof(MsgMusicEvents));

		for (int i = 0; i < msg->MusicEventsCount; i++) {
			switch (musicDataPointer->State) {
				case AudioMan::MUSIC_PLAY:
					char path[256];

					memset(path, 0, 256);
					strncpy(path, musicDataPointer->Path, 255);

					char buf[128];
					std::snprintf(buf, sizeof(buf), "MUSIC %s %d", path, musicDataPointer->LoopsOrSilence);

					g_AudioMan.PlayMusic(path, musicDataPointer->LoopsOrSilence);
					if (musicDataPointer->Position > 0) { g_AudioMan.SetMusicPosition(musicDataPointer->Position); }
					break;
				case AudioMan::MUSIC_STOP:
					g_AudioMan.StopMusic();
					break;
				case AudioMan::MUSIC_SET_PITCH:
					g_AudioMan.SetMusicPitch(musicDataPointer->Pitch);
					break;
				case AudioMan::MUSIC_SILENCE:
					g_AudioMan.QueueSilence(musicDataPointer->LoopsOrSilence);
					g_AudioMan.StopMusic();
					break;
				default:
					RTEAbort("Multiplayer client tried to receive unhandled Music Event, of state " + musicDataPointer->State);
			}
			musicDataPointer++;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::DrawBackgrounds(BITMAP *targetBitmap) {
		for (int i = m_ActiveBackgroundLayers - 1; i >= 0; i--) {
			if (m_BackgroundBitmaps[i] != 0) {
				//masked_blit(m_BackgroundBitmaps[i], targetBitmap, 0, 0, 0, 0, m_BackgroundBitmaps[i]->w, m_BackgroundBitmaps[i]->h);
				Vector scrollOverride(0, 0);
				bool scrollOverridden = false;

				int frame = m_CurrentFrame;

				// Set up the target box to draw to on the target bitmap, if it is larger than the scene in either dimension
				Box targetBox(Vector(0, 0), targetBitmap->w, targetBitmap->h);

				if (!m_SceneWrapsX && targetBitmap->w > m_SceneWidth) {
					targetBox.m_Corner.m_X = (targetBitmap->w - m_SceneWidth) / 2;
					targetBox.m_Width = m_SceneWidth;
				}
				if (targetBitmap->h > m_SceneHeight) {
					targetBox.m_Corner.m_Y = (targetBitmap->h - m_SceneHeight) / 2;
					targetBox.m_Height = m_SceneHeight;
				}

				int sourceX = 0;
				int sourceY = 0;
				int sourceW = 0;
				int sourceH = 0;
				int destX = 0;
				int destY = 0;
				std::list<SLDrawBox> drawList;

				int offsetX;
				int offsetY;

				if (scrollOverridden) {
					// Overridden scroll position
					offsetX = scrollOverride.GetFloorIntX();
					offsetY = scrollOverride.GetFloorIntY();
				} else {
					// Regular scroll
					offsetX = std::floor(m_BackgroundLayers[frame][i].OffsetX * m_BackgroundLayers[frame][i].ScrollRatioX);
					offsetY = std::floor(m_BackgroundLayers[frame][i].OffsetY * m_BackgroundLayers[frame][i].ScrollRatioY);
			
					// Only force bounds when doing regular scroll offset because the override is used to do terrain object application tricks and sometimes needs the offsets to be < 0
					// ForceBounds(offsetX, offsetY);
					// WrapPosition(offsetX, offsetY);
					int width = m_BackgroundBitmaps[i]->w;
					int height = m_BackgroundBitmaps[i]->h;

					if (m_BackgroundLayers[frame][i].WrapX) {
						if (offsetX < 0) {
							while (offsetX < 0) {
								offsetX += width;
							}
						} else if (offsetX >= width) {
							offsetX %= width;
						}
					}

					if (m_BackgroundLayers[frame][i].WrapY) {
						if (offsetY < 0) {
							while (offsetY < 0) {
								offsetY += height;
							}
						} else if (offsetY >= height) {
							offsetY %= height;
						}
					}
				}

				// Make target box valid size if it's empty
				if (targetBox.IsEmpty()) {
					targetBox.SetCorner(Vector(0, 0));
					targetBox.SetWidth(targetBitmap->w);
					targetBox.SetHeight(targetBitmap->h);
				}

				// Set the clipping rectangle of the target bitmap to match the specified target box
				set_clip_rect(targetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth() - 1, targetBox.GetCorner().m_Y + targetBox.GetHeight() - 1);

				// Choose the correct blitting function based on transparency setting
				void(*pfBlit)(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = m_BackgroundLayers[frame][i].DrawTrans ? &masked_blit : &blit;

				// See if this SceneLayer is wider AND higher than the target bitmap; then use simple wrapping logic - otherwise need to tile
				if (m_BackgroundBitmaps[i]->w >= targetBitmap->w && m_BackgroundBitmaps[i]->h >= targetBitmap->h) {
					sourceX = offsetX;
					sourceY = offsetY;
					sourceW = m_BackgroundBitmaps[i]->w - offsetX;
					sourceH = m_BackgroundBitmaps[i]->h - offsetY;
					destX = targetBox.GetCorner().m_X;
					destY = targetBox.GetCorner().m_Y;
					pfBlit(m_BackgroundBitmaps[i], targetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

					sourceX = 0;
					sourceY = offsetY;
					sourceW = offsetX;
					sourceH = m_BackgroundBitmaps[i]->h - offsetY;
					destX = targetBox.GetCorner().m_X + m_BackgroundBitmaps[i]->w - offsetX;
					destY = targetBox.GetCorner().m_Y;
					pfBlit(m_BackgroundBitmaps[i], targetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

					sourceX = offsetX;
					sourceY = 0;
					sourceW = m_BackgroundBitmaps[i]->w - offsetX;
					sourceH = offsetY;
					destX = targetBox.GetCorner().m_X;
					destY = targetBox.GetCorner().m_Y + m_BackgroundBitmaps[i]->h - offsetY;
					pfBlit(m_BackgroundBitmaps[i], targetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

					sourceX = 0;
					sourceY = 0;
					sourceW = offsetX;
					sourceH = offsetY;
					destX = targetBox.GetCorner().m_X + m_BackgroundBitmaps[i]->w - offsetX;
					destY = targetBox.GetCorner().m_Y + m_BackgroundBitmaps[i]->h - offsetY;
					pfBlit(m_BackgroundBitmaps[i], targetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);
				} else {
					// Target bitmap is larger in some dimension, so need to draw this tiled as many times as necessary to cover the whole target
					int tiledOffsetX = 0;
					int tiledOffsetY = 0;
					// Use the dimensions of the target box, if it has any area at all
					int targetWidth = std::min(targetBitmap->w, static_cast<int>(targetBox.GetWidth()));
					int targetHeight = std::min(targetBitmap->h, static_cast<int>(targetBox.GetHeight()));
					int toCoverX = offsetX + targetBox.GetCorner().m_X + targetWidth;
					int toCoverY = offsetY + targetBox.GetCorner().m_Y + targetHeight;

					// Check for special case adjustment when the screen is larger than the scene
					bool screenLargerThanSceneX = false;
					bool screenLargerThanSceneY = false;
					if (!scrollOverridden && m_SceneWidth > 0) {
						screenLargerThanSceneX = targetBitmap->w > m_SceneWidth;
						screenLargerThanSceneY = targetBitmap->h > m_SceneHeight;
					}

					// Y tiling
					do {
						// X tiling
						do {
							sourceX = 0;
							sourceY = 0;
							sourceW = m_BackgroundBitmaps[i]->w;
							sourceH = m_BackgroundBitmaps[i]->h;
							// If the unwrapped and untiled direction can't cover the target area, place it in the middle of the target bitmap, and leave the excess perimeter on each side untouched
							destX = (!m_BackgroundLayers[frame][i].WrapX && screenLargerThanSceneX) ? ((targetBitmap->w / 2) - (m_BackgroundBitmaps[i]->w / 2)) : (targetBox.GetCorner().m_X + tiledOffsetX - offsetX);
							destY = (!m_BackgroundLayers[frame][i].WrapY && screenLargerThanSceneY) ? ((targetBitmap->h / 2) - (m_BackgroundBitmaps[i]->h / 2)) : (targetBox.GetCorner().m_Y + tiledOffsetY - offsetY);

							pfBlit(m_BackgroundBitmaps[i], targetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

							tiledOffsetX += m_BackgroundBitmaps[i]->w;
						}
						// Only tile if we're supposed to wrap widthwise
						while (m_BackgroundLayers[frame][i].WrapX && toCoverX > tiledOffsetX);

						tiledOffsetY += m_BackgroundBitmaps[i]->h;
					}
					// Only tile if we're supposed to wrap height wise
					while (m_BackgroundLayers[frame][i].WrapY && toCoverY > tiledOffsetY);

					// TODO: Do this above instead, testing down here only
					// Detect if non-wrapping layer dimensions can't cover the whole target area with its main bitmap. If so, fill in the gap with appropriate solid color sampled from the hanging edge
					if (!m_BackgroundLayers[frame][i].WrapX && !screenLargerThanSceneX && m_BackgroundLayers[frame][i].ScrollRatioX < 0) {
						if (m_BackgroundLayers[frame][i].FillLeftColor != g_MaskColor && offsetX != 0) {
							rectfill(targetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X - offsetX, targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_BackgroundLayers[frame][i].FillLeftColor);
						}
						if (m_BackgroundLayers[frame][i].FillRightColor != g_MaskColor) {
							rectfill(targetBitmap, (targetBox.GetCorner().m_X - offsetX) + m_BackgroundBitmaps[i]->w, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_BackgroundLayers[frame][i].FillRightColor);
						}
					}
					if (!m_BackgroundLayers[frame][i].WrapY && !screenLargerThanSceneY && m_BackgroundLayers[frame][i].ScrollRatioY < 0) {
						if (m_BackgroundLayers[frame][i].FillUpColor != g_MaskColor && offsetY != 0) {
							rectfill(targetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y - offsetY, m_BackgroundLayers[frame][i].FillUpColor);
						}
						if (m_BackgroundLayers[frame][i].FillDownColor != g_MaskColor) {
							rectfill(targetBitmap, targetBox.GetCorner().m_X, (targetBox.GetCorner().m_Y - offsetY) + m_BackgroundBitmaps[i]->h, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_BackgroundLayers[frame][i].FillDownColor);
						}
					}
				}
				// Reset the clip rect back to the entire target bitmap
				set_clip_rect(targetBitmap, 0, 0, targetBitmap->w - 1, targetBitmap->h - 1);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::DrawPostEffects(int frame) { g_PostProcessMan.SetNetworkPostEffectsList(0, m_PostEffects[frame]); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::DrawFrame() {
		BITMAP *src_bmp = g_FrameMan.GetNetworkBackBufferIntermediate8Ready(0);
		BITMAP *dst_bmp = g_FrameMan.GetNetworkBackBuffer8Ready(0);

		BITMAP *src_gui_bmp = g_FrameMan.GetNetworkBackBufferIntermediateGUI8Ready(0);
		BITMAP *dst_gui_bmp = g_FrameMan.GetNetworkBackBufferGUI8Ready(0);

		// Have to clear to color to fallback if there's no skybox on client
		clear_to_color(dst_bmp, g_BlackColor);
		clear_to_color(dst_gui_bmp, g_MaskColor);

		// Draw Scene background
		int sourceX = m_TargetPos[m_CurrentFrame].m_X;
		int sourceY = m_TargetPos[m_CurrentFrame].m_Y;
		int destX = 0;
		int destY = 0;

		DrawBackgrounds(dst_bmp);
		masked_blit(m_SceneBackgroundBitmap, dst_bmp, sourceX, sourceY, destX, destY, src_bmp->w, src_bmp->h);
		
		if (sourceX < 0) {
			// Draw if the out of seam portion is to the left
			int newSourceX = m_SceneBackgroundBitmap->w + sourceX;

			masked_blit(m_SceneBackgroundBitmap, dst_bmp, newSourceX, sourceY, destX, destY, src_bmp->w, src_bmp->h);
		} else if (sourceX + g_FrameMan.GetResX() >= m_SceneBackgroundBitmap->w) {
			// Draw if the out of seam portion is to the right
			int newDestX = m_SceneBackgroundBitmap->w - sourceX;
			int width = g_FrameMan.GetResX() - newDestX;

			masked_blit(m_SceneBackgroundBitmap, dst_bmp, 0, sourceY, newDestX, destY, width, src_bmp->h);
		}

		//draw_sprite(src_bmp, dst_bmp, 0, 0);
		masked_blit(src_bmp, dst_bmp, 0, 0, 0, 0, src_bmp->w, src_bmp->h);
		masked_blit(src_gui_bmp, dst_gui_bmp, 0, 0, 0, 0, src_bmp->w, src_bmp->h);
		masked_blit(m_SceneForegroundBitmap, dst_bmp, sourceX, sourceY, destX, destY, src_bmp->w, src_bmp->h);

		if (sourceX < 0) {
			// Draw if the out of seam portion is to the left
			int newSourceX = m_SceneForegroundBitmap->w + sourceX;

			masked_blit(m_SceneForegroundBitmap, dst_bmp, newSourceX, sourceY, destX, destY, src_bmp->w, src_bmp->h);
		} else if (sourceX + g_FrameMan.GetResX() >= m_SceneForegroundBitmap->w) {
			// Draw if the out of seam portion is to the right
			int newDestX = m_SceneForegroundBitmap->w - sourceX;
			int width = g_FrameMan.GetResX() - newDestX;

			masked_blit(m_SceneForegroundBitmap, dst_bmp, 0, sourceY, newDestX, destY, width, src_bmp->h);
		}

		DrawPostEffects(m_CurrentFrame);

		g_PerformanceMan.SetCurrentPing(GetPing());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::Update() {
		HandleNetworkPackets();

		// Draw level loading animation
		if (m_CurrentSceneLayerReceived != -1) {
			BITMAP * bmp = 0;

			if (m_CurrentSceneLayerReceived == -1) {
				bmp = m_SceneBackgroundBitmap;
			} else if (m_CurrentSceneLayerReceived == 0) {
				bmp = m_SceneBackgroundBitmap;
			} else if (m_CurrentSceneLayerReceived == 1) {
				bmp = m_SceneForegroundBitmap;
			}

			BITMAP *dst_bmp = g_FrameMan.GetNetworkBackBuffer8Ready(0);

			clear_to_color(dst_bmp, g_BlackColor);

			float scale = static_cast<float>(dst_bmp->w) / static_cast<float>(bmp->w);
			int w = dst_bmp->w;
			int h = static_cast<float>(bmp->h) * scale;

			int x = 0;
			int y = g_FrameMan.GetResY() / 2 - h / 2;
			if (h >= g_FrameMan.GetResY()) { y = 0; }
				
			// Recalculate everything for tall maps
			if (static_cast<float>(bmp->h) / static_cast<float>(bmp->w) > 1) {
				scale = static_cast<float>(dst_bmp->h) / static_cast<float>(bmp->h);
				h = dst_bmp->h;
				w = static_cast<float>(bmp->w) * scale;

				x = g_FrameMan.GetResX() / 2 - w / 2;
				y = 0;
				if (w >= g_FrameMan.GetResX()) { x = 0; }					
			}

			// Draw previous layer
			if (m_CurrentSceneLayerReceived == 1) { masked_stretch_blit(m_SceneBackgroundBitmap, dst_bmp, 0, 0, bmp->w, bmp->h, x, y, w, h); }

			masked_stretch_blit(bmp, dst_bmp, 0, 0, bmp->w, bmp->h, x, y, w, h);
		}

		// Detect short mouse events like presses and releases. Holds are detected during input send
		if (m_MouseButtonPressedState[MOUSE_LEFT] < 1) { m_MouseButtonPressedState[MOUSE_LEFT] = g_UInputMan.MouseButtonPressed(MOUSE_LEFT, -1) ? 1 : 0; }
		if (m_MouseButtonPressedState[MOUSE_RIGHT] < 1) { m_MouseButtonPressedState[MOUSE_RIGHT] = g_UInputMan.MouseButtonPressed(MOUSE_RIGHT, -1) ? 1 : 0; }
		if (m_MouseButtonPressedState[MOUSE_MIDDLE] < 1) { m_MouseButtonPressedState[MOUSE_MIDDLE] = g_UInputMan.MouseButtonPressed(MOUSE_MIDDLE, -1) ? 1 : 0; }

		if (m_MouseButtonReleasedState[MOUSE_LEFT] < 1) { m_MouseButtonReleasedState[MOUSE_LEFT] = g_UInputMan.MouseButtonReleased(MOUSE_LEFT, -1) ? 1 : 0; }
		if (m_MouseButtonReleasedState[MOUSE_RIGHT] < 1) { m_MouseButtonReleasedState[MOUSE_RIGHT] = g_UInputMan.MouseButtonReleased(MOUSE_RIGHT, -1) ? 1 : 0; }
		if (m_MouseButtonReleasedState[MOUSE_MIDDLE] < 1) { m_MouseButtonReleasedState[MOUSE_MIDDLE] = g_UInputMan.MouseButtonReleased(MOUSE_MIDDLE, -1) ? 1 : 0; }

		// Input is sent at whatever settings are set in inputs per second
		float inputSend = m_ClientInputFps;

#if !defined DEBUG_RELEASE_BUILD && !defined RELEASE_BUILD
		// Reduce input rate for debugging because it may overflow the input queue
		inputSend = 10;
#endif

		long long currentTicks = g_TimerMan.GetRealTickCount();
		if (currentTicks - m_LastInputSentTime < 0) { m_LastInputSentTime = currentTicks; }

		if (static_cast<double>((currentTicks - m_LastInputSentTime)) / static_cast<double>(g_TimerMan.GetTicksPerSecond()) > 1.0 / inputSend) {
			m_LastInputSentTime = g_TimerMan.GetRealTickCount();
			if (IsConnectedAndRegistered()) { SendInputMsg(); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void NetworkClient::HandleNetworkPackets() {
		RakNet::Packet *packet;
		std::string msg;

		for (packet = m_Client->Receive(); packet; m_Client->DeallocatePacket(packet), packet = m_Client->Receive()) {
			// We got a packet, get the identifier with our handy function
			unsigned char packetIdentifier = GetPacketIdentifier(packet);

			// Check if this is a network message packet
			switch (packetIdentifier) {
				case ID_DISCONNECTION_NOTIFICATION:
					// Connection lost normally
					g_ConsoleMan.PrintString("ID_DISCONNECTION_NOTIFICATION");
					Disconnect();
					break;
				case ID_ALREADY_CONNECTED:
					// Connection lost normally
					msg = "ID_ALREADY_CONNECTED with GUID";
					msg += packet->guid.ToString();
					g_ConsoleMan.PrintString(msg);
					break;
				case ID_INCOMPATIBLE_PROTOCOL_VERSION:
					g_ConsoleMan.PrintString("ID_INCOMPATIBLE_PROTOCOL_VERSION");
					break;
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					// Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer environment if you want.
					g_ConsoleMan.PrintString("ID_REMOTE_DISCONNECTION_NOTIFICATION");
					break;
				case ID_REMOTE_CONNECTION_LOST:
					// Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer environment if you want.
					g_ConsoleMan.PrintString("ID_REMOTE_CONNECTION_LOST");
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION:
					// Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer environment if you want.
					g_ConsoleMan.PrintString("ID_REMOTE_NEW_INCOMING_CONNECTION");
					break;
				case ID_CONNECTION_BANNED:
					// Banned from this server
					g_ConsoleMan.PrintString("We are banned from this server.");
					break;
				case ID_CONNECTION_ATTEMPT_FAILED:
					g_ConsoleMan.PrintString("Connection attempt failed");
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					// Sorry, the server is full. I don't do anything here but A real app should tell the user
					g_ConsoleMan.PrintString("ID_NO_FREE_INCOMING_CONNECTIONS");
					break;
				case ID_INVALID_PASSWORD:
					g_ConsoleMan.PrintString("ID_INVALID_PASSWORD");
					break;
				case ID_CONNECTION_LOST:
					// Couldn't deliver a reliable packet - i.e. the other system was abnormally terminated
					g_ConsoleMan.PrintString("ID_CONNECTION_LOST");
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
					// This tells the client they have connected
					msg = "ID_CONNECTION_REQUEST_ACCEPTED to ";
					msg += packet->systemAddress.ToString(true);
					msg += " GUID ";
					msg += packet->guid.ToString();
					g_ConsoleMan.PrintString(msg);

					msg = "My external address is";
					msg += m_Client->GetExternalID(packet->systemAddress).ToString(true);
					g_ConsoleMan.PrintString(msg);

					m_ServerID = packet->systemAddress;

					if (m_UseNATPunchThroughService) {
						m_IsConnected = m_IsNATPunched ? true : false;
					} else {
						m_IsConnected = true;
					}

					if (m_IsConnected) {
						m_Client->SetTimeoutTime(5000, packet->systemAddress);
						SendRegisterMsg();
					}
					break;
				case ID_CONNECTED_PING:
				case ID_UNCONNECTED_PING:
					msg = "Ping from ";
					msg += packet->systemAddress.ToString(true);
					g_ConsoleMan.PrintString(msg);
					break;
				case ID_SRV_FRAME_SETUP:
					ReceiveFrameSetupMsg(packet);
					break;
				case ID_SRV_FRAME_LINE:
					ReceiveFrameLineMsg(packet);
					break;
				case ID_SRV_FRAME_BOX:
					ReceiveFrameBoxMsg(packet);
					break;
				case ID_SRV_SCENE_SETUP:
					ReceiveSceneSetupMsg(packet);
					break;
				case ID_SRV_SCENE:
					ReceiveSceneMsg(packet);
					break;
				case ID_SRV_SCENE_END:
					ReceiveSceneEndMsg();
					break;
				case ID_SRV_ACCEPTED:
					ReceiveAcceptedMsg();
					break;
				case ID_SRV_TERRAIN:
					ReceiveTerrainChangeMsg(packet);
					break;
				case ID_NAT_SERVER_GUID:
					ReceiveServerGUIDAnswer(packet);
					break;
				case ID_NAT_SERVER_NO_GUID:
					m_IsConnected = false;
					m_IsNATPunched = false;
					break;
				case ID_SRV_POST_EFFECTS:
					ReceivePostEffectsMsg(packet);
					break;
				case ID_SRV_SOUND_EVENTS:
					ReceiveSoundEventsMsg(packet);
					break;
				case ID_SRV_MUSIC_EVENTS:
					ReceiveMusicEventsMsg(packet);
					break;
				case ID_NAT_TARGET_NOT_CONNECTED:
					g_ConsoleMan.PrintString("Failed: ID_NAT_TARGET_NOT_CONNECTED");
					m_IsConnected = false;
					m_IsNATPunched = false;
					break;
				case ID_NAT_TARGET_UNRESPONSIVE:
					g_ConsoleMan.PrintString("Failed: ID_NAT_TARGET_UNRESPONSIVE");
					m_IsConnected = false;
					m_IsNATPunched = false;
					break;
				case ID_NAT_CONNECTION_TO_TARGET_LOST:
					g_ConsoleMan.PrintString("Failed: ID_NAT_CONNECTION_TO_TARGET_LOST");
					m_IsConnected = false;
					m_IsNATPunched = false;
					break;
				case ID_NAT_PUNCHTHROUGH_FAILED:
					g_ConsoleMan.PrintString("Failed: ID_NAT_PUNCHTHROUGH_FAILED");
					m_IsConnected = false;
					m_IsNATPunched = false;
					break;
				case ID_NAT_PUNCHTHROUGH_SUCCEEDED:
					g_ConsoleMan.PrintString("CLIENT: ID_NAT_PUNCHTHROUGH_SUCCEEDED");
					m_IsNATPunched = true;
					ConnectNAT(packet->systemAddress);
					break;
				default:
					break;
			}
		}
	}
}
