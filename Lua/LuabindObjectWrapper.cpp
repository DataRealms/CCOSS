// Make sure that this wrapper file is always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire cause luabind is a nightmare!

#include "LuabindObjectWrapper.h"
#include "luabind/object.hpp"

namespace RTE {
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuabindObjectWrapper::Destroy() {
		delete m_LuabindObject;
	}	
}