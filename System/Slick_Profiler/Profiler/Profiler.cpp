#include "pch.h"

#if defined(_MSC_VER)
#include <allegro.h>
#include <winalleg.h>
#endif // defined(_MSC_VER_)

#include "Profiler.h"

#ifdef PROFILER_ENABLED

#include <DebugTool/DebugTool.h>
#include "ProfileReportMessage.h"
#include "ProfileStatusMessage.h"
#include "ProfileControlMessage.h"



ProfileSystem*	ProfileSystem::s_instance = 0;




ProfileObject::ProfileObject( const char *string, unsigned int color )
{
	m_string = string;
	m_color = color;

	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

    ProfileSystem::Instance()->AddProfileSample( m_string, time.QuadPart, true, m_color );
}



ProfileObject::~ProfileObject()
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

    ProfileSystem::Instance()->AddProfileSample( m_string, time.QuadPart, false, m_color );
}





bool ProfileSystem::Initialize()
{
	SlickAssert(s_instance == NULL);
	s_instance = new ProfileSystem;
	if (s_instance)
	{
		if (!s_instance->StartUp())
		{
			delete s_instance;
			s_instance = 0;
		}
	}

	ProfileControlMessage m_dummyControlMessage;	// Need to define this here, because otherwise the linker will throw away the code for this class. Weird but true

	return s_instance != NULL;
}



bool ProfileSystem::Finalize()
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



ProfileSystem::ProfileSystem()
{
	m_currentMsg = new ProfileReportMessage();
	m_gamePaused = false;
	m_nrFramesToProfile = 0;
	m_profilingEnabled = false;
}



ProfileSystem::~ProfileSystem()
{
	delete m_currentMsg;
}


bool ProfileSystem::StartUp()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
    m_frequency = freq.QuadPart;
	m_currentMsg->SetFrequency( m_frequency );

	// Register the messages we're interested in with the debug tool:
	Debug::DebugTool::Instance()->SubscribeToMessage(0x50434D45, this);	// ProfileControlMessage
	return true;
}



bool ProfileSystem::ShutDown()
{
	Debug::DebugTool::Instance()->UnsubscribeListener(this);
	return false;
}



/*void ProfileSystem::DisableProfiling()
{
	s_profilingEnabled = false;
}



void ProfileSystem::EnableProfiling()
{
	s_profilingEnabled = true;
}*/



void ProfileSystem::AddProfileSample( const char *name, __int64 timing, bool enter, unsigned int color )
{
	if (m_profilingEnabled)
	{
		if (m_currentMsg != NULL)
		{
			m_currentMsg->AddProfileSample(name, timing, enter, color);
		}
	}
}



void ProfileSystem::Update()
{
	if (m_nrFramesToProfile > 0)
	{
		// Dump out a profile report to the network:
		if ((m_currentMsg != NULL) && (!m_currentMsg->IsEmpty()))
		{
			Debug::DebugTool::Instance()->SendMessage( m_currentMsg );
		}
		else
		{
			delete m_currentMsg;
		}
		m_currentMsg = new ProfileReportMessage();
		m_currentMsg->SetFrequency( m_frequency );
	}

	if (m_nrFramesToProfile > 0)
	{
		--m_nrFramesToProfile;
	}

	m_profilingEnabled = (m_nrFramesToProfile>0);
}


void ProfileSystem::HandleDebugMessage( const Debug::DebugMessage *debugmsg )
{
	switch (debugmsg->GetType())
	{
	case 0x50434D45:	// Control Message
		const ProfileControlMessage *msg = static_cast<const ProfileControlMessage*>(debugmsg);
		
		m_nrFramesToProfile = msg->NumberOfFramesToProfile()+1;
		break;
	}

}

#endif // #ifdef PROFILER_ENABLED