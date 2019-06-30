#include "pch.h"
#include "BinaryWriterEndianSwapped.h"


BinaryWriterEndianSwapped::BinaryWriterEndianSwapped()
{
	m_data.reserve(1024);
}



BinaryWriterEndianSwapped::BinaryWriterEndianSwapped(int reserveSize)
{
	m_data.reserve(reserveSize);
}



void BinaryWriterEndianSwapped::Write( bool boolean )
{
	m_data.push_back( (char)boolean );
}



void BinaryWriterEndianSwapped::Write( unsigned char byte )
{
	m_data.push_back(byte);
}



void BinaryWriterEndianSwapped::Write( const char* byte, int count )
{
	// Is there a faster way?
	for (int i=0; i<count; ++i)
	{
		m_data.push_back(byte[i]);
	}
}



void BinaryWriterEndianSwapped::Write( char ch )
{
	m_data.push_back(ch);
}

void BinaryWriterEndianSwapped::Write( wchar_t ch )
{
	const char *ptr = (const char*)&ch;
	for (int i=sizeof(ch)-1; i>=0; --i)
	{
		m_data.push_back(ptr[i]);
	}
}

void BinaryWriterEndianSwapped::Write( const unsigned char* byte, int count )
{
	// Is there a faster way?
	for (int i=0; i<count; ++i)
	{
		m_data.push_back(byte[i]);
	}
}



void BinaryWriterEndianSwapped::Write( double d )
{
	const char *ptr = (const char*)&d;
	for (int i= sizeof(d)-1; i >= 0; --i)
	{
		m_data.push_back(ptr[i]);
	}
}


void BinaryWriterEndianSwapped::Write( short sh )
{
	const char *ptr = (const char*)&sh;
	for (int i=sizeof(sh)-1; i>=0; --i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterEndianSwapped::Write( int in )
{
	const char *ptr = (const char*)&in;
	for (int i=sizeof(in)-1; i>=0; --i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterEndianSwapped::Write( int64_t i64 )
{
	const char *ptr = (const char*)&i64;
	for (int i=sizeof(i64)-1; i>=0; --i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterEndianSwapped::Write( const std::string &str )
{
	int len = (int)str.size();
	Write7BitEncodedInt( len );

	Write( str.c_str(), len );
}



void BinaryWriterEndianSwapped::Write( const char * str )
{
	int len = (int)strlen(str);
	Write7BitEncodedInt( len );

	Write( str, len );
}


void BinaryWriterEndianSwapped::Write( const std::wstring& str )
{
	Write( str.c_str() );
}

void BinaryWriterEndianSwapped::Write( const wchar_t* str )
{
/*	size_t len = wcslen(str);
	Write7BitEncodedInt( (int)len );

	char utf8str[4096];
	StringPool::UTF16_To_UTF8(str, utf8str, 4096);

	for( unsigned int i = 0; i < len; ++i )
	{
		Write( utf8str[i] );
	}*/
	SlickAssert(false);	// Daniel: put your implementation here if you need it
}


void BinaryWriterEndianSwapped::Write( float d )
{
	const char *ptr = (const char*)&d;
	for (int i=sizeof(d)-1; i>=0; --i)
	{
		m_data.push_back(ptr[i]);
	}
}



void BinaryWriterEndianSwapped::Write( unsigned short sh )
{
	const char *ptr = (const char*)&sh;
	for (int i=sizeof(sh)-1; i>=0; --i)
	{
		m_data.push_back(ptr[i]);
	}}



void BinaryWriterEndianSwapped::Write( unsigned int in )
{
	const char *ptr = (const char*)&in;
	for (int i=sizeof(in)-1; i>=0; --i)
	{
		m_data.push_back(ptr[i]);
	}
}


void BinaryWriterEndianSwapped::Write( uint64_t i64 )
{
	const char *ptr = (const char*)&i64;
	for (int i=sizeof(i64)-1; i>=0; --i)
	{
		m_data.push_back(ptr[i]);
	}
}


void BinaryWriterEndianSwapped::WriteBytes( const unsigned char* byte, int count )
{
	m_data.insert( m_data.end(), byte, byte + count );
}



void BinaryWriterEndianSwapped::Clear()
{
	m_data.clear();
}



size_t BinaryWriterEndianSwapped::GetDataSize() const
{
	return m_data.size();
}



const char*	BinaryWriterEndianSwapped::GetData() const
{
	return &m_data[0];
}



void BinaryWriterEndianSwapped::Write7BitEncodedInt( int length )
{
	unsigned int v = (unsigned int)length;

	while (v >= 0x80) 
	{
		Write((unsigned char) (v | 0x80));
		v >>= 7;
	}

	Write((unsigned char)v);
}
