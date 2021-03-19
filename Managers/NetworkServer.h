#ifndef _RTENETWORKSERVER_
#define _RTENETWORKSERVER_

#include "Singleton.h"
#include "SceneMan.h"

#include "NetworkClient.h"

#define g_NetworkServer NetworkServer::Instance()

/////////////////////////////////////////////////////////////////////////
// TODO: Get Weegee to comment all these because I don't even.
/////////////////////////////////////////////////////////////////////////

namespace RTE {

	/// <summary>
	/// The centralized singleton manager of the network multiplayer server.
	/// </summary>
	class NetworkServer : public Singleton<NetworkServer> {
		friend class SettingsMan;

	public:

		/// <summary>
		/// 
		/// </summary>
		enum NetworkServerStats {
			STAT_CURRENT = 0,
			STAT_SHOWN,
			MAX_STAT_RECORDS = 5
		};

		/// <summary>
		/// 
		/// </summary>
		enum ThreadExitReasons {
			NORMAL = 0,
			THREAD_FINISH,
			TOO_EARLY_TO_SEND,
			SEND_BUFFER_IS_FULL,
			SEND_BUFFER_IS_LIMITED_BY_CONGESTION,
			LOCKED
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a NetworkServer object in system memory. This will call Create() so it shouldn't be called after.
		/// </summary>
		NetworkServer() { Clear(); Initialize(); }

		/// <summary>
		/// Makes the NetworkServer object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a NetworkServer object before deletion from system memory.
		/// </summary>
		~NetworkServer() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the NetworkServer object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets whether server mode is enabled or not.
		/// </summary>
		/// <returns>Whether server mode is enabled or not.</returns>
		bool IsServerModeEnabled() const { return m_IsInServerMode; }

