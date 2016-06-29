/*
 * WebDeviceContactName.h
 *
 *  Created on: 2015. 12. 15.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTNAME_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTNAME_H_

#include "third_party/WebKit/public/platform/WebString.h"

namespace blink {

struct WebDeviceContactName {
	WebDeviceContactName()
		: mFamilyName(""),
		  mGivenName(""),
		  mMiddleName(""),
		  mHonorificPrefix(""),
		  mHonorificSuffix("")
	{
	}

	WebString mFamilyName;
	WebString mGivenName;
	WebString mMiddleName;
	WebString mHonorificPrefix;
	WebString mHonorificSuffix;
};

}

#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTNAME_H_ */
