/*
 * NavigatorContact.cpp
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_contact/NavigatorContact.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorContact& NavigatorContact::From(Navigator& navigator)
{
	NavigatorContact* supplement = static_cast<NavigatorContact*>(Supplement<Navigator>::From(navigator, SupplementName()));
    if (!supplement) {
        supplement = new NavigatorContact(navigator);
        ProvideTo(navigator, SupplementName(), supplement);
    }
    return *supplement;
}

Contact* NavigatorContact::contact(Navigator& navigator)
{
	return NavigatorContact::From(navigator).contact();
}

Contact* NavigatorContact::contact()
{
	return mContact;
}

DEFINE_TRACE(NavigatorContact)
{
    visitor->Trace(mContact);
    Supplement<Navigator>::Trace(visitor);
}

NavigatorContact::NavigatorContact(Navigator& navigator)
{
	if(navigator.GetFrame())
		mContact = Contact::create(*navigator.GetFrame());
}

const char* NavigatorContact::SupplementName()
{
    return "NavigatorContact";
}

}
