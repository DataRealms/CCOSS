#ifndef _RTENETWORKCLIENT_
#define _RTENETWORKCLIENT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            NrtworkClient.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the NetworkClient class.
// Project:         Retro Terrain Engine
// Author(s):       

// Without this nested includes somewhere deep inside Allegro will summon winsock.h and it will conflict with winsock2.h from RakNet
// and we can't move "Network.h" here because for whatever reasons everything will collapse
#define WIN32_LEAN_AND_MEAN

#include <string.h>
#include "Singleton.h"
#include "UInputMan.h"
#include "Sound.h"

#include <map>

#include "Network.h"
#include "NatPunchthroughClient.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#define g_NetworkClient NetworkClient::Instance()

namespace RTE
{
	//////////////////////////////////////////////////////////////////////////////////////////
	// Class:           NetworkClient
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     The centralized singleton manager of all Timer:s and overall
	//                  timekeeping in RTE.
	// Parent(s):       Singleton
	// Class history:   

	class NetworkClient :
		public Singleton<NetworkClient>//,
	//    public Serializable
	{
		//////////////////////////////////////////////////////////////////////////////////////////
		// Public member variable, method and friend function declarations

	public:


		//////////////////////////////////////////////////////////////////////////////////////////
		// Constructor:     NetworkClient
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Constructor method used to instantiate a NetworkClient object in system
		//                  memory. Create() should be called before using the object.
		// Arguments:       None.

		NetworkClient() { Clear(); Create(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Destructor:      ~NetworkClient
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destructor method used to clean up a NetworkClient object before deletion
		//                  from system memory.
		// Arguments:       None.

		virtual ~NetworkClient() { Destroy(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Makes the NetworkClient object ready for use.
		// Arguments:       None.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		virtual int Create();


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Reset
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Resets the entire NetworkClient, including its inherited members, to
		//                  their default settings or values.
		// Arguments:       None.
		// Return value:    None.

		virtual void Reset() { Clear(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Destroy
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destroys and resets (through Clear()) the NetworkClient object.
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


		void Update();

		void Connect(std::string serverName, unsigned short serverPort, std::string playerName);

		void ConnectNAT(RakNet::SystemAddress addr);

		void PerformNATPunchThrough(std::string serviceServerName, unsigned short serverPort, std::string playerName, std::string serverName, std::string serverPassword);

		RakNet::SystemAddress ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *address, unsigned short int port);

		void Disconnect();

		bool IsConnectedAndRegistred() { return m_IsConnected && m_IsRegistered; }

		struct MsgInput
		{
			unsigned char Id;

			int MouseX;
			int MouseY;
			bool MouseButtonPressed[UInputMan::MAX_MOUSE_BUTTONS];
			bool MouseButtonReleased[UInputMan::MAX_MOUSE_BUTTONS];
			bool MouseButtonHeld[UInputMan::MAX_MOUSE_BUTTONS];
			bool ResetActivityVote;

			int MouseWheelMoved;

			unsigned int InputElementPressed;
			unsigned int InputElementReleased;
			unsigned int InputElementHeld;

		};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Protected member variable and method declarations

	protected:


		// Member variables
		static const std::string m_ClassName;

		RakNet::SystemAddress m_ClientID;

		RakNet::RakPeerInterface *m_Client;

		RakNet::NatPunchthroughClient m_NatPunchthroughClient;

		RakNet::SystemAddress m_NATServiceServerID;

		RakNet::SystemAddress m_ServerID;

		RakNet::RakNetGUID m_ServerGuid;

		std::string m_PlayerName;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Private member variable and method declarations

	private:

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Clear
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Clears all the member variables of this NetworkClient, effectively
		//                  resetting the members of this abstraction level only.
		// Arguments:       None.
		// Return value:    None.

		void Clear();

		void SendRegisterMsg();

		void SendDisconnectMsg();

		void SendInputMsg();

		void ReceiveFrameSetupMsg(RakNet::Packet * p);

		void ReceiveFrameLineMsg(RakNet::Packet * p);

		void ReceiveFrameBoxMsg(RakNet::Packet * p);

		void ReceiveSceneMsg(RakNet::Packet * p);

		void ReceiveAcceptedMsg();

		void ReceiveSceneSetupMsg(RakNet::Packet * p);

		void SendSceneSetupAcceptedMsg();

		void ReceiveTerrainChangeMsg(RakNet::Packet * p);

		void SendSceneAcceptedMsg();

		void ReceiveSceneEndMsg();

		void DrawBackgrounds(BITMAP * pTargetBitmap);

		void DrawFrame();

		void SendServerGuidRequest(RakNet::SystemAddress addr, std::string serverName, std::string serverPassword);

		void ReceiveServerGiudAnswer(RakNet::Packet * p);

		void ReceivePosteEffectsMsg(RakNet::Packet * p);

		void ReceiveSoundEventsMsg(RakNet::Packet * p);

		void ReceiveMusicEventsMsg(RakNet::Packet * p);

		void DrawPostEffects(int frame);

		unsigned int GetPing();


		int64_t m_LastInputSentTime;

		unsigned char m_aPixelLineBuffer[MAX_PIXEL_LINE_BUFFER_SIZE];

		long int m_ReceivedData;

		long int m_CompressedData;

		bool m_IsConnected;

		bool m_IsRegistered;

		bool m_IsNATPunched;

		int m_ClientInputFps;

		//int m_LastLineReceived;

		int m_CurrentSceneLayerReceived;

		unsigned char m_SceneId;

		int m_CurrentFrame;

		Vector m_TargetPos[FRAMES_TO_REMEMBER];
		std::list<PostEffect> m_PostEffects[FRAMES_TO_REMEMBER];

		// List of sounds received from server. OWNED!!!
		std::map<short int, Sound *> m_Sounds;

		BITMAP * m_pSceneBackgroundBitmap;
		BITMAP * m_pSceneForegroundBitmap;

		BITMAP * m_BackgroundBitmaps[MAX_BACKGROUND_LAYERS_TRANSMITTED];
		LightweightSceneLayer m_aBackgroundLayers[FRAMES_TO_REMEMBER][MAX_BACKGROUND_LAYERS_TRANSMITTED];
		int m_ActiveBackgroundLayers;
		bool m_SceneWrapsX;
		int m_SceneWidth;
		int m_SceneHeight;

		int m_aMouseButtonPressedState[3];
		int m_aMouseButtonReleasedState[3];

		bool m_UseNATPunchThroughService;

		// Disallow the use of some implicit methods.
		NetworkClient(const NetworkClient &reference);
		NetworkClient & operator=(const NetworkClient &rhs);

		unsigned char GetPacketIdentifier(RakNet::Packet *p);
	};

} // namespace RTE

#endif // File
