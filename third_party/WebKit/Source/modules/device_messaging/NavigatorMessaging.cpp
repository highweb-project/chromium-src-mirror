/*
 * NavigatorContact.cpp
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_messaging/NavigatorMessaging.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorMessaging& NavigatorMessaging::From(Navigator& navigator)
{
	NavigatorMessaging* supplement = static_cast<NavigatorMessaging*>(Supplement<Navigator>::From(navigator, SupplementName()));
    if (!supplement) {
        supplement = new NavigatorMessaging(navigator);
        ProvideTo(navigator, SupplementName(), supplement);
    }
    return *supplement;
}

Messaging* NavigatorMessaging::messaging(Navigator& navigator)
{
	return NavigatorMessaging::From(navigator).messaging();
}

Messaging* NavigatorMessaging::messaging()
{
	return mMessaging;
}

DEFINE_TRACE(NavigatorMessaging)
{
    visitor->Trace(mMessaging);
    Supplement<Navigator>::Trace(visitor);
}

NavigatorMessaging::NavigatorMessaging(Navigator& navigator)
{
	if(navigator.GetFrame())
		mMessaging = Messaging::create(*navigator.GetFrame());
}

const char* NavigatorMessaging::SupplementName()
{
    return "NavigatorMessaging";
}

}
