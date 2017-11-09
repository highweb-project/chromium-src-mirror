/*
 * ContactObject.h
 *
 *  Created on: 2015. 12. 11.
 *      Author: azureskybox
 */

#ifndef ContactAddress_h
#define ContactAddress_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ContactAddress : public GarbageCollectedFinalized<ContactAddress>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static ContactAddress* Create()
	{
		ContactAddress* contactAddress = new ContactAddress();
		return contactAddress;
	}
	virtual ~ContactAddress();

	// JS API Implementations
	WTF::String type() const { return mType; }
	void setType(const WTF::String type) { mType = type; };
	bool hasType() { return mType.length() > 0; };

	WTF::String streetAddress() const { return mStreetAddress; };
	void setStreetAddress(const WTF::String streetAddress) { mStreetAddress = streetAddress; };
	bool hasStreetAddress() { return mStreetAddress.length() > 0; };

	WTF::String locality() const { return mLocality; };
	void setLocality(const WTF::String locality) { mLocality = locality; };
	bool hasLocality() { return mLocality.length() > 0; };

	WTF::String region() const { return mRegion; };
	void setRegion(const WTF::String region) { mRegion = region; };
	bool hasRegion() { return mRegion.length() > 0; };

	WTF::String postalCode() const { return mPostalCode; };
	void setPostalCode(const WTF::String postalCode) { mPostalCode = postalCode; };
	bool hasPostalCode() { return mPostalCode.length() > 0; };

	WTF::String country() const { return mCountry; };
	void setCountry(const WTF::String country) { mCountry = country; };
	bool hasCountry() { return mCountry.length() > 0; };

	DECLARE_TRACE();
private:
	ContactAddress();

	WTF::String mType;
	WTF::String mStreetAddress;
	WTF::String mLocality;
	WTF::String mRegion;
	WTF::String mPostalCode;
	WTF::String mCountry;
};

}

#endif /* ContactAddress_h */
