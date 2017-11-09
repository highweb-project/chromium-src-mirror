/*
 * NavigatorContact.h
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */

#ifndef NavigatorContact_h
#define NavigatorContact_h

#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"
#include "Contact.h"

namespace blink {

class Navigator;
class Contact;

class NavigatorContact final
	: public GarbageCollected<NavigatorContact>,
	  public Supplement<Navigator> {
	USING_GARBAGE_COLLECTED_MIXIN(NavigatorContact);
public:
	static NavigatorContact& From(Navigator&);

	static Contact* contact(Navigator& navigator);
	Contact* contact();

    DECLARE_TRACE();

private:
    explicit NavigatorContact(Navigator&);
    static const char* SupplementName();

    Member<Contact> mContact;
};

}

#endif  // NavigatorContact_h
