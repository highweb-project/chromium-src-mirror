/*
 * WebDeviceContactObject.h
 *
 *  Created on: 2015. 12. 9.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTOBJECT_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTOBJECT_H_

#include <vector>

#include "third_party/WebKit/public/platform/WebString.h"

#include "third_party/WebKit/public/platform/modules/device_contact/WebDeviceContactAddress.h"
#include "third_party/WebKit/public/platform/modules/device_contact/WebDeviceContactName.h"

namespace blink {

struct WebDeviceContactObject {

	WebDeviceContactObject()
		: mId(""),
		  mDisplayName(""),
		  mPhoneNumber(""),
		  mEmails(""),
		  mAddress(""),
		  mAccountName(""),
		  mAccountType("")
	{
	}

	WebDeviceContactObject(
			WebString id,
			WebString displayName,
			WebString phoneNumber,
			WebString emails,
			WebString address,
			WebString accountName,
			WebString accountType
	)
		: mId(id),
		  mDisplayName(displayName),
		  mPhoneNumber(phoneNumber),
		  mEmails(emails),
		  mAddress(address),
		  mAccountName(accountName),
		  mAccountType(accountType)
	{
	}

	WebString mId;
	WebString mDisplayName;
	WebString mPhoneNumber;
	WebString mEmails;
	WebString mAddress;
	std::vector<WebString> mCategories;
	WebString mAccountName;
	WebString mAccountType;
	WebDeviceContactName mStructuredName;
	WebDeviceContactAddress mStructuredAddress;
};

}

#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTOBJECT_H_ */
