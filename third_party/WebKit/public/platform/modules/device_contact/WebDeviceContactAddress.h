/*
 * WebDeviceContactAddress.h
 *
 *  Created on: 2015. 12. 15.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTADDRESS_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTADDRESS_H_

#include "third_party/WebKit/public/platform/WebString.h"

namespace blink {

struct WebDeviceContactAddress {
	WebDeviceContactAddress()
		: mType(""),
		  mStreetAddress(""),
		  mLocality(""),
		  mRegion(""),
		  mPostalcode(""),
		  mCountry("")
	{
	}

	WebString mType;
	WebString mStreetAddress;
	WebString mLocality;
	WebString mRegion;
	WebString mPostalcode;
	WebString mCountry;
};

}

#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTADDRESS_H_ */
