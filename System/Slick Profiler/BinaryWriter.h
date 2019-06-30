#ifndef BINARYWRITER_H
#define BINARYWRITER_H


#ifdef _XBOX
#include <BinaryWriterNormal.h>
typedef BinaryWriterNormal BinaryWriter;
#else
#include <BinaryWriterEndianSwapped.h>
typedef BinaryWriterEndianSwapped BinaryWriter;
#endif


#endif