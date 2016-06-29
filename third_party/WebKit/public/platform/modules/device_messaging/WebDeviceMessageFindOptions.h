/*
 * WebDeviceMessageFindOptions.h
 *
 *  Created on: 2015. 12. 22.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGEFINDOPTIONS_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGEFINDOPTIONS_H_

#include "third_party/WebKit/public/platform/WebString.h"

namespace blink {

struct WebDeviceMessageFindOptions {

	WebDeviceMessageFindOptions(
			unsigned target,
			unsigned maxItem,
			WebString condition
	)
		: mTarget(target),
		  mMaxItem(maxItem),
		  mCondition(condition)
	{
	}

	unsigned mTarget;
	unsigned mMaxItem;
	WebString mCondition;
};

}



#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGEFINDOPTIONS_H_ */
