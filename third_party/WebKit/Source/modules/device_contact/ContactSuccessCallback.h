/*
 * ContactSuccessCallback.h
 *
 *  Created on: 2015. 12. 9.
 *      Author: azureskybox
 */

#ifndef ContactSuccessCallback_h
#define ContactSuccessCallback_h

#include "wtf/text/WTFString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "modules/device_contact/ContactObject.h"

namespace blink {

class ContactSuccessCallback : public GarbageCollectedFinalized<ContactSuccessCallback> {
public:
	virtual ~ContactSuccessCallback() { }
	virtual void handleResult(const HeapVector<Member<ContactObject>>& contactObjs) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif  // ContactSuccessCallback_h
