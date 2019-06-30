#pragma once

#include <string>
#include <sstream>
#include <allegro.h>


class BinaryWriterEndianSwapped
{
public:
	BinaryWriterEndianSwapped();
	BinaryWriterEndianSwapped(int reserveSize);

public:
	void	Write( bool boolean );
	void	Write( unsigned char byte );
	void	Write( const char* byte, int count );
	void	Write( char ch );
	void	Write( wchar_t ch );
	void	Write( const unsigned char* byte, int count );
	void	Write( double d );
	void	Write( short sh );
	void	Write( int i );
	void	Write( int64_t i64 );

	void	Write( const std::string &str );
	void	Write( const char * str );	// Null terminated string
	void	Write( const std::wstring &str );
	void	Write( const wchar_t* str );

	void	Write( float d );
	void	Write( unsigned short sh );
	void	Write( unsigned int i );
	void	Write( uint64_t i64 );

	void	WriteBytes( const unsigned char* byte, int count );
	
	template<typename T>
	void WriteT( const T& data )
	{
		const char *ptr = (const char*)&data;
		for (int i=sizeof(T)-1; i>=0; --i)
		{
			m_data.push_back(ptr[i]);
		}
	}

	size_t		GetDataSize() const;
	const char*	GetData() const;

	void		Clear();

private:
	void		Write7BitEncodedInt( int length );

	std::vector<char>	m_data;
};

typedef BinaryWriterEndianSwapped BinaryWriter;

