//////////////////////////////////////////////////////////////////////////////////////////
// File:            NetworkClient.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the NetworkClient class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

// Without this nested includes somewhere deep inside Allegro will summon winsock.h and it will conflict with winsock2.h from RakNet
// and we can't move "Network.h" here because for whatever reasons everything will collapse
#define WIN32_LEAN_AND_MEAN

#include "ConsoleMan.h"
#include "FrameMan.h"
#include "AudioMan.h"
#include "SettingsMan.h"
#include "PerformanceMan.h"

#include "NetworkClient.h"

#include "LZ4/lz4.h"

#define PLAYERNAMECHARLIMIT 15

namespace RTE
{
	const std::string NetworkClient::m_ClassName = "NetworkClient";

	struct SLDrawBox
	{
		int sourceX;
		int sourceY;
		int sourceW;
		int sourceH;
		int destX;
		int destY;
	};


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Clear
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Clears all the member variables of this NetworkClient, effectively
	//                  resetting the members of this abstraction level only.

	void NetworkClient::Clear()
	{
		m_LastInputSentTime = 0;
		m_ReceivedData = 0;
		m_CompressedData = 0;
		m_IsConnected = false;
		m_IsRegistered = false;
		m_ClientInputFps = 30;
		//m_LastLineReceived = 0;
		m_pSceneBackgroundBitmap = 0;
		m_pSceneForegroundBitmap = 0;
		for (int f = 0; f < FRAMES_TO_REMEMBER; f++)
			m_TargetPos[f].Reset();
		m_CurrentSceneLayerReceived = -1;
		m_CurrentFrame = 0;
		m_UseNATPunchThroughService = false;
		m_ServerGuid = RakNet::UNASSIGNED_RAKNET_GUID;

		m_NATServiceServerID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		m_ServerID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

		m_IsNATPunched = false;

		for (int i = 0; i < MAX_BACKGROUND_LAYERS_TRANSMITTED; i++)
		{
			m_BackgroundBitmaps[i] = 0;
		}
		m_ActiveBackgroundLayers = 0;
		m_SceneWrapsX = false;

		for (int i = 0; i < 3; i++)
		{
			m_aMouseButtonPressedState[i] = -1;
			m_aMouseButtonReleasedState[i] = -1;
		}

		// Stop all sounds received from server
		for (std::map<short int, Sound *>::iterator it = m_Sounds.begin(); it != m_Sounds.end(); ++it)
		{
			it->second->Stop();
			delete it->second;
		}

		m_Sounds.clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the NetworkClient object ready for use.

	int NetworkClient::Create()
	{
		// Record the first client that connects to us so we can pass it to the ping function
		m_ClientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		m_Client = RakNet::RakPeerInterface::GetInstance();

		m_ClientInputFps = g_SettingsMan.GetClientInputFps();

		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Destroy
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Destroys and resets (through Clear()) the NetworkClient object.

	void NetworkClient::Destroy()
	{
		Clear();
	}


	void NetworkClient::SendServerGuidRequest(RakNet::SystemAddress addr, std::string serverName, std::string serverPassword)
	{
		MsgGetServerRequest msg;

		msg.Id = ID_NAT_SERVER_GET_SERVER_GUID;

		strncpy(msg.ServerName, serverName.c_str(), 62);
		strncpy(msg.ServerPassword, serverPassword.c_str(), 62);

		m_Client->Send((const char *)&msg, sizeof(RTE::MsgGetServerRequest), IMMEDIATE_PRIORITY, RELIABLE, 0, addr, false);
	}

	void NetworkClient::ReceiveServerGiudAnswer(RakNet::Packet * p)
	{
		MsgGetServerAnswer * msg = (MsgGetServerAnswer *)p->data;

		m_ServerGuid.FromString(msg->ServerGuid);

		m_NatPunchthroughClient.OpenNAT(m_ServerGuid, m_NATServiceServerID);
		g_ConsoleMan.PrintString("Client: Open NAT to server");
		g_ConsoleMan.PrintString(m_ServerGuid.ToString());
	}

	void NetworkClient::PerformNATPunchThrough(std::string serviceServerName, unsigned short serverPort, std::string playerName, std::string serverName, std::string serverPassword)
	{
		// DEBUG
		//serverName = "127.0.0.1";

		m_UseNATPunchThroughService = true;

		g_ConsoleMan.PrintString("Client: Connecting to NAT service" + serverName);

		RakNet::SocketDescriptor socketDescriptor;
		socketDescriptor.socketFamily = AF_INET;
		m_Client->Startup(8, &socketDescriptor, 1);
		m_Client->SetOccasionalPing(true);
		m_PlayerName = playerName;
		g_ConsoleMan.PrintString("Client: Connect request sent");
		m_NATServiceServerID = ConnectBlocking(m_Client, serviceServerName.c_str(), serverPort);
		if (m_NATServiceServerID == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			g_ConsoleMan.PrintString("Client: Unable to connect");
		}
		else 
		{
			m_Client->SetTimeoutTime(5000, m_NATServiceServerID);
			m_Client->AttachPlugin(&m_NatPunchthroughClient);
			SendServerGuidRequest(m_NATServiceServerID, serverName, serverPassword);
		}
	}

	void NetworkClient::ConnectNAT(RakNet::SystemAddress addr)
	{
		g_ConsoleMan.PrintString("Client: Connecting to server through NAT");
		g_ConsoleMan.PrintString(addr.ToString());

		m_ServerID = addr;

		RakNet::ConnectionAttemptResult car = m_Client->Connect(addr.ToString(false) , addr.GetPort(), NULL, 0);
		if (car == RakNet::CONNECTION_ATTEMPT_STARTED)
		{
			g_ConsoleMan.PrintString("Client: Connect request sent");
		}
		else
		{
			g_ConsoleMan.PrintString("Client: Unable to connect");
		}
	}


	void NetworkClient::Connect(std::string serverName, unsigned short serverPort, std::string playerName)
	{
		// DEBUG
		//serverName = "127.0.0.1";

		g_ConsoleMan.PrintString("Client: Connecting to " + serverName);

		RakNet::SocketDescriptor socketDescriptor;
		socketDescriptor.socketFamily = AF_INET;
		m_Client->Startup(8, &socketDescriptor, 1);
		m_Client->SetOccasionalPing(true);
		m_PlayerName = playerName;
		RakNet::ConnectionAttemptResult car = m_Client->Connect(serverName.c_str(), serverPort, NULL, 0);
		if (car == RakNet::CONNECTION_ATTEMPT_STARTED)
		{
			g_ConsoleMan.PrintString("Client: Connect request sent");
		}
		else 
		{
			g_ConsoleMan.PrintString("Client: Unable to connect");
		}
	}

	void NetworkClient::Disconnect()
	{
		if (m_IsConnected || m_IsRegistered)
			SendDisconnectMsg();
		m_IsRegistered = false;
		m_IsConnected = false;
		Sleep(250);
		RakNet::AddressOrGUID addr = m_Client->GetSystemAddressFromIndex(0);
		m_Client->CloseConnection(addr, true);
		g_ConsoleMan.PrintString("Client: Disconnect");
	}

	void NetworkClient::SendRegisterMsg()
	{
		MsgRegister msg;
		msg.Id = ID_CLT_REGISTER;
		msg.ResolutionX = g_FrameMan.GetResX();
		msg.ResolutionY = g_FrameMan.GetResY();
		strncpy(msg.Name, m_PlayerName.c_str(), PLAYERNAMECHARLIMIT);

		//client->Send(message, (int)strlen(message) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

		m_Client->Send((const char *)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);

		g_ConsoleMan.PrintString("Client: Registration Sent");
	}

	void NetworkClient::SendDisconnectMsg()
	{
		MsgRegister msg;
		msg.Id = ID_CLT_DISCONNECT;

		m_Client->Send((const char *)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);

		g_ConsoleMan.PrintString("Client: Disconnection Sent");
	}


	void NetworkClient::SendInputMsg()
	{
		MsgInput msg;
		msg.Id = ID_CLT_INPUT;

		Vector mouse = g_UInputMan.GetNetworkAccumulatedRawMouseMovement(0);
		msg.MouseX = (int)mouse.GetX();
		msg.MouseY = (int)mouse.GetY();

		// Those are update in Update every frame to cath short events like clicks and releases
		msg.MouseButtonPressed[UInputMan::MOUSE_LEFT] = m_aMouseButtonPressedState[UInputMan::MOUSE_LEFT] == 1 ? true : false;
		msg.MouseButtonPressed[UInputMan::MOUSE_MIDDLE] = m_aMouseButtonPressedState[UInputMan::MOUSE_MIDDLE] == 1 ? true : false;
		msg.MouseButtonPressed[UInputMan::MOUSE_RIGHT] = m_aMouseButtonPressedState[UInputMan::MOUSE_RIGHT] == 1 ? true : false;

//		if (m_aMouseButtonPressedState[UInputMan::MOUSE_LEFT] == 1) g_ConsoleMan.PrintString("#");


		msg.MouseButtonReleased[UInputMan::MOUSE_LEFT] = m_aMouseButtonReleasedState[UInputMan::MOUSE_LEFT] == 1 ? true : false;
		msg.MouseButtonReleased[UInputMan::MOUSE_MIDDLE] = m_aMouseButtonReleasedState[UInputMan::MOUSE_MIDDLE] == 1 ? true : false;
		msg.MouseButtonReleased[UInputMan::MOUSE_RIGHT] = m_aMouseButtonReleasedState[UInputMan::MOUSE_RIGHT] == 1 ? true : false;

//		if (m_aMouseButtonReleasedState[UInputMan::MOUSE_LEFT] == 1) g_ConsoleMan.PrintString("0");

		msg.MouseButtonHeld[UInputMan::MOUSE_LEFT] = g_UInputMan.MouseButtonHeld(UInputMan::MOUSE_LEFT, -1);
		msg.MouseButtonHeld[UInputMan::MOUSE_MIDDLE] = g_UInputMan.MouseButtonHeld(UInputMan::MOUSE_MIDDLE, -1);
		msg.MouseButtonHeld[UInputMan::MOUSE_RIGHT] = g_UInputMan.MouseButtonHeld(UInputMan::MOUSE_RIGHT, -1);

		for (int i = 0; i < 3; i++)
		{
			m_aMouseButtonPressedState[i] = -1;
			m_aMouseButtonReleasedState[i] = -1;
		}

		msg.MouseWheelMoved = g_UInputMan.MouseWheelMoved();

		unsigned int mask = 0x1;

		msg.InputElementHeld = 0;
		msg.InputElementPressed = 0;
		msg.InputElementReleased = 0;

		if (g_UInputMan.KeyHeld(KEY_BACKSPACE))
			msg.ResetActivityVote = true;
		else
			msg.ResetActivityVote = false;

		//char buf[256];

		/*if (msg.MouseButtonHeld[UInputMan::MOUSE_LEFT])
			buf[0] = '1';
		else
			buf[0] = '0';

		if (msg.MouseButtonPressed[UInputMan::MOUSE_LEFT])
			buf[1] = '1';
		else
			buf[1] = '0';

		if (msg.MouseButtonReleased[UInputMan::MOUSE_LEFT])
			buf[2] = '1';
		else
			buf[2] = '0';

		buf[3] = 0;*/

		// Store element states as bit flags
		for (int i = 0; i < UInputMan::INPUT_COUNT; i++)
		{
			if (g_UInputMan.ElementHeld(0, i))
			{
				msg.InputElementHeld = msg.InputElementHeld | mask;
				//buf[i] = '1';
			}
			else 
			{
				//buf[i] = '0';
			}

			if (g_UInputMan.AccumulatedElementPressed(i))
				msg.InputElementPressed = msg.InputElementPressed | mask;

			if (g_UInputMan.AccumulatedElementReleased(i))
				msg.InputElementReleased = msg.InputElementReleased | mask;

			mask <<= 1;
		}

		g_UInputMan.ClearAccumulatedStates();
		//buf[UInputMan::INPUT_COUNT] = 0;

		m_Client->Send((const char *)&msg, sizeof(msg), IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);

		/*if (msg.InputElementHeld > 0 || msg.InputElementPressed > 0 || msg.InputElementReleased > 0)
		{
			g_ConsoleMan.PrintString(buf);
			sprintf_s(buf, sizeof(buf), "%08x %08x %08x", msg.InputElementHeld, msg.InputElementPressed, msg.InputElementReleased);
			g_ConsoleMan.PrintString(buf);
		}*/
	}

	void NetworkClient::ReceiveTerrainChangeMsg(RakNet::Packet * p)
	{
		RTE::MsgTerrainChange * frameData = (RTE::MsgTerrainChange *)p->data;
		if (frameData->SceneId != m_SceneId)
			return;

		//char buf[256];
		//sprintf_s(buf, sizeof(buf), "+ %d %d %d %d", frameData->X, frameData->Y, frameData->W, frameData->H);
		//g_ConsoleMan.PrintString(buf);

		if (frameData->W == 1 && frameData->H == 1)
		{
			BITMAP * bmp = 0;

			if (frameData->Back)
				bmp = m_pSceneBackgroundBitmap;
			else
				bmp = m_pSceneForegroundBitmap;

			putpixel(bmp, frameData->X, frameData->Y, frameData->Color);
		}
		else
		{
			int size = frameData->UncompressedSize;

			if (frameData->DataSize == frameData->UncompressedSize)
				memcpy_s(m_aPixelLineBuffer, size, p->data + sizeof(MsgTerrainChange), size);
			else
				LZ4_decompress_safe((char *)(p->data + sizeof(MsgTerrainChange)), (char *)m_aPixelLineBuffer, frameData->DataSize, size);

			// Copy bitmap data to scene bitmap
			BITMAP * bmp = 0;

			if (frameData->Back)
				bmp = m_pSceneBackgroundBitmap;
			else
				bmp = m_pSceneForegroundBitmap;

			unsigned char * src = m_aPixelLineBuffer;

			for (int y = 0; y < frameData->H && frameData->Y + y < bmp->h; y++)
			{
				memcpy(bmp->line[frameData->Y + y] + frameData->X, src, frameData->W);
				src += frameData->W;
			}
		}
	}

	void NetworkClient::ReceiveSceneMsg(RakNet::Packet * p)
	{
		RTE::MsgSceneLine * frameData = (RTE::MsgSceneLine *)p->data;
		if (frameData->SceneId != m_SceneId)
			return;

		int linex = frameData->X;
		int liney = frameData->Y;

		BITMAP * bmp = 0;

		if (frameData->Layer == 0)
			bmp = m_pSceneBackgroundBitmap;
		if (frameData->Layer == 1)
			bmp = m_pSceneForegroundBitmap;

		int width = frameData->UncompressedSize;
		int pixels = MIN(bmp->w, width);

		m_CurrentSceneLayerReceived = frameData->Layer;

		if (liney < bmp->h)
		{
			if (frameData->DataSize == 0)
			{
				memset(bmp->line[liney] + linex, g_KeyColor, width);
			}
			else
			{
				if (frameData->DataSize == frameData->UncompressedSize)
					memcpy_s(bmp->line[liney] + linex, width, p->data + sizeof(MsgSceneLine), pixels);
				else
					LZ4_decompress_safe((char *)(p->data + sizeof(MsgSceneLine)), (char *)(bmp->line[liney] + linex), frameData->DataSize, width);
			}
		}
	}

	void NetworkClient::DrawBackgrounds(BITMAP * pTargetBitmap)
	{
		for (int i = m_ActiveBackgroundLayers - 1; i >= 0; i--)
		{
			if (m_BackgroundBitmaps[i] != 0)
			{
				//masked_blit(m_BackgroundBitmaps[i], pTargetBitmap, 0, 0, 0, 0, m_BackgroundBitmaps[i]->w, m_BackgroundBitmaps[i]->h);
				Vector scrollOverride(0,0);
				bool scrollOverridden = false;

				int frame = m_CurrentFrame;

				// Set up the target box to draw to on the target bitmap, if it is larger than the scene in either dimension
				Box targetBox(Vector(0, 0), pTargetBitmap->w, pTargetBitmap->h);

				if (!m_SceneWrapsX && pTargetBitmap->w > m_SceneWidth)
				{
					targetBox.m_Corner.m_X = (pTargetBitmap->w - m_SceneWidth) / 2;
					targetBox.m_Width = m_SceneWidth;
				}
				if (pTargetBitmap->h > m_SceneHeight)
				{
					targetBox.m_Corner.m_Y = (pTargetBitmap->h - m_SceneHeight) / 2;
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

				// Overridden scroll position
				if (scrollOverridden)
				{
					offsetX = scrollOverride.GetFloorIntX();
					offsetY = scrollOverride.GetFloorIntY();
				}
				// Regular scroll
				else
				{
					offsetX = floorf(m_aBackgroundLayers[frame][i].OffsetX * m_aBackgroundLayers[frame][i].ScrollRatioX);
					offsetY = floorf(m_aBackgroundLayers[frame][i].OffsetY * m_aBackgroundLayers[frame][i].ScrollRatioY);

					{
						// Only force bounds when doing regular scroll offset because the override is used to do terrain object application tricks and sometimes needs the offsets to be < 0
						// ForceBounds(offsetX, offsetY);
						// WrapPosition(offsetX, offsetY);
						int width =  m_BackgroundBitmaps[i]->w;
						int height =  m_BackgroundBitmaps[i]->h;

						if (m_aBackgroundLayers[frame][i].WrapX) {
							if (offsetX < 0) {
								while (offsetX < 0)
									offsetX += width;
							}
							else if (offsetX >= width) {
								offsetX %= width;
							}
						}

						if (m_aBackgroundLayers[frame][i].WrapY) {
							if (offsetY < 0) {
								while (offsetY < 0)
									offsetY += height;
							}
							else if (offsetY >= height) {
								offsetY %= height;
							}
						}
					}
				}

				// Make target box valid size if it's empty
				if (targetBox.IsEmpty())
				{
					targetBox.SetCorner(Vector(0, 0));
					targetBox.SetWidth(pTargetBitmap->w);
					targetBox.SetHeight(pTargetBitmap->h);
				}

				// Set the clipping rectangle of the target bitmap to match the specified target box
				set_clip_rect(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth() - 1, targetBox.GetCorner().m_Y + targetBox.GetHeight() - 1);

				// Choose the correct blitting function based on transparency setting
				void(*pfBlit)(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = m_aBackgroundLayers[frame][i].DrawTrans ? &masked_blit : &blit;

				// See if this SceneLayer is wider AND higher than the target bitmap; then use simple wrapping logic - oterhwise need to tile
				if (m_BackgroundBitmaps[i]->w >= pTargetBitmap->w && m_BackgroundBitmaps[i]->h >= pTargetBitmap->h)
				{
					sourceX = offsetX;
					sourceY = offsetY;
					sourceW = m_BackgroundBitmaps[i]->w - offsetX;
					sourceH = m_BackgroundBitmaps[i]->h - offsetY;
					destX = targetBox.GetCorner().m_X;
					destY = targetBox.GetCorner().m_Y;
					pfBlit(m_BackgroundBitmaps[i], pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

					sourceX = 0;
					sourceY = offsetY;
					sourceW = offsetX;
					sourceH = m_BackgroundBitmaps[i]->h - offsetY;
					destX = targetBox.GetCorner().m_X + m_BackgroundBitmaps[i]->w - offsetX;
					destY = targetBox.GetCorner().m_Y;
					pfBlit(m_BackgroundBitmaps[i], pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

					sourceX = offsetX;
					sourceY = 0;
					sourceW = m_BackgroundBitmaps[i]->w - offsetX;
					sourceH = offsetY;
					destX = targetBox.GetCorner().m_X;
					destY = targetBox.GetCorner().m_Y + m_BackgroundBitmaps[i]->h - offsetY;
					pfBlit(m_BackgroundBitmaps[i], pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

					sourceX = 0;
					sourceY = 0;
					sourceW = offsetX;
					sourceH = offsetY;
					destX = targetBox.GetCorner().m_X + m_BackgroundBitmaps[i]->w - offsetX;
					destY = targetBox.GetCorner().m_Y + m_BackgroundBitmaps[i]->h - offsetY;
					pfBlit(m_BackgroundBitmaps[i], pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);
				}
				// Target bitmap is larger in some dimension, so need to draw this tiled as many times as necessary to cover the whole target
				else
				{
					int tiledOffsetX = 0;
					int tiledOffsetY = 0;
					// Use the dimensions of the target box, if it has any area at all
					int targetWidth = MIN(pTargetBitmap->w, targetBox.GetWidth());
					int targetHeight = MIN(pTargetBitmap->h, targetBox.GetHeight());
					int toCoverX = offsetX + targetBox.GetCorner().m_X + targetWidth;
					int toCoverY = offsetY + targetBox.GetCorner().m_Y + targetHeight;

					// Check for special case adjustment when the screen is larger than the scene
					bool screenLargerThanSceneX = false;
					bool screenLargerThanSceneY = false;
					if (!scrollOverridden && m_SceneWidth > 0)
					{
						screenLargerThanSceneX = pTargetBitmap->w > m_SceneWidth;
						screenLargerThanSceneY = pTargetBitmap->h > m_SceneHeight;
					}

					// Y tiling
					do
					{
						// X tiling
						do
						{
							sourceX = 0;
							sourceY = 0;
							sourceW = m_BackgroundBitmaps[i]->w;
							sourceH = m_BackgroundBitmaps[i]->h;
							// If the unwrapped and untiled direction can't cover the target area, place it in the middle of the target bitmap, and leave the excess perimeter on each side untouched
							destX = (!m_aBackgroundLayers[frame][i].WrapX && screenLargerThanSceneX) ? ((pTargetBitmap->w / 2) - (m_BackgroundBitmaps[i]->w / 2)) : (targetBox.GetCorner().m_X + tiledOffsetX - offsetX);
							destY = (!m_aBackgroundLayers[frame][i].WrapY && screenLargerThanSceneY) ? ((pTargetBitmap->h / 2) - (m_BackgroundBitmaps[i]->h / 2)) : (targetBox.GetCorner().m_Y + tiledOffsetY - offsetY);

							pfBlit(m_BackgroundBitmaps[i], pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

							tiledOffsetX += m_BackgroundBitmaps[i]->w;
						}
						// Only tile if we're supposed to wrap widthwise
						while (m_aBackgroundLayers[frame][i].WrapX && toCoverX > tiledOffsetX);

						tiledOffsetY += m_BackgroundBitmaps[i]->h;
					}
					// Only tile if we're supposed to wrap heightwise
					while (m_aBackgroundLayers[frame][i].WrapY && toCoverY > tiledOffsetY);

					// TODO: Do this above instead, testing down here only
							// Detect if nonwrapping layer dimensions can't cover the whole target area with its main bitmap. If so, fill in the gap with appropriate solid color sampled from the hanging edge
					if (!m_aBackgroundLayers[frame][i].WrapX && !screenLargerThanSceneX && m_aBackgroundLayers[frame][i].ScrollRatioX < 0)
					{
						if (m_aBackgroundLayers[frame][i].FillLeftColor != g_KeyColor && offsetX != 0)
							rectfill(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X - offsetX, targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_aBackgroundLayers[frame][i].FillLeftColor);
						if (m_aBackgroundLayers[frame][i].FillRightColor != g_KeyColor)
							rectfill(pTargetBitmap, (targetBox.GetCorner().m_X - offsetX) + m_BackgroundBitmaps[i]->w, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_aBackgroundLayers[frame][i].FillRightColor);
					}

					if (!m_aBackgroundLayers[frame][i].WrapY && !screenLargerThanSceneY && m_aBackgroundLayers[frame][i].ScrollRatioY < 0)
					{
						if (m_aBackgroundLayers[frame][i].FillUpColor != g_KeyColor && offsetY != 0)
							rectfill(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y - offsetY, m_aBackgroundLayers[frame][i].FillUpColor);
						if (m_aBackgroundLayers[frame][i].FillDownColor != g_KeyColor)
							rectfill(pTargetBitmap, targetBox.GetCorner().m_X, (targetBox.GetCorner().m_Y - offsetY) + m_BackgroundBitmaps[i]->h, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_aBackgroundLayers[frame][i].FillDownColor);
					}
				}

				// Reset the clip rect back to the entire target bitmap
				set_clip_rect(pTargetBitmap, 0, 0, pTargetBitmap->w - 1, pTargetBitmap->h - 1);

			}
		}
	}

	void NetworkClient::ReceivePosteEffectsMsg(RakNet::Packet * p)
	{
		MsgPostEffects * msg = (MsgPostEffects *)p->data;
		PostEffectNetworkData * effDataPtr = (PostEffectNetworkData *)((char *)msg + sizeof(MsgPostEffects));

		for (int i = 0; i < msg->PostEffectsCount; i++)
		{
			BITMAP * bmp = 0;
			std::string bitmapPath = ContentFile::GetPathFromHash(effDataPtr->BitmapHash);
			if (bitmapPath != "")
			{
				ContentFile fl(bitmapPath.c_str());
				bmp = fl.GetAsBitmap();
			}

			if (bmp)
				m_PostEffects[msg->FrameNumber].push_back(PostEffect(Vector(effDataPtr->X, effDataPtr->Y), bmp, 0, effDataPtr->Strength, effDataPtr->Angle));
			else
			{
				//char buf[128];
				//sprintf_s(buf, sizeof(buf), "%lu", effDataPtr->BitmapHash);
				//g_ConsoleMan.PrintString(buf);
			}
			effDataPtr++;
		}

		//char buf[128];
		//sprintf_s(buf, sizeof(buf), "%d %d %d", msg->FrameNumber, m_PostEffects[msg->FrameNumber].size(), msg->PostEffectsCount);
		//g_ConsoleMan.PrintString(buf);
	}

	void NetworkClient::ReceiveSoundEventsMsg(RakNet::Packet * p)
	{
		MsgSoundEvents * msg = (MsgSoundEvents *)p->data;
		AudioMan::NetworkSoundData * sndDataPtr = (AudioMan::NetworkSoundData *)((char *)msg + sizeof(MsgSoundEvents));

		for (int i = 0; i < msg->SoundEventsCount; i++)
		{
			if (sndDataPtr->State == AudioMan::SOUND_PLAY)
			{
				BITMAP * bmp = 0;
				std::string path = ContentFile::GetPathFromHash(sndDataPtr->SoundHash);
				if (path != "")
				{
					Sound *pSound = new Sound();
					pSound->Create(path, sndDataPtr->AffectedByPitch > 0 ? true : false, sndDataPtr->Loops);
					g_AudioMan.SetSoundPitch(pSound, sndDataPtr->Pitch);

					pSound->Play(sndDataPtr->Distance);

					// Stop sound at this channel just in case
					if (m_Sounds.count(sndDataPtr->Channel) > 0)
					{
						m_Sounds[sndDataPtr->Channel]->Stop();
						delete m_Sounds[sndDataPtr->Channel];
					}

					m_Sounds[sndDataPtr->Channel] = pSound;

					//char buf[128];
					//sprintf_s(buf, sizeof(buf), "PLAY %d %d %f %s", sndDataPtr->Loops, pSound->GetCurrentChannel(), sndDataPtr->Pitch, path.c_str());
					//g_ConsoleMan.PrintString(buf);
				}
				else 
				{
					//char buf[128];
					//sprintf_s(buf, sizeof(buf), "NO SOUND %d", sndDataPtr->SoundHash);
					//g_ConsoleMan.PrintString(buf);
				}
			}
			else if (sndDataPtr->State == AudioMan::SOUND_SET_PITCH)
			{
				if (sndDataPtr->SoundHash == 0)
				{
					g_AudioMan.SetGlobalPitch(sndDataPtr->Pitch, sndDataPtr->AffectedByPitch > 0 ? true : false);
					//char buf[128];
					//sprintf_s(buf, sizeof(buf), "GLOBAL PITCH %f %d", sndDataPtr->Pitch, sndDataPtr->AffectedByPitch);
					//g_ConsoleMan.PrintString(buf);
				}
				else
				{
					if (m_Sounds.count(sndDataPtr->Channel) > 0)
					{
						//char buf[128];
						//sprintf_s(buf, sizeof(buf), "PITCH %d %f", m_Sounds[sndDataPtr->Channel]->GetCurrentChannel(), sndDataPtr->Pitch);
						//g_ConsoleMan.PrintString(buf);

						g_AudioMan.SetSoundPitch(m_Sounds[sndDataPtr->Channel], sndDataPtr->Pitch);
					}
					else
					{
						//char buf[128];
						//sprintf_s(buf, sizeof(buf), "Not found %d", sndDataPtr->Channel);
						//g_ConsoleMan.PrintString(buf);
					}
				}
			}
			else if (sndDataPtr->State == AudioMan::SOUND_STOP)
			{
				if (m_Sounds.count(sndDataPtr->Channel) > 0)
				{
					//char buf[128];
					//sprintf_s(buf, sizeof(buf), "STOP %d", m_Sounds[sndDataPtr->Channel]->GetCurrentChannel());
					//g_ConsoleMan.PrintString(buf);

					m_Sounds[sndDataPtr->Channel]->Stop();
				}
			}

			sndDataPtr++;
		}

		//char buf[128];
		//sprintf_s(buf, sizeof(buf), "%d %d %d", msg->FrameNumber, m_PostEffects[msg->FrameNumber].size(), msg->PostEffectsCount);
		//g_ConsoleMan.PrintString(buf);
	}


	void NetworkClient::ReceiveMusicEventsMsg(RakNet::Packet * p)
	{
		MsgMusicEvents * msg = (MsgMusicEvents *)p->data;
		AudioMan::NetworkMusicData * sndDataPtr = (AudioMan::NetworkMusicData *)((char *)msg + sizeof(MsgMusicEvents));

		for (int i = 0; i < msg->MusicEventsCount; i++)
		{
			if (sndDataPtr->State == AudioMan::MUSIC_PLAY)
			{
				char path[256];

				memset(path, 0, 256);
				strncpy(path, sndDataPtr->Path, 255);

				char buf[128];
				sprintf_s(buf, sizeof(buf), "MUSIC %s %d", path, sndDataPtr->Loops);
				g_ConsoleMan.PrintString(buf);

				g_AudioMan.PlayMusic(path, sndDataPtr->Loops);
				if (sndDataPtr->Position > 0)
					g_AudioMan.SetMusicPosition(sndDataPtr->Position);
			} 
			else if (sndDataPtr->State == AudioMan::MUSIC_STOP)
			{ 
				g_ConsoleMan.PrintString("MUSIC STOP");
				g_AudioMan.StopMusic();
			}
			else if (sndDataPtr->State == AudioMan::MUSIC_SET_PITCH)
			{
				//char buf[128];
				//sprintf_s(buf, sizeof(buf), "MUSIC PITCH %f", sndDataPtr->Pitch);
				//g_ConsoleMan.PrintString(buf);

				//g_AudioMan.SetMusicPitch(sndDataPtr->Pitch);
			}
			else if (sndDataPtr->State == AudioMan::MUSIC_SILENCE)
			{
				g_ConsoleMan.PrintString("MUSIC SILENCE");

				//g_AudioMan.QueueSilence(sndDataPtr->Loops);
				g_AudioMan.StopMusic();
			}
			sndDataPtr++;
		}
	}


	void NetworkClient::DrawPostEffects(int frame)
	{
		g_PostProcessMan.SetNetworkPostEffectsList(0, m_PostEffects[frame]);
		//m_PostEffects[frame].clear();
	}

	void NetworkClient::DrawFrame()
	{
		while (g_FrameMan.IsNetworkBitmapLocked(0));
		BITMAP * src_bmp = g_FrameMan.GetNetworkBackBufferIntermediate8Ready(0);
		BITMAP * dst_bmp = g_FrameMan.GetNetworkBackBuffer8Ready(0);

		BITMAP * src_gui_bmp = g_FrameMan.GetNetworkBackBufferIntermediateGUI8Ready(0);
		BITMAP * dst_gui_bmp = g_FrameMan.GetNetworkBackBufferGUI8Ready(0);

		// Have to clear to color to fallback if there's no skybox on client
		clear_to_color(dst_bmp, g_BlackColor);
		clear_to_color(dst_gui_bmp, g_KeyColor);

		// Draw Scene background
		int sourceX = m_TargetPos[m_CurrentFrame].m_X;
		int sourceY = m_TargetPos[m_CurrentFrame].m_Y;
		int sourceW = src_bmp->w;
		int sourceH = src_bmp->h;
		int destX = 0;
		int destY = 0;

		DrawBackgrounds(dst_bmp);

		masked_blit(m_pSceneBackgroundBitmap, dst_bmp, sourceX, sourceY, destX, destY, src_bmp->w, src_bmp->h);

		// Draw if the out of seam portion is to the left
		if (sourceX < 0)
		{
			int newSourceX = m_pSceneBackgroundBitmap->w + sourceX;

			masked_blit(m_pSceneBackgroundBitmap, dst_bmp, newSourceX, sourceY, destX, destY, src_bmp->w, src_bmp->h);
		}
		// Draw if the out of seam portion is to the right
		else if (sourceX + g_FrameMan.GetResX() >= m_pSceneBackgroundBitmap->w)
		{
			int newDestX = m_pSceneBackgroundBitmap->w - sourceX;
			int width = g_FrameMan.GetResX() - newDestX;

			masked_blit(m_pSceneBackgroundBitmap, dst_bmp, 0, sourceY, newDestX, destY, width, src_bmp->h);
		}

		//draw_sprite(src_bmp, dst_bmp, 0, 0);
		masked_blit(src_bmp, dst_bmp, 0, 0, 0, 0, src_bmp->w, src_bmp->h);
		masked_blit(src_gui_bmp, dst_gui_bmp, 0, 0, 0, 0, src_bmp->w, src_bmp->h);

		masked_blit(m_pSceneForegroundBitmap, dst_bmp, sourceX, sourceY, destX, destY, src_bmp->w, src_bmp->h);

		// Draw if the out of seam portion is to the left
		if (sourceX < 0)
		{
			int newSourceX = m_pSceneForegroundBitmap->w + sourceX;

			masked_blit(m_pSceneForegroundBitmap, dst_bmp, newSourceX, sourceY, destX, destY, src_bmp->w, src_bmp->h);
		}
		// Draw if the out of seam portion is to the right
		else if (sourceX + g_FrameMan.GetResX() >= m_pSceneForegroundBitmap->w)
		{
			int newDestX = m_pSceneForegroundBitmap->w - sourceX;
			int width = g_FrameMan.GetResX() - newDestX;

			masked_blit(m_pSceneForegroundBitmap, dst_bmp, 0, sourceY, newDestX, destY, width, src_bmp->h);
		}

		DrawPostEffects(m_CurrentFrame);

		g_PerformanceMan.SetCurrentPing(GetPing());

		//clear_to_color(src_bmp, g_KeyColor);
		//clear_to_color(src_gui_bmp, g_KeyColor);

		// DEBUG!!!
		//if (m_CurrentFrame != m_CurrentFrameSetup)
		//	clear_to_color(dst_bmp, g_BlackColor);
	}

	void NetworkClient::ReceiveFrameBoxMsg(RakNet::Packet * p)
	{
		RTE::MsgFrameBox * frameData = (RTE::MsgFrameBox *)p->data;
		int bpx = frameData->BoxX;
		int bpy = frameData->BoxY;

		m_CurrentSceneLayerReceived = -1;

		// Looks like we've started receiving a new frame, time to draw current frame then
		//if (lineNumber < m_LastLineReceived/* && frameData->Layer == 1*/)
		/*if (m_CurrentFrame != frameData->FrameNumber)
		{
			DrawFrame();

			if (frameData->FrameNumber >= 0 && frameData->FrameNumber < FRAMES_TO_REMEMBER)
			{
				m_PostEffects[m_CurrentFrame].clear();
				m_CurrentFrame = frameData->FrameNumber;
			}
		}*/
		//m_LastLineReceived = lineNumber;

		BITMAP * bmp = 0;

		if (frameData->Layer == 0)
			bmp = g_FrameMan.GetNetworkBackBufferIntermediate8Ready(0);
		if (frameData->Layer == 1)
			bmp = g_FrameMan.GetNetworkBackBufferIntermediateGUI8Ready(0);

		acquire_bitmap(bmp);

		int maxWidth = frameData->BoxWidth;
		int maxHeight = frameData->BoxHeight;
		int size = frameData->DataSize;

		m_ReceivedData += frameData->DataSize;
		m_CompressedData += frameData->UncompressedSize;

		if (bpx + maxWidth - 1 < bmp->w && bpy + maxHeight - 1 < bmp->h && bpx >= 0 && bpy >= 0)
		{
			// Unpack box
			if (frameData->DataSize == 0)
			{
				//memset(bmp->line[lineNumber], g_KeyColor, bmp->w);
				rectfill(bmp, bpx, bpy, bpx + maxWidth - 1, bpy + maxHeight - 1, g_KeyColor);
			}
			else
			{
				if (frameData->DataSize == frameData->UncompressedSize)
					memcpy_s(m_aPixelLineBuffer, size, p->data + sizeof(MsgFrameBox), size);
				else
					LZ4_decompress_safe((char *)(p->data + sizeof(MsgFrameBox)), (char *)(m_aPixelLineBuffer), size, frameData->UncompressedSize);

				// Copy box to bitmap line by line
				unsigned char * lineAddr = m_aPixelLineBuffer;
				for (int y = 0; y < maxHeight; y++)
				{
					memcpy_s(bmp->line[bpy + y] + bpx, maxWidth, lineAddr, maxWidth);
					lineAddr += maxWidth;
				}

				if (g_UInputMan.KeyHeld(KEY_0))
					rect(bmp, bpx, bpy, bpx + maxWidth - 1, bpy + maxHeight - 1, g_BlackColor);
			}

		}

		release_bitmap(bmp);
	}

	void NetworkClient::ReceiveFrameLineMsg(RakNet::Packet * p)
	{
		RTE::MsgFrameLine * frameData = (RTE::MsgFrameLine *)p->data;
		int lineNumber = frameData->LineNumber;

		m_CurrentSceneLayerReceived = -1;

		// Looks like we've started receiving a new frame, time to draw current frame then
		//if (lineNumber < m_LastLineReceived/* && frameData->Layer == 1*/)
		/*if (m_CurrentFrame != frameData->FrameNumber)
		{
			DrawFrame();

			if (frameData->FrameNumber >= 0 && frameData->FrameNumber < FRAMES_TO_REMEMBER)
				m_CurrentFrame = frameData->FrameNumber;
		}*/
		//m_LastLineReceived = lineNumber;

		BITMAP * bmp = 0;
		
		if (frameData->Layer == 0)
			bmp = g_FrameMan.GetNetworkBackBufferIntermediate8Ready(0);
		if (frameData->Layer == 1)
			bmp = g_FrameMan.GetNetworkBackBufferIntermediateGUI8Ready(0);

		acquire_bitmap(bmp);

		int width = frameData->DataSize;
		int pixels = MIN(bmp->w, width);


		m_ReceivedData += frameData->DataSize;
		m_CompressedData += frameData->UncompressedSize;

		if (lineNumber < bmp->h)
		{
			if (frameData->DataSize == 0)
			{
				memset(bmp->line[lineNumber], g_KeyColor, bmp->w);
			}
			else 
			{
				if (frameData->DataSize == frameData->UncompressedSize)
					memcpy_s(bmp->line[lineNumber], bmp->w, p->data + sizeof(MsgFrameLine), pixels);
				else
					LZ4_decompress_safe((char *)(p->data + sizeof(MsgFrameLine)), (char *)(bmp->line[lineNumber]), frameData->DataSize, bmp->w);
			}
		}

		release_bitmap(bmp);
	}

	void NetworkClient::ReceiveAcceptedMsg()
	{
		g_ConsoleMan.PrintString("Client: Registration accepted.");
		m_IsRegistered = true;
	}

	void NetworkClient::ReceiveSceneEndMsg()
	{
		g_ConsoleMan.PrintString("Client: Scene received.");
		SendSceneAcceptedMsg();
	}

	void NetworkClient::ReceiveFrameSetupMsg(RakNet::Packet * p)
	{
		RTE::MsgFrameSetup * frameData = (RTE::MsgFrameSetup *)p->data;
		if (frameData->FrameNumber < 0 || frameData->FrameNumber >= FRAMES_TO_REMEMBER)
			return;

		DrawFrame();

		m_PostEffects[m_CurrentFrame].clear();
		m_CurrentFrame = frameData->FrameNumber;

		m_TargetPos[m_CurrentFrame].m_X = frameData->TargetPosX;
		m_TargetPos[m_CurrentFrame].m_Y = frameData->TargetPosY;

		for (int i = 0; i < MAX_BACKGROUND_LAYERS_TRANSMITTED; i++)
		{
			m_aBackgroundLayers[m_CurrentFrame][i].OffsetX = frameData->OffsetX[i];
			m_aBackgroundLayers[m_CurrentFrame][i].OffsetY = frameData->OffsetY[i];
		}
	}

	void NetworkClient::ReceiveSceneSetupMsg(RakNet::Packet * p)
	{
		clear_to_color(g_FrameMan.GetNetworkBackBufferIntermediateGUI8Ready(0), g_KeyColor);
		clear_to_color(g_FrameMan.GetNetworkBackBufferGUI8Ready(0), g_KeyColor);

		RTE::MsgSceneSetup * frameData = (RTE::MsgSceneSetup *)p->data;

		m_SceneId = frameData->SceneId;

		if (m_pSceneBackgroundBitmap)
			destroy_bitmap(m_pSceneBackgroundBitmap);
		if (m_pSceneForegroundBitmap)
			destroy_bitmap(m_pSceneForegroundBitmap);

		m_pSceneBackgroundBitmap = create_bitmap_ex(8, frameData->Width, frameData->Height);
		m_pSceneForegroundBitmap = create_bitmap_ex(8, frameData->Width, frameData->Height);
		// This is purely for aesthetic reasons to draw bitmap during level loading
		clear_to_color(m_pSceneForegroundBitmap, g_KeyColor);

		m_SceneWrapsX = frameData->SceneWrapsX;
		m_SceneWidth = frameData->Width;
		m_SceneHeight = frameData->Height;

		m_ActiveBackgroundLayers = frameData->BackgroundLayerCount;

		for (int i = 0; i < m_ActiveBackgroundLayers; i++)
		{
			size_t hash = frameData->BackgroundLayers[i].BitmapHash;
			std::string bitmapPath = ContentFile::GetPathFromHash(hash);
			if (bitmapPath != "")
			{
				ContentFile fl(bitmapPath.c_str());
				m_BackgroundBitmaps[i] = fl.GetAsBitmap();
			}
			else
			{
				m_BackgroundBitmaps[i] = 0;
			}

			for (int f = 0; f < FRAMES_TO_REMEMBER; f++)
			{
				m_aBackgroundLayers[f][i].DrawTrans = frameData->BackgroundLayers[i].DrawTrans;
				m_aBackgroundLayers[f][i].OffsetX = frameData->BackgroundLayers[i].OffsetX;
				m_aBackgroundLayers[f][i].OffsetY = frameData->BackgroundLayers[i].OffsetY;

				m_aBackgroundLayers[f][i].ScrollInfoX = frameData->BackgroundLayers[i].ScrollInfoX;
				m_aBackgroundLayers[f][i].ScrollInfoY = frameData->BackgroundLayers[i].ScrollInfoY;
				m_aBackgroundLayers[f][i].ScrollRatioX = frameData->BackgroundLayers[i].ScrollRatioX;
				m_aBackgroundLayers[f][i].ScrollRatioY = frameData->BackgroundLayers[i].ScrollRatioY;
				m_aBackgroundLayers[f][i].ScaleFactorX = frameData->BackgroundLayers[i].ScaleFactorX;
				m_aBackgroundLayers[f][i].ScaleFactorY = frameData->BackgroundLayers[i].ScaleFactorY;
				m_aBackgroundLayers[f][i].ScaleInverseX = frameData->BackgroundLayers[i].ScaleInverseX;
				m_aBackgroundLayers[f][i].ScaleInverseY = frameData->BackgroundLayers[i].ScaleInverseY;
				m_aBackgroundLayers[f][i].ScaledDimensionsX = frameData->BackgroundLayers[i].ScaledDimensionsX;
				m_aBackgroundLayers[f][i].ScaledDimensionsY = frameData->BackgroundLayers[i].ScaledDimensionsY;

				m_aBackgroundLayers[f][i].WrapX = frameData->BackgroundLayers[i].WrapX;
				m_aBackgroundLayers[f][i].WrapY = frameData->BackgroundLayers[i].WrapY;

				m_aBackgroundLayers[f][i].FillLeftColor = frameData->BackgroundLayers[i].FillLeftColor;
				m_aBackgroundLayers[f][i].FillRightColor = frameData->BackgroundLayers[i].FillRightColor;
				m_aBackgroundLayers[f][i].FillUpColor = frameData->BackgroundLayers[i].FillUpColor;
				m_aBackgroundLayers[f][i].FillDownColor = frameData->BackgroundLayers[i].FillDownColor;
			}
		}

		SendSceneSetupAcceptedMsg();
		g_ConsoleMan.PrintString("Client: Scene setup accepted");
	}

	void NetworkClient::SendSceneSetupAcceptedMsg()
	{
		MsgRegister msg;
		msg.Id = ID_CLT_SCENE_SETUP_ACCEPTED;

		m_Client->Send((const char *)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);

		g_ConsoleMan.PrintString("Client: Scene setup ACK Sent");
	}

	void NetworkClient::SendSceneAcceptedMsg()
	{
		MsgRegister msg;
		msg.Id = ID_CLT_SCENE_ACCEPTED;

		m_Client->Send((const char *)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_ServerID, false);

		g_ConsoleMan.PrintString("Client: Scene ACK Sent");
	}

	unsigned int NetworkClient::GetPing()
	{
		if (IsConnectedAndRegistred())
		{
			return m_Client->GetLastPing(m_ServerID);
		}
		return 0;
	}

	void NetworkClient::Update()
	{
		RakNet::Packet *p;
		std::string msg;

		for (p = m_Client->Receive(); p; m_Client->DeallocatePacket(p), p = m_Client->Receive())
		{
			// We got a packet, get the identifier with our handy function
			unsigned char packetIdentifier = GetPacketIdentifier(p);

			// Check if this is a network message packet
			switch (packetIdentifier)
			{
			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				g_ConsoleMan.PrintString("ID_DISCONNECTION_NOTIFICATION");
				Disconnect();
				break;
			case ID_ALREADY_CONNECTED:
				// Connection lost normally
				msg = "ID_ALREADY_CONNECTED with guid";
				msg += p->guid.ToString();
				g_ConsoleMan.PrintString(msg);
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				g_ConsoleMan.PrintString("ID_INCOMPATIBLE_PROTOCOL_VERSION");
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				g_ConsoleMan.PrintString("ID_REMOTE_DISCONNECTION_NOTIFICATION");
				break;
			case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				g_ConsoleMan.PrintString("ID_REMOTE_CONNECTION_LOST");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
				g_ConsoleMan.PrintString("ID_REMOTE_NEW_INCOMING_CONNECTION");
				break;
			case ID_CONNECTION_BANNED: // Banned from this server
				g_ConsoleMan.PrintString("We are banned from this server.");
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				g_ConsoleMan.PrintString("Connection attempt failed");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				// Sorry, the server is full.  I don't do anything here but
				// A real app should tell the user
				g_ConsoleMan.PrintString("ID_NO_FREE_INCOMING_CONNECTIONS");
				break;
			case ID_INVALID_PASSWORD:
				g_ConsoleMan.PrintString("ID_INVALID_PASSWORD");
				break;
			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				g_ConsoleMan.PrintString("ID_CONNECTION_LOST");
				break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
				// This tells the client they have connected
				msg = "ID_CONNECTION_REQUEST_ACCEPTED to ";
				msg += p->systemAddress.ToString(true);
				msg += " guid ";
				msg += p->guid.ToString();
				g_ConsoleMan.PrintString(msg);

				msg = "My external address is";
				msg += m_Client->GetExternalID(p->systemAddress).ToString(true);
				g_ConsoleMan.PrintString(msg);

				m_ServerID = p->systemAddress;

				if (m_UseNATPunchThroughService)
				{
					if (m_IsNATPunched)
						m_IsConnected = true;
					else
						m_IsConnected = false;
				} 
				else
				{
					m_IsConnected = true;
				}

				if (m_IsConnected)
				{
					m_Client->SetTimeoutTime(5000, p->systemAddress);
					SendRegisterMsg();
				}

				break;
			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				msg = "Ping from ";
				msg += p->systemAddress.ToString(true);
				g_ConsoleMan.PrintString(msg);
				break;

			case ID_SRV_FRAME_SETUP:
				ReceiveFrameSetupMsg(p);
				break;

			case ID_SRV_FRAME_LINE:
				ReceiveFrameLineMsg(p);
				break;

			case ID_SRV_FRAME_BOX:
				ReceiveFrameBoxMsg(p);
				break;

			case ID_SRV_SCENE_SETUP:
				ReceiveSceneSetupMsg(p);
				break;

			case ID_SRV_SCENE:
				ReceiveSceneMsg(p);
				break;

			case ID_SRV_SCENE_END:
				ReceiveSceneEndMsg();
				break;

			case ID_SRV_ACCEPTED:
				ReceiveAcceptedMsg();
				break;

			case ID_SRV_TERRAIN:
				ReceiveTerrainChangeMsg(p);
				break;

			case ID_NAT_SERVER_GUID:
				ReceiveServerGiudAnswer(p);
				break;

			case ID_NAT_SERVER_NO_GUID:
				m_IsConnected = false;
				m_IsNATPunched = false;
				break;

			case ID_SRV_POST_EFFECTS:
				ReceivePosteEffectsMsg(p);
				break;

			case ID_SRV_SOUND_EVENTS:
				ReceiveSoundEventsMsg(p);
				break;

			case ID_SRV_MUSIC_EVENTS:
				ReceiveMusicEventsMsg(p);
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
				g_ConsoleMan.PrintString("Client: ID_NAT_PUNCHTHROUGH_SUCCEEDED");
				m_IsNATPunched = true;
				ConnectNAT(p->systemAddress);
				break;

			default:
				// It's a client, so just show the message
				//printf("%s\n", p->data);
				break;
			}
		}

		// Draw level loading animation
		if (m_CurrentSceneLayerReceived != -1)
		{
			BITMAP * bmp = 0;

			if (m_CurrentSceneLayerReceived == -1)
				bmp = m_pSceneBackgroundBitmap;

			if (m_CurrentSceneLayerReceived == 0)
				bmp = m_pSceneBackgroundBitmap;
			if (m_CurrentSceneLayerReceived == 1)
				bmp = m_pSceneForegroundBitmap;

			BITMAP * dst_bmp = g_FrameMan.GetNetworkBackBuffer8Ready(0);

			clear_to_color(dst_bmp, g_BlackColor);

			float scale = (float)dst_bmp->w / (float)bmp->w;
			int w = dst_bmp->w;
			int h = (float)bmp->h * scale;

			int x = 0;
			int y = g_FrameMan.GetResY() / 2 - h / 2;
			if (h >= g_FrameMan.GetResY())
				y = 0;

			// Recalculate everything for tall maps
			if ((float)bmp->h / (float)bmp->w > 1)
			{
				scale = (float)dst_bmp->h / (float)bmp->h;
				h = dst_bmp->h;
				w = (float)bmp->w * scale;

				x = g_FrameMan.GetResX() / 2 - w / 2;
				y = 0;
				if (w >= g_FrameMan.GetResX())
					x = 0;
			}

			// Draw previous layer
			if (m_CurrentSceneLayerReceived == 1)
				masked_stretch_blit(m_pSceneBackgroundBitmap, dst_bmp, 0, 0, bmp->w, bmp->h, x, y, w, h);

			masked_stretch_blit(bmp, dst_bmp, 0, 0, bmp->w, bmp->h, x, y, w, h);
		}

		// Detect short mouse events like presses and releases. Helds are detected during input send
		if (m_aMouseButtonPressedState[UInputMan::MOUSE_LEFT] < 1)
		{
			m_aMouseButtonPressedState[UInputMan::MOUSE_LEFT] = g_UInputMan.MouseButtonPressed(UInputMan::MOUSE_LEFT, -1) ? 1 : 0;
			//if (m_aMouseButtonPressedState[UInputMan::MOUSE_LEFT] == 1) g_ConsoleMan.PrintString("+");
		}

		if (m_aMouseButtonPressedState[UInputMan::MOUSE_RIGHT] < 1)
			m_aMouseButtonPressedState[UInputMan::MOUSE_RIGHT] = g_UInputMan.MouseButtonPressed(UInputMan::MOUSE_RIGHT, -1) ? 1 : 0;

		if (m_aMouseButtonPressedState[UInputMan::MOUSE_MIDDLE] < 1)
			m_aMouseButtonPressedState[UInputMan::MOUSE_MIDDLE] = g_UInputMan.MouseButtonPressed(UInputMan::MOUSE_MIDDLE, -1) ? 1 : 0;


		if (m_aMouseButtonReleasedState[UInputMan::MOUSE_LEFT] < 1)
		{
			m_aMouseButtonReleasedState[UInputMan::MOUSE_LEFT] = g_UInputMan.MouseButtonReleased(UInputMan::MOUSE_LEFT, -1) ? 1 : 0;
			//if (m_aMouseButtonPressedState[UInputMan::MOUSE_LEFT] == 1) g_ConsoleMan.PrintString("-");
		}

		if (m_aMouseButtonReleasedState[UInputMan::MOUSE_RIGHT] < 1)
			m_aMouseButtonReleasedState[UInputMan::MOUSE_RIGHT] = g_UInputMan.MouseButtonReleased(UInputMan::MOUSE_RIGHT, -1) ? 1 : 0;

		if (m_aMouseButtonReleasedState[UInputMan::MOUSE_MIDDLE] < 1)
			m_aMouseButtonReleasedState[UInputMan::MOUSE_MIDDLE] = g_UInputMan.MouseButtonReleased(UInputMan::MOUSE_MIDDLE, -1) ? 1 : 0;


		// Input is sent at whetever settings are set in inputs per second
		float ips = m_ClientInputFps;
#if defined DEBUG_BUILD || defined MIN_DEBUG_BUILD
		// Reduce input rate for debugging because it may overflow the input queue
		ips = 10; 
#endif

		int64_t currentTicks = g_TimerMan.GetRealTickCount();

		if (currentTicks - m_LastInputSentTime < 0)
			m_LastInputSentTime = currentTicks;

		if ((double)(currentTicks - m_LastInputSentTime) / g_TimerMan.GetTicksPerSecond() > 1.0 / ips)
		{
			m_LastInputSentTime = g_TimerMan.GetRealTickCount();

			if (IsConnectedAndRegistred())
				SendInputMsg();
		}

	}

	unsigned char NetworkClient::GetPacketIdentifier(RakNet::Packet *p)
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

	RakNet::SystemAddress NetworkClient::ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, unsigned short int port)
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
