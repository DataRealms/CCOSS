#include "pch.h"
#include "BinaryWriterNormal.h"


BinaryWriterNormal::BinaryWriterNormal()
{
	m_data.reserve(1024);
}



BinaryWriterNormal::BinaryWriterNormal(int reserveSize)
{
	m_data.reserve(reserveSize);
}



void BinaryWriterNormal::Write( bool boolean )
{
	m_data.push_back( (char)boolean );
}



void BinaryWriterNormal::Write( unsigned char byte )
{
	m_data.push_back(byte);
}



void BinaryWriterNormal::Write( const char* byte, int count )
{
	// Is there a faster way?
	for (int i=0; i<count; ++i)
	{
		m_data.push_back(byte[i]);
	}
}



void BinaryWriterNormal::Write( char ch )
{
	m_data.push_back(ch);
}


void BinaryWriterNormal::Write( wchar_t ch )
{
	const char *ptr = (const char*)&ch;
	for (int i=0; i<sizeof(ch); ++i)
	{
		m_data.push_back(ptr[i]);
	}
}

void BinaryWriterNormal::Write( const unsigned char* byte, int count )
{
	// Is there a faster way?
	for (int i=0; i<count; ++i)
	{
		m_data.push_back(byte[i]);
	}
}



void BinaryWriterNormal::Write( double d )
{
	const char *ptr = (const char*)&d;
	for (int i=0; i<sizeof(d); ++i)
	{
		m_data.push_back(ptr[i]);
	}
}


void BinaryWriterNormal::Write( short sh )
{
	const char *ptr = (const char*)&sh;
	for (int i=0; i<sizeof(sh); ++i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterNormal::Write( int in )
{
	const char *ptr = (const char*)&in;
	for (int i=0; i<sizeof(in); ++i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterNormal::Write( int64_t i64 )
{
	const char *ptr = (const char*)&i64;
	for (int i=0; i<sizeof(i64); ++i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterNormal::Write( const std::string &str )
{
	int len = (int)str.size();
	Write7BitEncodedInt( len );

	Write( str.c_str(), len );
}



void BinaryWriterNormal::Write( const char * str )
{
	int len = (int)strlen(str);
	Write7BitEncodedInt( len );

	Write( str, len );
}

void BinaryWriterNormal::Write( const std::wstring& str )
{
	Write( str.c_str() );
}

void BinaryWriterNormal::Write( const wchar_t* str )
{
	SlickAssert(false); // DISABLED, Daniel: put your string utf16 to utf 8 here if you need it
/*	size_t len = wcslen(str);
	Write7BitEncodedInt( (int)len );

	char utf8str[4096];
	StringPool::UTF16_To_UTF8(str, utf8str, 4096);

	for( unsigned int i = 0; i < len; ++i )
	{
		Write( utf8str[i] );
	}*/
}


void BinaryWriterNormal::Write( float d )
{
	const char *ptr = (const char*)&d;
	for (int i=0; i<sizeof(d); ++i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterNormal::Write( unsigned short sh )
{
	const char *ptr = (const char*)&sh;
	for (int i=0; i<sizeof(sh); ++i)
	{
		m_data.push_back(ptr[i]);
	}}



void BinaryWriterNormal::Write( unsigned int in )
{
	const char *ptr = (const char*)&in;
	for (int i=0; i<sizeof(in); ++i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterNormal::Write( uint64_t i64 )
{
	const char *ptr = (const char*)&i64;
	for (int i=0; i<sizeof(i64); ++i)
	{
		m_data.push_back(ptr[i]);
	}
}


void BinaryWriterNormal::WriteBytes( const unsigned char* byte, int count )
{
	m_data.insert( m_data.end(), byte, byte + count );
}


void BinaryWriterNormal::Clear()
{
	m_data.clear();
}



size_t BinaryWriterNormal::GetDataSize() const
{
	return m_data.size();
}



const char*	BinaryWriterNormal::GetData() const
{
	return &m_data[0];
}



void BinaryWriterNormal::Write7BitEncodedInt( int length )
{
	unsigned int v = (unsigned int)length;

	while (v >= 0x80) 
	{
		Write((unsigned char) (v | 0x80));
		v >>= 7;
	}

	Write((unsigned char)v);
}
