/*
 * NavigatorMessaging.h
 *
 *  Created on: 2015. 12. 21.
 *      Author: azureskybox
 */

#ifndef NavigatorMessaging_h
#define NavigatorMessaging_h

#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"
#include "Messaging.h"

namespace blink {

class Navigator;
class Messaging;

class NavigatorMessaging final
	: public GarbageCollected<NavigatorMessaging>,
	  public Supplement<Navigator> {
	USING_GARBAGE_COLLECTED_MIXIN(NavigatorMessaging);
public:
	static NavigatorMessaging& From(Navigator&);

	static Messaging* messaging(Navigator& navigator);
	Messaging* messaging();

    DECLARE_TRACE();

private:
    explicit NavigatorMessaging(Navigator&);
    static const char* SupplementName();

    Member<Messaging> mMessaging;
};

}

#endif // NavigatorMessaging_h
