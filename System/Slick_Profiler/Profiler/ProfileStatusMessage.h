#pragma once



#include <DebugTool/DebugMessage.h>



class ProfileStatusMessage : public Debug::DebugMessageTemplate<ProfileStatusMessage, 0x50534D45>
{
public:
	ProfileStatusMessage();
	ProfileStatusMessage( int currentFrame, bool paused );

	virtual void Serialize( BinaryWriterNormal &writer );
	virtual void Deserialize( BinaryReaderNormal &reader );

private:
	int	m_currentFrame;
	bool m_paused;
};