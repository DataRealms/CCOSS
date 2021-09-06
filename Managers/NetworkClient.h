#ifndef _RTENETWORKCLIENT_
#define _RTENETWORKCLIENT_

#include "Singleton.h"
#include "SoundContainer.h"

#include "NetworkMessages.h"

// TODO: Figure out how to deal with anything that is defined by these and include them in implementation only to remove Windows.h macro pollution from our headers.
#include "RakPeerInterface.h"
#include "NatPunchthroughClient.h"

// RakNet includes Windows.h so we need to undefine macros that conflict with our method names.
#undef GetClassName

#define g_NetworkClient NetworkClient::Instance()

/////////////////////////////////////////////////////////////////////////
// TODO: Get Weegee to comment all these because I don't even.
/////////////////////////////////////////////////////////////////////////

namespace RTE {

	struct PostEffect;

	/// <summary>
	/// The centralized singleton manager of the network multiplayer client.
	/// </summary>
	class NetworkClient : public Singleton<NetworkClient> {
		friend class SettingsMan;

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a NetworkClient object in system memory. Create() should be called before using the object.
		/// </summary>
		NetworkClient() { Clear(); Initialize(); }

		/// <summary>
		/// Makes the NetworkClient object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a NetworkClient object before deletion from system memory.
		/// </summary>
		~NetworkClient() { Destroy(); }

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
		bool IsConnectedAndRegistered() const { return m_IsConnected && m_IsRegistered; }
		
		/// <summary>
		/// Gets scene width for network client.
		/// </summary>
		/// <returns>Current scene width.</returns>
		int GetSceneWidth() const { return m_SceneWidth; }

		/// <summary>
		/// Gets scene height for network client.
		/// </summary>
		/// <returns>Current scene height.</returns>
		int GetSceneHeight() const { return m_SceneHeight; }
		
		/// <summary>
		/// Indicates whether the scene wraps its scrolling around the X axis for network client.
		/// </summary>
		/// <returns>Whether the scene wraps around the X axis or not.</returns>
		bool SceneWrapsX() const { return m_SceneWrapsX; }

		/// <summary>
		/// Get the coordinates of the centre of the current frame.
		/// </summary>
		/// <returns>A vector containing the X/Y coordinates of the frame target.</returns>
		const Vector & GetFrameTarget()const { return m_TargetPos[m_CurrentFrame]; }

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
		/// Connects to a NAT service and performs punch-through.
		/// </summary>
		/// <param name="serviceServerName">NAT service server name (or address) to use for punch-through.</param>
		/// <param name="serviceServerPort">NAT service server port.</param>
		/// <param name="playerName">Player name to be used in network game.</param>
		/// <param name="serverName">Server name (or address) to connect to.</param>
		/// <param name="serverPassword">Server password.</param>
		void PerformNATPunchThrough(std::string serviceServerName, unsigned short serviceServerPort, std::string playerName, std::string serverName, std::string serverPassword);

		// TODO: Figure out
		/// <summary>
		/// 
		/// </summary>
		/// <param name="rakPeer"></param>
		/// <param name="address"></param>
		/// <param name="port"></param>
		/// <returns></returns>
		RakNet::SystemAddress ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, unsigned short port);
#pragma endregion

	protected:

		static constexpr unsigned short c_PlayerNameCharLimit = 15; //!< Maximum length of the player name.
		std::string m_PlayerName; //!< The player name the will be used by the client in network games.

		RakNet::RakPeerInterface *m_Client; //!< The client RakPeerInterface.
		RakNet::SystemAddress m_ClientID; //!< The client's identifier.

		RakNet::SystemAddress m_ServerID; //!< The server's identifier.
		RakNet::RakNetGUID m_ServerGUID; //!< The server's Globally Unique Identifier.

		RakNet::NatPunchthroughClient m_NATPunchthroughClient; //!< The NAT punch-through client.
		RakNet::SystemAddress m_NATServiceServerID; //!< The NAT server's identifier.

		bool m_UseNATPunchThroughService; //!< Whether to use NAT service for connecting to server.

		bool m_IsConnected; //!< Is client connected to server.
		bool m_IsRegistered; //!< Is client registered at server.
		bool m_IsNATPunched; //!< Is client connected through NAT service.

		unsigned char m_PixelLineBuffer[c_MaxPixelLineBufferSize]; //!<

		long int m_ReceivedData; //!<
		long int m_CompressedData; //!<

		int m_ClientInputFps; //!< The rate (in FPS) the client input is sent to the server.
		long long m_LastInputSentTime; //!< The last time input was sent in real time ticks.

		int m_CurrentFrame; //!<

		Vector m_TargetPos[c_FramesToRemember]; //!<
		std::list<PostEffect> m_PostEffects[c_FramesToRemember]; //!< List of post-effects received from server.

		std::unordered_map<int, SoundContainer *> m_ServerSounds; //!< Unordered map of SoundContainers received from server. OWNED!!!

		unsigned char m_SceneID; //!< 
		int m_CurrentSceneLayerReceived; //!<

		BITMAP *m_SceneBackgroundBitmap; //!<
		BITMAP *m_SceneForegroundBitmap; //!<

		BITMAP *m_BackgroundBitmaps[c_MaxLayersStoredForNetwork]; //!<
		LightweightSceneLayer m_BackgroundLayers[c_FramesToRemember][c_MaxLayersStoredForNetwork]; //!<
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
		unsigned char GetPacketIdentifier(RakNet::Packet *packet) const;

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
		/// <param name="address"></param>
		/// <param name="serverName"></param>
		/// <param name="serverPassword"></param>
		void SendServerGUIDRequest(RakNet::SystemAddress address, std::string serverName, std::string serverPassword);

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
		/// Receive and handle a packet of terrain change data.
		/// </summary>
		/// <param name="packet">The packet to handle.</param>
		void ReceiveTerrainChangeMsg(RakNet::Packet *packet);

		/// <summary>
		/// Receive and handle a packet of post-effect data. 
		/// </summary>
		/// <param name="packet">The packet to handle.</param>
		void ReceivePostEffectsMsg(RakNet::Packet *packet);

		/// <summary>
		/// Receive and handle a packet of sound event data.
		/// </summary>
		/// <param name="packet">The packet to handle.</param>
		void ReceiveSoundEventsMsg(RakNet::Packet *packet);

		/// <summary>
		/// Receive and handle a packet of music event data.
		/// </summary>
		/// <param name="packet">The packet to handle.</param>
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
		unsigned short GetPing() const { return IsConnectedAndRegistered() ? m_Client->GetLastPing(m_ServerID) : 0; }

		/// <summary>
		/// Clears all the member variables of this NetworkClient, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		NetworkClient(const NetworkClient &reference) = delete;
		NetworkClient & operator=(const NetworkClient &rhs) = delete;
	};
}
#endif
