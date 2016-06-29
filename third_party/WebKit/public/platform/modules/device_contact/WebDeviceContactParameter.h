/*
 * WebDeviceContactParameter.h
 *
 *  Created on: 2015. 12. 9.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTPARAMETER_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTPARAMETER_H_

namespace blink {

struct WebDeviceContactFindOptions;
class WebDeviceContactResultListener;
struct WebDeviceContactObject;

struct WebDeviceContactParameter {
	WebDeviceContactParameter(WebDeviceContactResultListener* listener)
		: mFindOptions(nullptr),
		  mContactObject(nullptr),
		  mListener(listener)
	{
	}

	int mRequestId;
	WebDeviceContactFindOptions* mFindOptions;
	WebDeviceContactObject* mContactObject;
	WebDeviceContactResultListener* mListener;
};

} // namespace blink

#endif  // THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTPARAMETER_H_ */
