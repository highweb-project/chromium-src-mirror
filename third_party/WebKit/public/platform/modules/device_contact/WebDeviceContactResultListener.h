/*
 * WebDeviceContactResultListener.h
 *
 *  Created on: 2015. 12. 10.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTRESULTLISTENER_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTRESULTLISTENER_H_

#include "third_party/WebKit/public/platform/modules/device_contact/WebDeviceContactFindOptions.h"
#include "third_party/WebKit/public/platform/modules/device_contact/WebDeviceContactObject.h"

namespace blink {

class WebDeviceContactResultListener {
public:
	virtual void OnContactFindResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results) = 0;
	virtual void OnContactAddResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results) = 0;
	virtual void OnContactDeleteResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results) = 0;
	virtual void OnContactUpdateResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results) = 0;
};

} // namespace blink



#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICECONTACTRESULTLISTENER_H_ */
