/*
 * ContactObject.h
 *
 *  Created on: 2015. 12. 11.
 *      Author: azureskybox
 */

#ifndef ContactName_h
#define ContactName_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ContactName : public GarbageCollectedFinalized<ContactName>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static ContactName* Create()
	{
		ContactName* contactName = new ContactName();
		return contactName;
	}
	virtual ~ContactName();

	// JS API Implementations
	WTF::String familyName() const { return mFamilyName; }
	void setFamilyName(const WTF::String familyName) { mFamilyName = familyName; };
	bool hasFamilyName() { return mFamilyName.length() > 0; };

	WTF::String givenName() const { return mGivenName; };
	void setGivenName(const WTF::String givenName) { mGivenName = givenName; };
	bool hasGivenName() { return mGivenName.length() > 0; };

	WTF::String middleName() const { return mMiddleName; };
	void setMiddleName(const WTF::String middleName) { mMiddleName = middleName; };
	bool hasMiddleName() { return mMiddleName.length() > 0; };

	WTF::String prefix() const { return mPrefix; };
	void setPrefix(const WTF::String prefix) { mPrefix = prefix; };
	bool hasPrefix() { return mPrefix.length() > 0; };

	WTF::String suffix() const { return mSuffix; };
	void setSuffix(const WTF::String suffix) { mSuffix = suffix; };
	bool hasSuffix() { return mSuffix.length() > 0; };

	DECLARE_TRACE();
private:
	ContactName();

	WTF::String mFamilyName;
	WTF::String mGivenName;
	WTF::String mMiddleName;
	WTF::String mPrefix;
	WTF::String mSuffix;
};

}

#endif /* ContactName_h */
