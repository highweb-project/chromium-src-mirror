/*
 * ContactObject.h
 *
 *  Created on: 2015. 12. 11.
 *      Author: azureskybox
 */

#ifndef ContactObject_h
#define ContactObject_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"
#include "platform/wtf/Vector.h"

namespace blink {

class ContactAddress;
class ContactName;

class ContactObject : public GarbageCollectedFinalized<ContactObject>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static ContactObject* Create()
	{
		ContactObject* contactObject = new ContactObject();
		return contactObject;
	}
	virtual ~ContactObject();

	// JS API Implementations
	WTF::String id() { return mID; };
	void setId(WTF::String id) { mID = id; };
	bool hasId() { return mID.length() > 0; };

	WTF::String displayName() const { return mDisplayName; }
	void setDisplayName(const WTF::String displayName) { mDisplayName = displayName; };
	bool hasDisplayName() { return mDisplayName.length() > 0; };

	WTF::String phoneNumber() { return mPhoneNumber; };
	void setPhoneNumber(WTF::String phoneNumber) { mPhoneNumber = phoneNumber; };
	bool hasPhoneNumber() { return mPhoneNumber.length() > 0; };

	WTF::String emails() { return mEmails; };
	void setEmails(WTF::String emails) { mEmails = emails; };
	bool hasEmails() { return mEmails.length() > 0; };

	WTF::String address() { return mAddress; };
	void setAddress(WTF::String address) { mAddress = address; };
	bool hasAddress() { return mAddress.length() > 0; };

	WTF::Vector<WTF::String> categories() { return mCategories; };
	void setCategories(WTF::Vector<WTF::String> categories) { mCategories = categories; };
	bool hasCategories() { return mCategories.size() >= 1; };

	WTF::String accountName() { return mAccountName; };
	void setAccountName(WTF::String accountName) { mAccountName = accountName; };
	bool hasAccountName() { return mAccountName.length() > 0; };

	WTF::String accountType() { return mAccountType; };
	void setAccountType(WTF::String accountType) { mAccountType = accountType; };
	bool hasAccountType() { return mAccountType.length() > 0; };

	ContactAddress* structuredAddress();
	void setStructuredAddress(ContactAddress* structuredAddress);
	bool hasStructuredAddress() { return mStructuredAddress.Get() != nullptr; };

	ContactName* structuredName();
	void setStructuredName(ContactName* structuredName);
	bool hasStructuredName() { return mStructuredName.Get() != nullptr; };

	DECLARE_TRACE();

private:
	ContactObject();

	WTF::String mID;
	WTF::String mDisplayName;
	WTF::String mName;
	WTF::String mNickName;
	WTF::String mPhoneNumber;
	WTF::String mEmails;
	WTF::String mAddress;
	WTF::Vector<WTF::String> mCategories;
	WTF::String mAccountName;
	WTF::String mAccountType;
	Member<ContactAddress> mStructuredAddress;
	Member<ContactName> mStructuredName;
};

}

#endif /* ContactObject_h */
