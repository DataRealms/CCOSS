#ifndef DEBUGTOOL_H
#define DEBUGTOOL_H

#ifdef SLICK_PROFILER
#define DEBUGTOOL_ENABLED
#endif

#ifdef DEBUGTOOL_ENABLED

#include <vector>
#include <list>
#include <map>
#include "debug.h"

namespace Debug
{
	class DebugMessage;
	class DebugMessageListener;



	typedef std::vector<DebugMessage*> DebugMessageContainer;



	class DebugTool
	{
	public:
		DebugTool();
		~DebugTool();

		static bool 			Initialize();
		static bool 			Finalize();	
		static DebugTool*		Instance()						{ SlickAssert(s_instance); return s_instance; }

		bool					StartUp();
		bool					ShutDown();

		bool					IsDebugToolConnected() const;

		void					Update();

		void					SendMessage( DebugMessage *message );	// This class will take ownership of the message

		void					SubscribeToMessage( unsigned int messageID, DebugMessageListener* listener );
		void					UnsubscribeListener( DebugMessageListener* listener );

	private:
		static DebugTool *		s_instance;
/* Moved these to be outside and static in the DebugTool.cpp, to make the h independent of windows.h
		SOCKET					m_listenSocket;
		SOCKET					m_clientSocket;

		CRITICAL_SECTION		m_outgoingMessageCS;
*/

		DebugMessageContainer	m_outgoing;

		void					ReceiveMessages();
		void					SendMessages();

		void					HandleMessage( DebugMessage *msg );

		typedef std::list<DebugMessageListener*> ListenerContainer;
		typedef std::map< unsigned int, ListenerContainer> MessageListenersContainer;
		MessageListenersContainer	m_listeners;
	};
}
#endif

#endif