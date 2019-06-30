#ifndef PROFILECONTROLMESSAGE_H
#define PROFILECONTROLMESSAGE_H



#include <DebugTool/DebugMessage.h>



class ProfileControlMessage : public Debug::DebugMessageTemplate<ProfileControlMessage, 0x50434D45>
{
public:
	virtual void Serialize( BinaryWriterNormal &writer );
	virtual void Deserialize( BinaryReaderNormal &reader );

	bool	IsPaused() const					{ return m_paused; }
	int		NumberOfFramesToProfile() const		{ return m_numberOfFramesToProfile; }

private:
	bool	m_paused;
	int		m_numberOfFramesToProfile;
};



#endif