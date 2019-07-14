#include "pch.h"
#include "BinaryReaderEndianSwapped.h"


BinaryReaderEndianSwapped::BinaryReaderEndianSwapped( const char* ptr, int size )
{
	m_ptr = ptr;
	m_endptr = ptr + size;
	m_readPtr = 0;
}

BinaryReaderEndianSwapped::~BinaryReaderEndianSwapped()
{
}

bool BinaryReaderEndianSwapped::IsEOF() const
{
	return m_ptr >= m_endptr;
}


const char *BinaryReaderEndianSwapped::EndianSwap( int count )
{
	SlickAssert(count <= 8);

	for(int i=0; i<count; ++i)
	{
		m_swapbuffer[count-1-i] = *m_ptr;
		m_ptr++;
	}

	return m_swapbuffer;
}



bool BinaryReaderEndianSwapped::ReadBoolean()
{
	SlickAssert( m_ptr <= m_endptr-1 );
	bool result = (*m_ptr != 0);
	m_ptr++;
	return result;
}



unsigned char BinaryReaderEndianSwapped::ReadByte()
{
	SlickAssert( m_ptr <= m_endptr-1 );
	unsigned char result = (unsigned char)(*m_ptr);
	m_ptr++;
	return result;
}



const unsigned char *	BinaryReaderEndianSwapped::ReadBytes(int count)
{
	SlickAssert( m_ptr <= m_endptr-count );
	const unsigned char *result = (const unsigned char *)m_ptr;
	m_ptr += count;
	return result;
}



char BinaryReaderEndianSwapped::ReadChar()
{
	SlickAssert( m_ptr <= m_endptr-1 );
	char result = (char)(*m_ptr);
	m_ptr++;
	return result;
}



const char * BinaryReaderEndianSwapped::ReadChars(int count)
{
	SlickAssert( m_ptr <= m_endptr-count );
	const char *result = (const char *)m_ptr;
	m_ptr += count;
	return result;
}



double BinaryReaderEndianSwapped::ReadDouble()
{
	SlickAssert( m_ptr <= m_endptr-8 );
	return *((double*)EndianSwap( 8 ));
}



short BinaryReaderEndianSwapped::ReadInt16()
{
	SlickAssert( m_ptr <= m_endptr-2 );
	return *((short*)EndianSwap( 2 ));
}



int	BinaryReaderEndianSwapped::ReadInt32()
{
	SlickAssert( m_ptr <= m_endptr-4 );
	return *((int*)EndianSwap( 4 ));
}


#if defined(__APPLE__) || defined(__unix__)
int64_t BinaryReaderEndianSwapped::ReadInt64()
{
	SlickAssert( m_ptr <= m_endPtr-8 );
	return *((int64_t*)EndianSwap( 8 ));
}
#else
__int64	BinaryReaderEndianSwapped::ReadInt64()
{
	SlickAssert( m_ptr <= m_endptr-8 );
	return *((__int64*)EndianSwap( 8 ));
}
#endif // defined(__APPLE__)



char BinaryReaderEndianSwapped::ReadSByte()
{
	SlickAssert( m_ptr <= m_endptr-1 );
	char result = (char)(*m_ptr);
	m_ptr++;
	return result;
}


float BinaryReaderEndianSwapped::ReadSingle()
{
	SlickAssert( m_ptr <= m_endptr-4 );
	return *((float*)EndianSwap( 4 ));
}



std::string	BinaryReaderEndianSwapped::ReadString()
{
	SlickAssert( m_ptr < m_endptr );
	int length = Read7BitEncodedInt();

	SlickAssert( m_ptr <= m_endptr-length );
	if (m_ptr <= m_endptr-length)
	{			
		std::string str;

		// Add it character by character. There must be a better way to do this.. ?
		for(int i=0; i<length; ++i)
		{
			str += *m_ptr;
			m_ptr++;
		}
		return str;
	}

	return std::string("");	// empty string
}

std::wstring BinaryReaderEndianSwapped::ReadWString()
{
	std::wstring str;

	SlickAssert(false);	// DISABLED. Daniel: put your stuff in here if you want to use it.

/*	SlickAssert( m_ptr < m_endptr );
	int length = Read7BitEncodedInt();

	SlickAssert( m_ptr <= m_endptr-length );
	if (m_ptr <= m_endptr-length)
	{			
		char utf8str[4096];
		memcpy_s(utf8str, 4096, m_ptr, length);
		m_ptr += length;
		utf8str[__min(length, 4096-1)] = 0;	// terminator

		wchar_t wstr[4096];
		StringPool::UTF8_To_UTF16( utf8str, wstr, 4096 );
		str = wstr;
	}*/

	return str;
}


unsigned short BinaryReaderEndianSwapped::ReadUInt16()
{
	SlickAssert( m_ptr <= m_endptr-2 );
	return *((unsigned short *)EndianSwap( 2 ));
}

unsigned int BinaryReaderEndianSwapped::ReadUInt32()
{
	SlickAssert( m_ptr <= m_endptr-4 );
	return *((unsigned int *)EndianSwap( 4 ));
}


#if defined(__APPLE__) || defined(__unix__)
uint64_t BinaryReaderEndianSwapped::ReadUInt64()
{
	SlickAssert(m_ptr <= m_endPtr-8);
	return *((uint64_t*)EndianSwap(8));
}
#else
unsigned __int64 BinaryReaderEndianSwapped::ReadUInt64()
{
	SlickAssert( m_ptr <= m_endptr-8 );
	return *((unsigned __int64 *)EndianSwap( 8 ));
}
#endif // defined(__APPLE__)



int	BinaryReaderEndianSwapped::Read7BitEncodedInt()
{
	SlickAssert( m_ptr < m_endptr );
	int ret = 0;
	for (int shift = 0; shift < 35; shift += 7)
	{
		int b = (int)(*m_ptr);
		
		m_ptr++;
		SlickAssert( m_ptr < m_endptr );

		ret = ret | ((b & 0x7f) << shift);
		if ((b & 0x80) == 0)
		{
			return ret;
		}
	}

	return 0;
}

