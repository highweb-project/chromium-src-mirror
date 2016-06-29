/*
 * ContactSuccessCallback.h
 *
 *  Created on: 2015. 12. 9.
 *      Author: azureskybox
 */

#ifndef MessagingErrorCallback_h
#define MessagingErrorCallback_h

#include "wtf/text/WTFString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

namespace blink {

class MessagingErrorCallback : public GarbageCollectedFinalized<MessagingErrorCallback> {
public:
	virtual ~MessagingErrorCallback() { }
	virtual void handleResult(unsigned error) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif  // ContactErrorCallback_h.h
