#include "pch.h"
#include <allegro.h>
#include <winalleg.h>
#include <winsock.h>
#include "DebugTool.h"
#include <DebugTool/DebugMessage.h>
#include <DebugTool/DebugMessageFactory.h>
#include <DebugTool/DebugMessageListener.h>
#include <BinaryReaderNormal.h>
#include <BinaryWriterNormal.h>
#ifdef _XBOX
//#include <xbdm.h>
#endif

#ifdef DEBUGTOOL_ENABLED

Debug::DebugTool* Debug::DebugTool::s_instance = 0;

static SOCKET					s_listenSocket;
static SOCKET					s_clientSocket;
static CRITICAL_SECTION		s_outgoingMessageCS;

bool Debug::DebugTool::Initialize()
{
	SlickAssert(s_instance == NULL);
	s_instance = new DebugTool;
	if (s_instance)
	{
		if (!s_instance->StartUp())
		{
			delete s_instance;
			s_instance = 0;
		}
	}
	return s_instance != NULL;
}



bool Debug::DebugTool::Finalize()
{
	SlickAssert(s_instance != NULL);
	if (s_instance)
	{
		s_instance->ShutDown();
	}
	delete s_instance;
	s_instance = NULL;
	return true;
}



Debug::DebugTool::DebugTool()
{
	s_listenSocket = INVALID_SOCKET;
	s_clientSocket = INVALID_SOCKET;
	InitializeCriticalSection(&s_outgoingMessageCS);
	m_outgoing.reserve(32);
}



Debug::DebugTool::~DebugTool()
{
	DeleteCriticalSection(&s_outgoingMessageCS);
}


bool Debug::DebugTool::StartUp()
{
#ifdef _XBOX
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);

	// Disable security on non RTM builds:
#ifdef SLICK_PROFILER
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
#endif

	if( XNetStartup( &xnsp ) != 0 )
	{
		return false;
	}
#endif

	//----------------------
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
		Printf("Error at WSAStartup()\n");
		return false;
	}

	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests.
	s_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s_listenSocket == INVALID_SOCKET) 
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(23481);

	if (bind( s_listenSocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) 
	{
		Printf("bind() failed.\n");
		closesocket(s_listenSocket);
		return false;
	}

	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the 
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled; 
	// If iMode != 0, non-blocking mode is enabled.
	u_long iMode = 1;
	ioctlsocket(s_listenSocket, FIONBIO, &iMode);

	//----------------------
	// Listen for incoming connection requests 
	// on the created socket
	if (listen( s_listenSocket, 1 ) == SOCKET_ERROR)
	{
		Printf("Error listening on socket.\n");
		return false;
	}

	return true;
}



bool Debug::DebugTool::ShutDown()
{
	WSACleanup();
#ifdef _XBOX
	XNetCleanup();
#endif
	return false;
}


bool    Debug::DebugTool::IsDebugToolConnected() const
{
    return s_clientSocket != INVALID_SOCKET;
}


void Debug::DebugTool::Update()
{
    // Accept a client socket
	if (s_clientSocket == INVALID_SOCKET)
	{
		// See if there is a client available:
		s_clientSocket = accept(s_listenSocket, NULL, NULL);
		if (s_clientSocket != INVALID_SOCKET) 
		{
			Printf("Debug Tool connected.");
			
			// Set to non blocking
			u_long iMode = 1;
			ioctlsocket(s_clientSocket, FIONBIO, &iMode);

			// Send the welcome message:
			unsigned int dbg = 'DEBG';
			send( s_clientSocket, (const char*)&dbg, 4, 0 );
		}
	}

	if (s_clientSocket != INVALID_SOCKET)
	{
		// Receive messages:
		ReceiveMessages();

		// Send messages:
		SendMessages();		
	}
}



void Debug::DebugTool::SendMessage( DebugMessage *message )
{
	EnterCriticalSection(&s_outgoingMessageCS);
	if (IsDebugToolConnected())
	{
		m_outgoing.push_back(message);
	}
	LeaveCriticalSection(&s_outgoingMessageCS);
}



