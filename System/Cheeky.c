#if defined(__APPLE__)
#include "CoreServices/CoreServices.h"

// [CHRISK] libfmod.a wants to link with an ancient version of Carbon 
// The methods listed below are no longer available so we are faking
// them and hoping for the best. This method is from open transport
// and AFAIK it is not being used at all in our use of FMOD. I 
// *think* it is only used for streaming stuff over a network.

typedef SInt32 OTResult;
typedef struct OpaqueEndpointRef * EndpointRef;
typedef ByteCount OTByteCount;

OTResult OTCountDataBytes (EndpointRef ref, OTByteCount * countPtr)
{
    return 0;
}
#endif // 
