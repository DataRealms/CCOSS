#include "pch.h"
#include "BinaryWriterNormal.h"
#include "ProfileStatusMessage.h"
#include "DebugTool/DebugMessageFactory.h"



REGISTER_DEBUGMESSAGE( ProfileStatusMessage, 0x50534D45 );



ProfileStatusMessage::ProfileStatusMessage()
{
	m_currentFrame = -1;
	m_paused = false;
}



ProfileStatusMessage::ProfileStatusMessage( int currentFrame, bool paused )
{
	m_currentFrame = currentFrame;
	m_paused = paused;
}



void ProfileStatusMessage::Serialize( BinaryWriterNormal &writer )
{
	writer.Write( m_currentFrame );
	writer.Write( m_paused );
}



void ProfileStatusMessage::Deserialize( BinaryReaderNormal& )
{
	SlickAssert(false);	// Should never get called.
}
