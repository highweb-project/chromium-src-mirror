/*
 * ContactSuccessCallback.h
 *
 *  Created on: 2015. 12. 9.
 *      Author: azureskybox
 */

#ifndef MessagingSuccessCallback_h
#define MessagingSuccessCallback_h

#include "wtf/text/WTFString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "modules/device_messaging/MessageObject.h"

namespace blink {

class MessagingSuccessCallback : public GarbageCollectedFinalized<MessagingSuccessCallback> {
public:
	virtual ~MessagingSuccessCallback() { }
	virtual void handleResult(const HeapVector<Member<MessageObject>>& messageObjs) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif  // ContactSuccessCallback_h
