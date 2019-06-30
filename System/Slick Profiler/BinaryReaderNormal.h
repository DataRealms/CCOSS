#ifndef BINARYREADERNORMAL_H
#define BINARYREADERNORMAL_H



#include <string>
#include <allegro.h>


class BinaryReaderNormal
{
public:
	BinaryReaderNormal( const char* ptr, int size );
	~BinaryReaderNormal();

public:
	bool					IsEOF() const;

	bool					ReadBoolean();
	unsigned char			ReadByte();
	const unsigned char *	ReadBytes(int count);
	char					ReadChar();
	const char *			ReadChars(int count);
	double					ReadDouble();
	short					ReadInt16();
	int						ReadInt32();
	
#if defined(__APPLE__) || defined(__unix__)
	int64_t					ReadInt64();
#else
	__int64					ReadInt64();
#endif // defined(__APPLE__)

	char					ReadSByte();
	float					ReadSingle();
	std::string				ReadString();
	std::wstring			ReadWString();
	unsigned short			ReadUInt16();
	unsigned int			ReadUInt32();
	
#if defined(__APPLE__) || defined(__unix__)
	uint64_t				ReadUInt64();
#else
	unsigned __int64		ReadUInt64();
#endif // defined(__APPLE__)

	template<typename T>
	T						Read();

private:
	int						Read7BitEncodedInt();

	const char*				m_endptr;
	const char*				m_ptr;

	int						m_readPtr;
};

template<typename T>
T BinaryReaderNormal::Read()
{
	SlickAssert( m_ptr <= m_endptr-sizeof(T) );
	T result;
	
	char *resultptr = (char*)&result;
	for( unsigned int i = 0; i < sizeof(T); ++i )
		resultptr[i] = m_ptr[i];

	m_ptr += sizeof(T);
	return result;
}

#endif
