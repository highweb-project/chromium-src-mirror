/*
 * WebDeviceMessageResultListener.h
 *
 *  Created on: 2015. 12. 22.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGINGRESULTLISTENER_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGINGRESULTLISTENER_H_

#include "third_party/WebKit/public/platform/modules/device_messaging/WebDeviceMessageObject.h"

namespace blink {

class WebDeviceMessagingResultListener {
public:
	virtual void OnMessageFindResult(int requestId, unsigned error, std::vector<WebDeviceMessageObject*> results) = 0;
	virtual void OnMessageReceived(int observerId, unsigned error, std::vector<WebDeviceMessageObject*> results) = 0;
};

} // namespace blink

#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGINGRESULTLISTENER_H_ */
