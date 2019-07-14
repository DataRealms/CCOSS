#include "pch.h"
#include "DebugMessageFactory.h"

Debug::DebugMessageCreator& Debug::DebugMessageCreator::Instance()
{
	static Debug::DebugMessageCreator s_instance;
	return s_instance;
}
