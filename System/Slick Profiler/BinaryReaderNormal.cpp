#include "pch.h"
#include "BinaryReaderNormal.h"


BinaryReaderNormal::BinaryReaderNormal( const char* ptr, int size )
{
	m_ptr = ptr;
	m_endptr = ptr + size;
	m_readPtr = 0;
}

BinaryReaderNormal::~BinaryReaderNormal()
{
}


bool BinaryReaderNormal::IsEOF() const
{
	return m_ptr >= m_endptr;
}


bool BinaryReaderNormal::ReadBoolean()
{
	SlickAssert( m_ptr <= m_endptr-1 );
	bool result = (*m_ptr != 0);
	m_ptr++;
	return result;
}



unsigned char BinaryReaderNormal::ReadByte()
{
	SlickAssert( m_ptr <= m_endptr-1 );
	unsigned char result = (unsigned char)(*m_ptr);
	m_ptr++;
	return result;
}



const unsigned char *	BinaryReaderNormal::ReadBytes(int count)
{
	SlickAssert( m_ptr <= m_endptr-count );
	const unsigned char *result = (const unsigned char *)m_ptr;
	m_ptr += count;
	return result;
}



char BinaryReaderNormal::ReadChar()
{
	SlickAssert( m_ptr <= m_endptr-1 );
	char result = (char)(*m_ptr);
	m_ptr++;
	return result;
}



const char * BinaryReaderNormal::ReadChars(int count)
{
	SlickAssert( m_ptr <= m_endptr-count );
	const char *result = (const char *)m_ptr;
	m_ptr += count;
	return result;
}



double BinaryReaderNormal::ReadDouble()
{
	SlickAssert( m_ptr <= m_endptr-8 );

	double result;
	
	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	resultptr[2] = m_ptr[2];
	resultptr[3] = m_ptr[3];
	resultptr[4] = m_ptr[0];
	resultptr[5] = m_ptr[1];
	resultptr[6] = m_ptr[2];
	resultptr[7] = m_ptr[3];

	m_ptr += 8;

	return result;
}



short BinaryReaderNormal::ReadInt16()
{
	SlickAssert( m_ptr <= m_endptr-2 );

	short result;
	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	m_ptr += 2;

	return result;
}



int	BinaryReaderNormal::ReadInt32()
{
	SlickAssert( m_ptr <= m_endptr-4 );

	int result;
	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	resultptr[2] = m_ptr[2];
	resultptr[3] = m_ptr[3];

	m_ptr += 4;
	return result;
}


#if defined(__APPLE__) || defined(__unix__)
int64_t	BinaryReaderNormal::ReadInt64()
{
	SlickAssert( m_ptr <= m_endptr-8 );
	int64_t result;
	
	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	resultptr[2] = m_ptr[2];
	resultptr[3] = m_ptr[3];
	resultptr[4] = m_ptr[0];
	resultptr[5] = m_ptr[1];
	resultptr[6] = m_ptr[2];
	resultptr[7] = m_ptr[3];

	m_ptr += 8;
	return result;
}
#else
__int64	BinaryReaderNormal::ReadInt64()
{
	SlickAssert( m_ptr <= m_endptr-8 );
	__int64 result;
	
	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	resultptr[2] = m_ptr[2];
	resultptr[3] = m_ptr[3];
	resultptr[4] = m_ptr[0];
	resultptr[5] = m_ptr[1];
	resultptr[6] = m_ptr[2];
	resultptr[7] = m_ptr[3];

	m_ptr += 8;
	return result;
}
#endif // defined(__APPLE__)



char BinaryReaderNormal::ReadSByte()
{
	SlickAssert( m_ptr <= m_endptr-1 );
	char result = (char)(*m_ptr);
	m_ptr++;
	return result;
}


float BinaryReaderNormal::ReadSingle()
{
	SlickAssert( m_ptr <= m_endptr-4 );
	float result;
	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	resultptr[2] = m_ptr[2];
	resultptr[3] = m_ptr[3];
	m_ptr += 4;
	return result;
}



std::string	BinaryReaderNormal::ReadString()
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


std::wstring BinaryReaderNormal::ReadWString()
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


unsigned short BinaryReaderNormal::ReadUInt16()
{
	SlickAssert( m_ptr <= m_endptr-2 );

	unsigned short result;
	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	m_ptr += 2;

	return result;
}



unsigned int BinaryReaderNormal::ReadUInt32()
{
	SlickAssert( m_ptr <= m_endptr-4 );

	unsigned int result;

	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	resultptr[2] = m_ptr[2];
	resultptr[3] = m_ptr[3];
	m_ptr += 4;

	return result;
}


#if defined(__APPLE__) || defined(__unix__)
uint64_t BinaryReaderNormal::ReadUInt64()
{
	SlickAssert( m_ptr <= m_endptr-8 );
	uint64_t result;

	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	resultptr[2] = m_ptr[2];
	resultptr[3] = m_ptr[3];
	resultptr[4] = m_ptr[4];
	resultptr[5] = m_ptr[5];
	resultptr[6] = m_ptr[6];
	resultptr[7] = m_ptr[7];

	m_ptr += 8;
	return result;
}
#else
unsigned __int64 BinaryReaderNormal::ReadUInt64()
{
	SlickAssert( m_ptr <= m_endptr-8 );
	unsigned __int64 result;

	char *resultptr = (char*)&result;
	resultptr[0] = m_ptr[0];			// Needs to be copied because the data may not be correctly aligned. This is a problem on the XBox 360
	resultptr[1] = m_ptr[1];
	resultptr[2] = m_ptr[2];
	resultptr[3] = m_ptr[3];
	resultptr[4] = m_ptr[4];
	resultptr[5] = m_ptr[5];
	resultptr[6] = m_ptr[6];
	resultptr[7] = m_ptr[7];

	m_ptr += 8;
	return result;
}
#endif // defined(__APPLE__)


int	BinaryReaderNormal::Read7BitEncodedInt()
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

