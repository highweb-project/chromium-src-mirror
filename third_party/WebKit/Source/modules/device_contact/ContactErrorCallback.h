/*
 * ContactSuccessCallback.h
 *
 *  Created on: 2015. 12. 9.
 *      Author: azureskybox
 */

#ifndef ContactErrorCallback_h
#define ContactErrorCallback_h

#include "wtf/text/WTFString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "modules/device_contact/ContactObject.h"

namespace blink {

class ContactErrorCallback : public GarbageCollectedFinalized<ContactErrorCallback> {
public:
	virtual ~ContactErrorCallback() { }
	virtual void handleResult(unsigned error) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif  // ContactErrorCallback_h.h
