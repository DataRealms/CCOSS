#ifndef _RTENETWORKCLIENT_
#define _RTENETWORKCLIENT_

#include "Singleton.h"

#include <WinSock2.h>
#include "RakPeerInterface.h"
#include "NetworkMessages.h"

#include "NatPunchthroughClient.h"

#define g_NetworkClient NetworkClient::Instance()

/////////////////////////////////////////////////////////////////////////
// TODO: Get Weegee to comment all these because I don't even.
/////////////////////////////////////////////////////////////////////////

namespace RTE {

	struct PostEffect;
	class SoundContainer;

	/// <summary>
	/// The centralized singleton manager of the network multiplayer client.
	/// </summary>
	class NetworkClient : public Singleton<NetworkClient> {

	public:

		/// <summary>
		/// 
		/// </summary>
		struct MsgInput {
			unsigned char Id;

			int MouseX;
			int MouseY;
			bool MouseButtonPressed[MAX_MOUSE_BUTTONS];
			bool MouseButtonReleased[MAX_MOUSE_BUTTONS];
			bool MouseButtonHeld[MAX_MOUSE_BUTTONS];
			bool ResetActivityVote;

			int MouseWheelMoved;

			unsigned int InputElementPressed;
			unsigned int InputElementReleased;
			unsigned int InputElementHeld;
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a NetworkClient object in system memory. Create() should be called before using the object.
		/// </summary>
		NetworkClient() { Clear(); Create(); }

		/// <summary>
		/// Makes the NetworkClient object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a NetworkClient object before deletion from system memory.
		/// </summary>
		virtual ~NetworkClient() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the NetworkClient object.
		/// </summary>
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets whether the client is connected and registered to a server.
		/// </summary>
		/// <returns>Whether the client is connected and registered to a server.</returns>
		bool IsConnectedAndRegistred() const { return m_IsConnected && m_IsRegistered; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this NetworkClient. Supposed to be done every frame.
		/// </summary>
		void Update();
#pragma endregion

#pragma region Connection Handling
		/// <summary>
		/// Connects the client to a server.
		/// </summary>
		/// <param name="serverName">Server name (or address) to connect to.</param>
		/// <param name="serverPort">Server port.</param>
		/// <param name="playerName">Player name to be used in network game.</param>
		void Connect(std::string serverName, unsigned short serverPort, std::string playerName);

		/// <summary>
		/// Connects the client to a server through a NAT service.
		/// </summary>
		/// <param name="address">The NAT service address.</param>
		void ConnectNAT(RakNet::SystemAddress address);

		/// <summary>
		/// Disconnects the client from the currently connected server.
		/// </summary>
		void Disconnect();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="serviceServerName"></param>
		/// <param name="serverPort"></param>
		/// <param name="playerName"></param>
		/// <param name="serverName"></param>
		/// <param name="serverPassword"></param>
		void PerformNATPunchThrough(std::string serviceServerName, unsigned short serverPort, std::string playerName, std::string serverName, std::string serverPassword);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="rakPeer"></param>
		/// <param name="address"></param>
		/// <param name="port"></param>
		/// <returns></returns>
		RakNet::SystemAddress ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, unsigned short port);
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this object.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		static constexpr unsigned short c_PlayerNameCharLimit = 15; //!< Maximum length of the player name.
		std::string m_PlayerName; //!< The player name the will be used by the client in network games.

		RakNet::RakPeerInterface *m_Client; //!<
		RakNet::SystemAddress m_ClientID; //!<

		RakNet::SystemAddress m_ServerID; //!<
		RakNet::RakNetGUID m_ServerGUID; //!< The server's Globally Unique Identifier.

		RakNet::NatPunchthroughClient m_NatPunchthroughClient; //!<
		RakNet::SystemAddress m_NATServiceServerID; //!<

		bool m_UseNATPunchThroughService; //!<

		bool m_IsConnected; //!<
		bool m_IsRegistered; //!<
		bool m_IsNATPunched; //!<

		unsigned char m_PixelLineBuffer[c_MaxPixelLineBufferSize]; //!<

		long int m_ReceivedData; //!<
		long int m_CompressedData; //!<

		int m_ClientInputFps; //!<
		long long m_LastInputSentTime; //!<

		int m_CurrentFrame; //!<

		Vector m_TargetPos[c_FramesToRemember]; //!<
		std::list<PostEffect> m_PostEffects[c_FramesToRemember]; //!<

		std::unordered_map<unsigned short, SoundContainer *> m_ServerSounds; //!< Unordered map of SoundContainers received from server. OWNED!!!

		unsigned char m_SceneID; //!<
		int m_CurrentSceneLayerReceived; //!<

		BITMAP *m_SceneBackgroundBitmap; //!<
		BITMAP *m_SceneForegroundBitmap; //!<

		BITMAP *m_BackgroundBitmaps[c_MaxBackgroundLayersTransmitted]; //!<
		LightweightSceneLayer m_BackgroundLayers[c_FramesToRemember][c_MaxBackgroundLayersTransmitted]; //!<
		int m_ActiveBackgroundLayers; //!<
		bool m_SceneWrapsX; //!<
		int m_SceneWidth; //!<
		int m_SceneHeight; //!<

		int m_MouseButtonPressedState[3]; //!<
		int m_MouseButtonReleasedState[3]; //!<

	private:

#pragma region Update Breakdown
		/// <summary>
		/// 
		/// </summary>
		void HandleNetworkPackets();
#pragma endregion

#pragma region Network Event Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		/// <returns></returns>
		unsigned char GetPacketIdentifier(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		void SendRegisterMsg();

		/// <summary>
		/// 
		/// </summary>
		void ReceiveAcceptedMsg();

		/// <summary>
		/// 
		/// </summary>
		void SendDisconnectMsg();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="addr"></param>
		/// <param name="serverName"></param>
		/// <param name="serverPassword"></param>
		void SendServerGUIDRequest(RakNet::SystemAddress addr, std::string serverName, std::string serverPassword);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveServerGUIDAnswer(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		void SendInputMsg();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveFrameSetupMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveFrameLineMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveFrameBoxMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		void SendSceneAcceptedMsg();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveSceneMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		void ReceiveSceneEndMsg();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveSceneSetupMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		void SendSceneSetupAcceptedMsg();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveTerrainChangeMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceivePostEffectsMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveSoundEventsMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveMusicEventsMsg(RakNet::Packet *packet);
#pragma endregion

#pragma region Drawing
		/// <summary>
		/// 
		/// </summary>
		/// <param name="targetBitmap"></param>
		void DrawBackgrounds(BITMAP *targetBitmap);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="frame"></param>
		void DrawPostEffects(int frame);

		/// <summary>
		/// 
		/// </summary>
		void DrawFrame();
#pragma endregion

		/// <summary>
		/// Gets the ping time between the client and the server.
		/// </summary>
		/// <returns>The ping time between the client and the server.</returns>
		unsigned short GetPing() const { return IsConnectedAndRegistred() ? m_Client->GetLastPing(m_ServerID) : 0; }

		/// <summary>
		/// Clears all the member variables of this NetworkClient, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		NetworkClient(const NetworkClient &reference) {}
		NetworkClient & operator=(const NetworkClient &rhs) {}
	};
}
#endif