		/// <summary>
		/// Enables server mode.
		/// </summary>
		void EnableServerMode() { m_IsInServerMode = true; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool ReadyForSimulation();

		/// <summary>
		/// Gets the network player's name.
		/// </summary>
		/// <param name="player">The player to check for.</param>
		/// <returns>A string with the network player's name.</returns>
		std::string & GetPlayerName(short player) { return m_ClientConnections[player].PlayerName; }

		/// <summary>
		/// Gets whether the specified player is connected to the server or not.
		/// </summary>
		/// <param name="player">The player to check for.</param>
		/// <returns>Whether the player is connected to the server or not.</returns>
		bool IsPlayerConnected(short player) const { return m_ClientConnections[player].IsActive; }

		/// <summary>
		/// Sets the port this server will be using.
		/// </summary>
		/// <param name="newPort">The new port to set.</param>
		void SetServerPort(const std::string &newPort);

		/// <summary>
		/// Sets whether interlacing is used to reduce bandwidth usage or not.
		/// </summary>
		/// <param name="newMode">Whether to use interlacing or not.</param>
		void SetInterlacingMode(bool newMode) { m_UseInterlacing = newMode; }

		/// <summary>
		/// Sets the duration this thread should be put to sleep for in milliseconds.
		/// </summary>
		/// <param name="player">The player to set for.</param>
		/// <param name="msecs">Milliseconds to sleep for.</param>
		void SetMSecsToSleep(short player, int msecs) { m_MSecsToSleep[player] = msecs; };

		/// <summary>
		/// Gets the ping time of the specified player.
		/// </summary>
		/// <param name="player">The player to get for.</param>
		/// <returns>The ping time of the player.</returns>
		unsigned short GetPing(short player) const { return m_Ping[player]; }

		/// <summary>
		/// Gets whether server puts threads to sleep if it didn't receive anything for 10 seconds to reduce CPU load.
		/// </summary>
		/// <returns>Whether threads will be put to sleep when server isn't receiving any data or not.</returns>
		bool GetServerSleepWhenIdle() const { return m_SleepWhenIdle; }

		/// <summary>
		/// Gets whether the server will try to put the thread to sleep to reduce CPU load if the sim frame took less time to complete than it should at 30 fps.
		/// </summary>
		/// <returns>Whether threads will be put to sleep if server completed frame faster than it normally should or not.</returns>
		bool GetServerSimSleepWhenIdle() const { return m_SimSleepWhenIdle; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Start server, open ports etc.
		/// </summary>
		void Start();

		/// <summary>
		/// Updates the state of this NetworkServer. Supposed to be done every frame before drawing.
		/// </summary>
		/// <param name="processInput">Whether to process packets of player input data or not.</param>
		void Update(bool processInput = false);
#pragma endregion

#pragma region Network Scene Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="isLocked"></param>
		void LockScene(bool isLocked);

		/// <summary>
		/// 
		/// </summary>
		void ResetScene();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="terrainChange"></param>
		void RegisterTerrainChange(SceneMan::TerrainChange terrainChange);
#pragma endregion

	protected:

		/// <summary>
		/// 
		/// </summary>
		struct ClientConnection {
			bool IsActive; //!<
			RakNet::SystemAddress ClientId; //!<
			RakNet::SystemAddress InternalId; //!<
			int ResX; //!<
			int ResY; //!<
			std::thread *SendThread; //!<
			std::string PlayerName; //!<
		};

		bool m_IsInServerMode = false; //!<

		bool m_SleepWhenIdle; //!< If true puts thread to sleep if it didn't receive anything for 10 seconds to avoid melting the CPU at 100% even if there are no connections.
		bool m_SimSleepWhenIdle; //!< If true the server will try to put the thread to sleep to reduce CPU load if the sim frame took less time to complete than it should at 30 fps.

		int m_ThreadExitReason[c_MaxClients]; //!<

		long m_MSecsSinceLastUpdate[c_MaxClients]; //!<
		long m_MSecsToSleep[c_MaxClients]; //!<

		RakNet::RakPeerInterface *m_Server; //!<

		std::string m_ServerPort; //!<

		ClientConnection m_ClientConnections[c_MaxClients]; //!<

		bool m_UseNATService; //!< Whether a NAT service is used for punch-through.
		RakNet::NatPunchthroughClient m_NATPunchthroughClient; //!<
		RakNet::SystemAddress m_NATServiceServerID; //!<
		bool m_NatServerConnected; //!<

		unsigned char m_PixelLineBuffer[c_MaxClients][c_MaxPixelLineBufferSize]; //!<

		BITMAP *m_BackBuffer8[c_MaxClients]; //!<
		BITMAP *m_BackBufferGUI8[c_MaxClients]; //!<

		void *m_LZ4CompressionState[c_MaxClients]; //!<
		void *m_LZ4FastCompressionState[c_MaxClients]; //!<

		int m_MouseState1[c_MaxClients]; //!<
		int m_MouseState2[c_MaxClients]; //!<
		int m_MouseState3[c_MaxClients]; //!<

		int m_MouseEvent1[c_MaxClients]; //!<
		int m_MouseEvent2[c_MaxClients]; //!<
		int m_MouseEvent3[c_MaxClients]; //!<

		bool m_UseHighCompression; //!< Whether to use higher compression methods (default).
		bool m_UseFastCompression; //!< Whether to use faster compression methods and conserve CPU.
		int m_HighCompressionLevel; //!< Compression level. 10 is optimal, 12 is highest.

		/// <summary>
		/// Acceleration factor, higher values consume more bandwidth but less CPU.
		/// The larger the acceleration value, the faster the algorithm, but also lesser the compression. It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed. 
		/// An acceleration value of "1" is the same as regular LZ4_compress_default(). Values <= 0 will be replaced by ACCELERATION_DEFAULT(currently == 1, see lz4 documentation).
		/// </summary>
		int m_FastAccelerationFactor;

		bool m_UseInterlacing; //!< Use interlacing to heavily reduce bandwidth usage at the cost of visual degradation (unusable at 30 fps, but may be suitable at 60 fps).
		int m_EncodingFps; //!< Frame transmission rate. Higher value equals more CPU and bandwidth consumption.

		bool m_SendEven[c_MaxClients]; //!<

		bool m_ShowStats; //!<
		bool m_ShowInput; //!<

		bool m_SendSceneSetupData[c_MaxClients]; //!<
		bool m_SendSceneData[c_MaxClients]; //!<
		bool m_SceneAvailable[c_MaxClients]; //!<
		bool m_SendFrameData[c_MaxClients]; //!<
		std::mutex m_SceneLock[c_MaxClients]; //!<

		unsigned char m_TerrainChangeBuffer[c_MaxClients][c_MaxPixelLineBufferSize]; //!<
		std::queue<SceneMan::TerrainChange> m_PendingTerrainChanges[c_MaxClients]; //!<
		std::queue<SceneMan::TerrainChange> m_CurrentTerrainChanges[c_MaxClients]; //!<

		std::mutex m_Mutex[c_MaxClients]; //!<

		//std::mutex m_InputQueueMutex[c_MaxClients];
		std::queue<MsgInput> m_InputMessages[c_MaxClients]; //!<

		unsigned char m_SceneID; //!<

		bool m_ResetActivityVotes[c_MaxClients]; //!<

		int m_FrameNumbers[c_MaxClients]; //!<

		unsigned short m_Ping[c_MaxClients]; //!< 
		Timer m_PingTimer[c_MaxClients]; //!<

		Timer m_LastPackedReceived; //!<

		/// <summary>
		/// Transmit frames as blocks instead of lines. Provides better compression at the cost of higher CPU usage.
		/// Though the compression is quite high it is recommended that Width * Height are less than MTU size or about 1500 bytes or packets may be fragmented by network hardware or dropped completely.
		/// </summary>
		bool m_TransmitAsBoxes;
		int m_BoxWidth; //!< Width of the transmitted CPU block. Different values may improve bandwidth usage.
		int m_BoxHeight; //!< Height of the transmitted CPU block. Different values may improve bandwidth usage.

		int m_EmptyBlocks[MAX_STAT_RECORDS]; //!<
		int m_FullBlocks[MAX_STAT_RECORDS]; //!<
		int m_SendBufferBytes[MAX_STAT_RECORDS]; //!<
		int m_SendBufferMessages[MAX_STAT_RECORDS]; //!<
		int m_DelayedFrames[c_MaxClients]; //!<
		int m_MsecPerFrame[c_MaxClients]; //!<
		int m_MsecPerSendCall[c_MaxClients]; //!<

		long long m_LastFrameSentTime[c_MaxClients]; //!<
		long long m_LastStatResetTime[c_MaxClients]; //!<

		unsigned int m_FramesSent[MAX_STAT_RECORDS]; //!< Number of frames sent by the server to each client and total.
		unsigned int m_FramesSkipped[MAX_STAT_RECORDS]; //!< Numbers of frames skipped by the server for each client and total.

		unsigned long m_DataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_DataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long m_DataUncompressedTotal[MAX_STAT_RECORDS]; //!<
		unsigned long m_DataUncompressedCurrent[MAX_STAT_RECORDS][2]; //!<

		unsigned long m_FrameDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_FrameDataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long  m_PostEffectDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long  m_PostEffectDataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long  m_SoundDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long  m_SoundDataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long  m_TerrainDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long  m_TerrainDataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long m_OtherDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_OtherDataSentTotal[MAX_STAT_RECORDS]; //!<

	private:

#pragma region Thread Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="server"></param>
		/// <param name="player"></param>
		static void BackgroundSendThreadFunction(NetworkServer *server, short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <param name="reason"></param>
		void SetThreadExitReason(short player, int reason) { m_ThreadExitReason[player] = reason; };
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
		/// <param name="packet"></param>
		void ReceiveNewIncomingConnection(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void SendAcceptedMsg(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveDisconnection(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveRegisterMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="addr"></param>
		void SendNATServerRegistrationMsg(RakNet::SystemAddress address);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveInputMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <param name="msg"></param>
		void ProcessInputMsg(short player, MsgInput msg);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void ClearInputMessages(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void SendSoundData(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void SendMusicData(short player);
#pragma endregion

#pragma region Network Scene Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		bool IsSceneAvailable(short player) const { return m_SceneAvailable[player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		bool NeedToSendSceneSetupData(short player) const { return m_SendSceneSetupData[player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void SendSceneSetupData(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveSceneSetupDataAccepted(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		bool NeedToSendSceneData(short player) const { return m_SendSceneData[player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void SendSceneData(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void ClearTerrainChangeQueue(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		bool NeedToProcessTerrainChanges(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void ProcessTerrainChanges(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <param name="terrainChange"></param>
		void SendTerrainChangeMsg(short player, SceneMan::TerrainChange terrainChange);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="packet"></param>
		void ReceiveSceneAcceptedMsg(RakNet::Packet *packet);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void SendSceneEndMsg(short player);
#pragma endregion

#pragma region Network Frame Handling and Drawing
		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <param name="w"></param>
		/// <param name="h"></param>
		void CreateBackBuffer(short player, int w, int h);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void DestroyBackBuffer(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void SendFrameSetupMsg(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		bool SendFrameData(short player) const { return m_SendFrameData[player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void SendPostEffectData(short player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		int SendFrame(short player);
#pragma endregion

#pragma region Network Stats Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void UpdateStats(short player);

		/// <summary>
		/// 
		/// </summary>
		void DrawStatisticsData();
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// 
		/// </summary>
		void HandleNetworkPackets();
#pragma endregion

		/// <summary>
		/// Gets the Globally Unique Identifier of the server.
		/// </summary>
		/// <returns>The GUID of the server.</returns>
		RakNet::RakNetGUID GetServerGUID() const { return m_Server->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS); }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="rakPeer"></param>
		/// <param name="address"></param>
		/// <param name="port"></param>
		/// <returns></returns>
		RakNet::SystemAddress ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, unsigned short port);

		/// <summary>
		/// Clears all the member variables of this NetworkServer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		NetworkServer(const NetworkServer &reference) = delete;
		NetworkServer & operator=(const NetworkServer &rhs) = delete;
	};
}
#endif
