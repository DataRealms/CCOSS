#pragma once



namespace Debug
{
	class DebugMessage;
	
	class DebugMessageListener
	{
	public:
		virtual ~DebugMessageListener()	{}

		virtual void HandleDebugMessage(const Debug::DebugMessage *debugmsg ) = 0;
	};
}