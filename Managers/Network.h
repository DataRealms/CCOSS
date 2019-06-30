#ifndef _RTENETWORK_
#define _RTENETWORK_


#define WIN32_LEAN_AND_MEAN
#define NOGDI

// Allegro defines those via define in astdlib.h and RakNet goes crazy about those so we need to undefine them manually.
#undef int8_t
#undef uint8_t
#undef int16_t
#undef uint16_t
#undef int32_t
#undef uint32_t
#undef intptr_t
#undef uintptr_t

#undef LONG_LONG
#undef int64_t
#undef uint64_t

#include <WinSock2.h>
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"

#undef WIN32_LEAN_AND_MEAN
#undef NO_GDI

#include "NetworkMessages.h"

#endif