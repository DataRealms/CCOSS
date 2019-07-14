#ifndef BINARYREADER_H
#define BINARYREADER_H


#ifdef _XBOX
#include <BinaryReaderNormal.h>
typedef BinaryReaderNormal BinaryReader;
#else
#include <BinaryReaderEndianSwapped.h>
typedef BinaryReaderEndianSwapped BinaryReader;
#endif



#endif