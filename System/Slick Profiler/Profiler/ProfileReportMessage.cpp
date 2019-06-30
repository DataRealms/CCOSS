#include "pch.h"
#include <BinaryWriterNormal.h>
#include <cstdlib>
#include "ProfileReportMessage.h"


REGISTER_DEBUGMESSAGE( ProfileReportMessage, 0x50524D45 );


void ProfileReportMessage::ProfileSample::Save( BinaryWriterNormal &writer ) const
{
	writer.Write( (uintptr_t)m_name );
	writer.Write( m_timing );
	writer.Write( m_enter );
	writer.Write( m_color );
}




ProfileReportMessage::ProfileReportMessage()
{
	m_samples.reserve(10000);	// Reserve a large batch
}



void ProfileReportMessage::AddProfileSample( const char *name, int64_t timing, bool enter, unsigned int color )
{
	m_samples.push_back( ProfileSample( name, timing, enter, color ) );
}



void ProfileReportMessage::Serialize( BinaryWriterNormal &writer )
{
	writer.Write((int) 1);	// version
	writer.Write(m_frequency);

	ProfileSampleContainer::const_iterator i = m_samples.begin();
	ProfileSampleContainer::const_iterator e = m_samples.end();

	// Construct the string table:
	typedef std::set<const char *> PtrSet;
	PtrSet ptrTable;

	for (; i!=e; ++i)
	{
		const ProfileSample &sample = *i;
		if (ptrTable.find(sample.m_name) == ptrTable.end())
		{
			ptrTable.insert(sample.m_name);
		}
	}

	// Write out the string table:
	writer.Write( ptrTable.size() );

	// Write out the strings:
	PtrSet::const_iterator si = ptrTable.begin();
	PtrSet::const_iterator se = ptrTable.end();
	for (; si!=se; ++si)
	{
		uintptr_t ptr = (uintptr_t)*si;
		writer.Write(ptr);			// Write the pointer value
		writer.Write(*si);		// Write the actual string
	}

	// Write out the individual sample data:
	writer.Write( m_samples.size() );

	i = m_samples.begin();
	e = m_samples.end();
	for (; i!=e; ++i)
	{
		(*i).Save(writer);
	}
}



void ProfileReportMessage::Deserialize( BinaryReaderNormal &/*reader*/ )
{
	SlickAssert(false);	// This should never be called!
}
