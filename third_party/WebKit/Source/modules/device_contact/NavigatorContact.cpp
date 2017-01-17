/*
 * NavigatorContact.cpp
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */
#include "wtf/build_config.h"
#include "modules/device_contact/NavigatorContact.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorContact& NavigatorContact::from(Navigator& navigator)
{
	NavigatorContact* supplement = static_cast<NavigatorContact*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        supplement = new NavigatorContact(navigator);
        provideTo(navigator, supplementName(), supplement);
    }
    return *supplement;
}

Contact* NavigatorContact::contact(Navigator& navigator)
{
	return NavigatorContact::from(navigator).contact();
}

Contact* NavigatorContact::contact()
{
	return mContact;
}

DEFINE_TRACE(NavigatorContact)
{
    visitor->trace(mContact);
		Supplement<Navigator>::trace(visitor);
}

NavigatorContact::NavigatorContact(Navigator& navigator)
{
	if(navigator.frame())
		mContact = Contact::create(*navigator.frame());
}

const char* NavigatorContact::supplementName()
{
    return "NavigatorContact";
}

}
