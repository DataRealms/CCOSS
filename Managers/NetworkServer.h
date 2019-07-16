#ifndef _RTENETWORKSERVER_
#define _RTENETWORKSERVER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            NrtworkClient.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the NetworkServer class.
// Project:         Retro Terrain Engine
// Author(s):       

// Without this nested includes somewhere deep inside Allegro will summon winsock.h and it will conflict with winsock2.h from RakNet
// and we can't move "Network.h" here because for whatever reasons everything will collapse
#define WIN32_LEAN_AND_MEAN

#include <string.h>
#include "Singleton.h"
#include "UInputMan.h"

#include "Network.h"
#include "NetworkClient.h"
#include "NatPunchthroughClient.h"

#include <thread>
#include <mutex>

#include "TimerMan.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#define STATS_SUM MAX_CLIENTS
#define MAX_STAT_RECORDS MAX_CLIENTS + 1

#define STAT_CURRENT 0
#define STAT_SHOWN 1

#define g_NetworkServer NetworkServer::Instance()

namespace RTE
{
	//////////////////////////////////////////////////////////////////////////////////////////
	// Class:           NetworkServer
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     The centralized singleton manager of all Timer:s and overall
	//                  timekeeping in RTE.
	// Parent(s):       Singleton
	// Class history:   

