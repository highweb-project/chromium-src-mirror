/*
 * WebDeviceMessageObject.h
 *
 *  Created on: 2015. 12. 22.
 *      Author: azureskybox
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGEOBJECT_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGEOBJECT_H_

#include "third_party/WebKit/public/platform/WebString.h"

namespace blink {

struct WebDeviceMessageObject {

	WebDeviceMessageObject()
		: mId(""),
		  mType(""),
		  mTo(""),
		  mFrom(""),
		  mTitle(""),
		  mBody(""),
		  mDate("")
	{
	}

	WebDeviceMessageObject(
			WebString id,
			WebString type,
			WebString to,
			WebString from,
			WebString title,
			WebString body,
			WebString date
	)
		: mId(id),
		  mType(type),
		  mTo(to),
		  mFrom(from),
		  mTitle(title),
		  mBody(body),
		  mDate(date)
	{
	}

	WebString mId;
	WebString mType;
	WebString mTo;
	WebString mFrom;
	WebString mTitle;
	WebString mBody;
	WebString mDate;
};

}

#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_MESSAGING_WEBDEVICEMESSAGEOBJECT_H_ */
