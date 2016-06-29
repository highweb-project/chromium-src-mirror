/*
 * NavigatorContact.cpp
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */
#include "wtf/build_config.h"
#include "modules/device_messaging/NavigatorMessaging.h"

#include "core/frame/Navigator.h"
#include "modules/device_messaging/Messaging.h"

namespace blink {

NavigatorMessaging& NavigatorMessaging::from(Navigator& navigator)
{
	NavigatorMessaging* supplement = static_cast<NavigatorMessaging*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        supplement = new NavigatorMessaging(navigator);
        provideTo(navigator, supplementName(), supplement);
    }
    return *supplement;
}

Messaging* NavigatorMessaging::messaging(Navigator& navigator)
{
	return NavigatorMessaging::from(navigator).messaging();
}

Messaging* NavigatorMessaging::messaging()
{
	return mMessaging;
}

DEFINE_TRACE(NavigatorMessaging)
{
    visitor->trace(mMessaging);
}

NavigatorMessaging::NavigatorMessaging(Navigator& navigator)
{
	if(navigator.frame())
		mMessaging = Messaging::create(*navigator.frame());
}

const char* NavigatorMessaging::supplementName()
{
    return "NavigatorMessaging";
}

}



