/*
 * WebDeviceGalleryStruct.h
 *
 *  Created on: 2016. 01. 26.
 *      Author: pjs3232
 */

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICEGALLERYSTRUCT_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICEGALLERYSTRUCT_H_

#include "third_party/WebKit/public/platform/WebString.h"
#include "base/logging.h"

namespace blink {

struct WebDeviceGalleryMediaContent {
	WebDeviceGalleryMediaContent()
		: mUri(""), mBlobSize(0)
	{
		mBlob = NULL;
	}
	~WebDeviceGalleryMediaContent() {
		if (mBlob != NULL) {
			delete[] mBlob;
			mBlob = NULL;
		}
	}
	WebString mUri;
	unsigned mBlobSize;
	uint8_t* mBlob;
};

struct WebDeviceGalleryMediaObject {
	WebDeviceGalleryMediaObject()
		: mType(""),
		  mDescription(""),
		  mId(""),
		  mTitle(""),
		  mFileName(""),
		  mFileSize(0),
		  mCreatedDate(0)
	{
		mContent = NULL;
	}
	~WebDeviceGalleryMediaObject() {
		if (mContent != NULL) {
			delete mContent;
			mContent = NULL;
		}
	}
	WebString mType;
	WebString mDescription;
	WebString mId;
	WebString mTitle;
	WebString mFileName;
	unsigned long mFileSize;
	double mCreatedDate;
	WebDeviceGalleryMediaContent* mContent;
};

struct WebDeviceGalleryFindOptions {
	WebDeviceGalleryFindOptions()
	{
	}
	WebString mOperation;
	int mMaxItem;
	WebDeviceGalleryMediaObject mObject;
};

}

#endif /* THIRD_PARTY_WEBKIT_PUBLIC_PLATFORM_MODULES_DEVICE_CONTACT_WEBDEVICEGALLERYSTRUCT_H_ */