	class NetworkServer :
		public Singleton<NetworkServer>//,
	//    public Serializable
	{
		//////////////////////////////////////////////////////////////////////////////////////////
		// Public member variable, method and friend function declarations

	public:

		enum TreadExitReasons
		{
			NORMAL = 0,
			THREAD_FINISH,
			TOO_EARLY_TO_SEND,
			SEND_BUFFER_IS_FULL,
			SEND_BUFFER_IS_LIMITED_BY_CONGESTION,
			LOCKED
		};


		//////////////////////////////////////////////////////////////////////////////////////////
		// Constructor:     NetworkServer
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Constructor method used to instantiate a NetworkServer object in system
		//                  memory. Create() should be called before using the object.
		// Arguments:       None.

		NetworkServer() { Clear(); Create(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Destructor:      ~NetworkServer
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destructor method used to clean up a NetworkServer object before deletion
		//                  from system memory.
		// Arguments:       None.

		virtual ~NetworkServer() { Destroy(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Makes the NetworkServer object ready for use.
		// Arguments:       None.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		virtual int Create();


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Reset
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Resets the entire NetworkServer, including its inherited members, to
		//                  their default settings or values.
		// Arguments:       None.
		// Return value:    None.

		virtual void Reset() { Clear(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Destroy
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destroys and resets (through Clear()) the NetworkServer object.
		// Arguments:       None.
		// Return value:    None.

		void Destroy();


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  GetClassName
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets the class name of this Entity.
		// Arguments:       None.
		// Return value:    A string with the friendly-formatted type name of this object.

		virtual const std::string & GetClassName() const { return m_ClassName; }




		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Start
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Start server, open ports etc.
		// Arguments:       None.
		// Return value:    None.

		void Start();

		void Update(bool processInput = false);

		void EnableServerMode() { m_IsInServerMode = true; }

		bool IsServerModeEnabled() { return m_IsInServerMode; }

		bool IsPlayerConnected(int player) { return m_ClientConnections[player].IsActive; }

		void SetServerPort(std::string newPort) { m_ServerPort = newPort; }

		void ReceiveNewIncomingConnection(RakNet::Packet * p);

		void ReceiveDisconnection(RakNet::Packet * p);

		void ReceiveRegisterMsg(RakNet::Packet * p);

		void ReceiveInputMsg(RakNet::Packet * p);

		void SendAcceptedMsg(int player);

		int SendFrame(int player);

		void CreateBackBuffer(int player, int w, int h);

		void DestroyBackBuffer(int player);

		std::string & GetPlayerName(int player);

		void SetThreadExitReason(int player, int reason) { m_ThreadExitReason[player] = reason; };

		void SetMSecsToSleep(int player, int msecs) { m_MSecsToSleep[player] = msecs; };

		void SendSceneSetupData(int player);

		void ReceiveSceneSetupDataAccepted(RakNet::Packet * p);

		bool NeedToSendSceneSetupData(int player) { return m_SendSceneSetupData[player]; }

		bool NeedToSendSceneData(int player) { return m_SendSceneData[player]; }

		bool IsSceneAvailable(int player) { return m_SceneAvailable[player]; }

		void SendFrameSetupMsg(int player);

		bool SendFrameData(int player) { return m_SendFrameData[player]; }

		void ResetScene();

		void SendSceneData(int player);

		void SendSceneEndMsg(int player);

		void ReceiveSceneAcceptedMsg(RakNet::Packet * p);

		void RegisterTerrainChange(SceneMan::TerrainChange tc);

		void NetworkServer::ClearTerrainChangeQueue(int player);

		bool NeedToProcessTerrainChanges(int player);

		void ProcessTerrainChanges(int player);

		void SendTerrainChangeMsg(int player, SceneMan::TerrainChange tc);

		bool ReadyForSimulation();

		void SetInterlacingMode(bool newMode) { m_UseInterlacing = newMode; }

		void SendNATServerRegistrationMsg(RakNet::SystemAddress addr);

		void ClearInputMessages(int player);

		void ProcessInputMessage(int player, NetworkClient::MsgInput msg);

		RakNet::RakNetGUID GetServerGuid();

		void DrawStatisticsData();

		RakNet::SystemAddress NetworkServer::ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, int port);

		void UpdateStats(int player);

		void SendPostEffectData(int player);

		void SendSoundData(int player);

		void SendMusicData(int player);

		void LockScene(bool isLocked);

		unsigned int GetPing(int player) const { return m_Ping[player]; }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Protected member variable and method declarations


	protected:

		struct ClientConnection
		{
			bool IsActive;
			RakNet::SystemAddress ClientId;
			RakNet::SystemAddress InternalId;
			int ResX;
			int ResY;
			std::thread * pSendThread;
			std::string PlayerName;
		};

		ClientConnection m_ClientConnections[MAX_CLIENTS];

		// Member variables
		static const std::string m_ClassName;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Private member variable and method declarations

		RakNet::RakPeerInterface *m_Server;

		RakNet::NatPunchthroughClient m_NatPunchthroughClient;


		std::string m_ServerPort;

		bool m_IsInServerMode = false;

		unsigned char m_aPixelLineBuffer[MAX_CLIENTS][MAX_PIXEL_LINE_BUFFER_SIZE];

		unsigned char m_aTerrainChangeBuffer[MAX_CLIENTS][MAX_PIXEL_LINE_BUFFER_SIZE];

		int64_t m_LastFrameSentTime[MAX_CLIENTS];

		int64_t m_LastStatResetTime[MAX_CLIENTS];

		unsigned int m_FramesSent[MAX_STAT_RECORDS];

		unsigned int m_FramesSkipped[MAX_STAT_RECORDS];

		unsigned long int m_DataSentCurrent[MAX_STAT_RECORDS][2];
		unsigned long int m_DataSentTotal[MAX_STAT_RECORDS];

		unsigned long int m_DataUncompressedTotal[MAX_STAT_RECORDS];
		unsigned long int m_DataUncompressedCurrent[MAX_STAT_RECORDS][2];

		unsigned long int m_FrameDataSentCurrent[MAX_STAT_RECORDS][2];
		unsigned long int m_FrameDataSentTotal[MAX_STAT_RECORDS];

		unsigned long int m_PostEffectDataSentCurrent[MAX_STAT_RECORDS][2];
		unsigned long int m_PostEffectDataSentTotal[MAX_STAT_RECORDS];

		unsigned long int m_SoundDataSentCurrent[MAX_STAT_RECORDS][2];
		unsigned long int m_SoundDataSentTotal[MAX_STAT_RECORDS];

		unsigned long int m_TerrainDataSentCurrent[MAX_STAT_RECORDS][2];
		unsigned long int m_TerrainDataSentTotal[MAX_STAT_RECORDS];

		unsigned long int m_OtherDataSentCurrent[MAX_STAT_RECORDS][2];
		unsigned long int m_OtherDataSentTotal[MAX_STAT_RECORDS];


		int m_EmptyBlocks[MAX_STAT_RECORDS];

		int m_FullBlocks[MAX_STAT_RECORDS];

		int m_SendBufferBytes[MAX_STAT_RECORDS];

		int m_SendBufferMessages[MAX_STAT_RECORDS];

		int m_DelayedFrames[MAX_CLIENTS];

		int m_MsecPerFrame[MAX_CLIENTS];

		int m_MsecPerSendCall[MAX_CLIENTS];

		BITMAP * m_pBackBuffer8[MAX_CLIENTS];

		BITMAP * m_pBackBufferGUI8[MAX_CLIENTS];

		void * m_pLZ4CompressionState[MAX_CLIENTS];

		void * m_pLZ4FastCompressionState[MAX_CLIENTS];

		const int m_MicroSecs = 1000000;

		int m_MouseState1[MAX_CLIENTS];
		int m_MouseState2[MAX_CLIENTS];
		int m_MouseState3[MAX_CLIENTS];

		int m_MouseEvent1[MAX_CLIENTS];
		int m_MouseEvent2[MAX_CLIENTS];
		int m_MouseEvent3[MAX_CLIENTS];


		bool m_UseHighCompression;

		bool m_UseFastCompression;

		int m_HighCompressionLevel;

		int m_FastAccelerationFactor;

		bool m_UseInterlacing;

		int m_EncodingFps;

		bool m_SendEven[MAX_CLIENTS];

		bool m_ShowStats;

		bool m_ShowInput;

		int m_ThreadExitReason[MAX_CLIENTS];

		long m_MSecsSinceLastUpdate[MAX_CLIENTS];
		long m_MSecsToSleep[MAX_CLIENTS];

		bool m_SendSceneSetupData[MAX_CLIENTS];
		bool m_SendSceneData[MAX_CLIENTS];
		bool m_SceneAvailable[MAX_CLIENTS];
		bool m_SendFrameData[MAX_CLIENTS];
		std::mutex m_SceneLock[MAX_CLIENTS];

		std::queue<SceneMan::TerrainChange> m_PendingTerrainChanges[MAX_CLIENTS];

		std::queue<SceneMan::TerrainChange> m_CurrentTerrainChanges[MAX_CLIENTS];

		std::mutex m_Mutex[MAX_CLIENTS];

		//std::mutex m_InputQueueMutex[MAX_CLIENTS];
		std::queue<NetworkClient::MsgInput>m_InputMessages[MAX_CLIENTS];

		float OffsetX[MAX_CLIENTS][MAX_BACKGROUND_LAYERS_TRANSMITTED];
		float OffsetY[MAX_CLIENTS][MAX_BACKGROUND_LAYERS_TRANSMITTED];

		unsigned char m_SceneId;

		bool m_ResetActivityVotes[MAX_CLIENTS];

		int m_FrameNumbers[MAX_CLIENTS];

		unsigned int m_Ping[MAX_CLIENTS];

		Timer m_PingTimer[MAX_CLIENTS];

		Timer m_LastPackedReceived;


		// Transmit frames divided into boxes instead of lines
		bool m_TransmitAsBoxes;
		int m_BoxWidth;
		int m_BoxHeight;

		bool m_NatServerConnected;

		RakNet::SystemAddress m_NATServiceServerID;


	private:

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Clear
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Clears all the member variables of this NetworkServer, effectively
		//                  resetting the members of this abstraction level only.
		// Arguments:       None.
		// Return value:    None.

		void Clear();

		// Disallow the use of some implicit methods.
		NetworkServer(const NetworkServer &reference);
		NetworkServer & operator=(const NetworkServer &rhs);

		unsigned char GetPacketIdentifier(RakNet::Packet *p);
	};

} // namespace RTE

#endif // File
