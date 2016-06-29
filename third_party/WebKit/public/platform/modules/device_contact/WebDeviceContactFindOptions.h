/*
 * WebDeviceContactFindOptions.h
 *
 *  Created on: 2015. 12. 9.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTFINDOPTIONS_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTFINDOPTIONS_H_

#include <vector>

#include "third_party/WebKit/public/platform/WebString.h"

namespace blink {

struct WebDeviceContactObject;
class WebDeviceContactResultListener;

struct WebDeviceContactFindOptions {
	WebDeviceContactFindOptions(unsigned target, unsigned maxItem, WebString condition)
		: mTarget(target),
		  mMaxItem(maxItem),
		  mCondition(condition)
	{
	}

	unsigned mTarget;
	unsigned mMaxItem;
	WebString mCondition;
};

} // namespace blink

#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTFINDOPTIONS_H_ */