void Debug::DebugTool::ReceiveMessages()
{
	unsigned int type = 0;
	int received = recv( s_clientSocket, (char*)&type, 4, 0);
	if (received == 0)
	{
		// Connection Lost
		s_clientSocket = INVALID_SOCKET;
	}
	if (received == 4)
	{
		// Put the socket in blocking mode until we've received the whole message:
		u_long iMode = 0;
		ioctlsocket(s_clientSocket, FIONBIO, &iMode);

		// Read the size of the data block:
		int size = 0;
		recv( s_clientSocket, (char*)&size, 4, 0);

		// Read the data:
		char *data = new char[size];
		recv( s_clientSocket, data, size, 0);
		
		// Return the socket to non blocking mode
		iMode = 1;
		ioctlsocket(s_clientSocket, FIONBIO, &iMode);

		// Construct the message:
		DebugMessage *msg = DebugMessageCreator::Instance().Create( type );
		BinaryReaderNormal reader( data, size );
		msg->Deserialize(reader);
		delete[] data;

		// Send the message to the message dispatcher
		HandleMessage(msg);
	}
}



void Debug::DebugTool::HandleMessage( DebugMessage *msg )
{
	MessageListenersContainer::const_iterator it = m_listeners.find(msg->GetType());

	if (it != m_listeners.end())
	{
		ListenerContainer::const_iterator i = it->second.begin();
		ListenerContainer::const_iterator e = it->second.end();

		for (; i!=e; ++i)
		{
			DebugMessageListener* listener = *i;
			listener->HandleDebugMessage(msg);
		}
	}	

	delete msg;
}



void Debug::DebugTool::SendMessages()
{
	EnterCriticalSection(&s_outgoingMessageCS);

	DebugMessageContainer::iterator i = m_outgoing.begin();
	DebugMessageContainer::iterator e = m_outgoing.end();
		
	// Set the socket to blocking mode
	u_long iMode = 0;
	ioctlsocket(s_clientSocket, FIONBIO, &iMode);

	for (;i!=e; ++i)
	{
		DebugMessage *msg = *i;

		BinaryWriterNormal writer;

		msg->Serialize(writer);

		int size = writer.GetDataSize();
		unsigned int type = msg->GetType();
		if (send( s_clientSocket, (const char *)&type, 4, 0 ) != 4)
		{
			Printf("Not sending all data!!");
		}

		if (send( s_clientSocket, (const char *)&size, 4, 0 ) != 4)
		{
			Printf("Not sending all data!!");
		}

		int blocksize = 1024;
		int bytesleft = size;

		while(bytesleft > 0)
		{
			int bytes_to_send = __min(blocksize, bytesleft);
			int bytessent = send( s_clientSocket, writer.GetData() + size - bytesleft, bytes_to_send, 0 );

			if (bytes_to_send != bytessent)
			{
				Printf("Not sending all data!!");
			}

			bytesleft -= blocksize;
		}

		delete msg; // temp
	}

	// Return the socket to non blocking mode
	iMode = 1;
	ioctlsocket(s_clientSocket, FIONBIO, &iMode);

	m_outgoing.resize(0);
	LeaveCriticalSection(&s_outgoingMessageCS);
}



void Debug::DebugTool::SubscribeToMessage( unsigned int messageID, DebugMessageListener* listener )
{
	MessageListenersContainer::iterator it = m_listeners.find( messageID );
	if (it == m_listeners.end())
	{
		// message ID wasn't added before, so add a new entry for this:
		ListenerContainer lc;
		lc.push_back( listener );
		m_listeners[messageID] = lc;
	}
	else
	{
		// Message ID was already in there, so just add this listener to that same message:
		(it->second).push_back( listener );
	}
}


void Debug::DebugTool::UnsubscribeListener( DebugMessageListener* listener )
{
	MessageListenersContainer::iterator it = m_listeners.begin();

	for (;it!=m_listeners.end(); )
	{
		// Look in each listener container to find this listener:
		ListenerContainer& c = (it->second);

		{
			ListenerContainer::iterator i = c.begin();
			for (;i!=c.end();)
			{
				if (*i == listener)
				{
					i = c.erase(i);
				}
				else
				{
					++i;
				}
			}
		}

		if (c.empty())
		{
			it = m_listeners.erase(it);
		}
		else
		{
			++it;
		}
	}
}

#endif