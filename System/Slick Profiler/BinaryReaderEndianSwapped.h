#ifndef BINARYREADERENDIANSWAPPED_H
#define BINARYREADERENDIANSWAPPED_H

#include <string>
#include <allegro.h>

class BinaryReaderEndianSwapped
{
public:
	BinaryReaderEndianSwapped( const char* ptr, int size );
	~BinaryReaderEndianSwapped();

public:
	bool					IsEOF() const;

	bool					ReadBoolean();
	unsigned char			ReadByte();
	const unsigned char *	ReadBytes(int count);
	char					ReadChar();
	const char *			ReadChars(int count);
	double					ReadDouble();
	short					ReadInt16();
	int					ReadInt32();
	int64_t					ReadInt64();
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
	#endif //
	
	template<typename T>
	T						Read();

private:
	int						Read7BitEncodedInt();

	const char*				m_endptr;
	const char*				m_ptr;

	int						m_readPtr;

	char					m_swapbuffer[8];
	const char *			EndianSwap( int count );
};

template<typename T>
T BinaryReaderEndianSwapped::Read()
{
	SlickAssert( m_ptr <= m_endptr-sizeof(T) );
	return *((T*)EndianSwap( sizeof(T) ));
}


#endif
