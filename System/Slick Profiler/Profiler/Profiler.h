#ifndef PROFILER_H
#define PROFILER_H


class ProfileReportMessage;
#include <DebugTool/DebugMessageListener.h>

#ifdef SLICK_PROFILER
#define PROFILER_ENABLED
#endif

#ifdef PROFILER_ENABLED
class ProfileObject
{
public:
	ProfileObject( const char *string, unsigned int color );
	~ProfileObject();

private:
	const char *	m_string;
	unsigned int	m_color;
};



#define SLICK_PROFILE(color) ProfileObject _prof_obj_(__FUNCTION__, color);
#define SLICK_PROFILENAME( name, color ) ProfileObject _prof_obj_(__FUNCTION__ #name, color);
#define SLICK_PROFILE_DYNAMIC_NAME( name, colour ) ProfileObject _prof_obj( name, colour );



class ProfileSystem : public Debug::DebugMessageListener
{
public:
	ProfileSystem();
	~ProfileSystem();

	static bool 	Initialize();
	static bool 	Finalize();	
	static ProfileSystem*	Instance()		{ SlickAssert(s_instance); return s_instance; }

	bool			StartUp();
	bool			ShutDown();

	void			Update();

/*	void			DisableProfiling();
	void			EnableProfiling();
*/
	void			AddProfileSample( const char *name, __int64 timing, bool enter, unsigned int color );
	virtual void	HandleDebugMessage( const Debug::DebugMessage *debugmsg );

private:
	static ProfileSystem *	s_instance;

	bool					m_profilingEnabled;

	__int64			        m_frequency;
	ProfileReportMessage *	m_currentMsg;

	bool					m_gamePaused;
	int						m_nrFramesToProfile;
};

#else	// #ifdef PROFILER_ENABLED
	#define SLICK_PROFILE(color);
	#define SLICK_PROFILENAME( name, color );
	#define SLICK_PROFILE_DYNAMIC_NAME( name, colour );
#endif

#endif