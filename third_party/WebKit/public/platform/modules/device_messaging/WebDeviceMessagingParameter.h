/*
 * WebDeviceMessagingParameter.h
 *
 *  Created on: 2015. 12. 22.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGINGPARAMETER_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGINGPARAMETER_H_

namespace blink {

struct WebDeviceMessageFindOptions;
class WebDeviceMessagingResultListener;
struct WebDeviceMessageObject;

struct WebDeviceMessagingParameter {
	WebDeviceMessagingParameter(WebDeviceMessagingResultListener* listener)
		: mFindOptions(nullptr),
		  mMessageObject(nullptr),
		  mListener(listener)
	{
	}
	~WebDeviceMessagingParameter()
	{
	}

	int mRequestId;
	WebDeviceMessageFindOptions* mFindOptions;
	WebDeviceMessageObject* mMessageObject;
	WebDeviceMessagingResultListener* mListener;
};

} // namespace blink

#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGINGPARAMETER_H_ */
