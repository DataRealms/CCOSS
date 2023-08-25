#include "RTEStackTrace.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string RTEStackTrace::GetCallStackAsString(const HANDLE &handle, const CONTEXT *context) {
		m_CallstackStream.clear();

		if (!handle || !context) {
			CONTEXT currentContext;
			RtlCaptureContext(&currentContext);
			this->StackWalker::ShowCallstack(GetCurrentThread(), &currentContext);
		} else {
			this->StackWalker::ShowCallstack(handle, context);
		}
		return m_CallstackStream.str();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEStackTrace::OnOutput(LPCSTR text) {
		this->StackWalker::OnOutput(text);
		m_CallstackStream << text;
	}
}