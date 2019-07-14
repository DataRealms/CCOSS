#include "pch.h"
#include <BinaryWriterNormal.h>
#include "ProfileControlMessage.h"
#include <BinaryReaderNormal.h>

REGISTER_DEBUGMESSAGE( ProfileControlMessage, 0x50434D45 );



void ProfileControlMessage::Serialize( BinaryWriterNormal &/*writer*/ )
{
}



void ProfileControlMessage::Deserialize( BinaryReaderNormal &reader )
{
	m_paused = reader.ReadBoolean();
	m_numberOfFramesToProfile = reader.ReadInt32();

}
