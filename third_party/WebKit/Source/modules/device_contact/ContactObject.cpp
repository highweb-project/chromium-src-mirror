/*
 * ContactObject.cpp
 *
 *  Created on: 2015. 12. 11.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_contact/ContactObject.h"

#include "base/logging.h"

#include "modules/device_contact/ContactAddress.h"
#include "modules/device_contact/ContactName.h"

namespace blink {

ContactObject::ContactObject()
	: mID(""),
	  mDisplayName(""),
	  mName(""),
	  mNickName(""),
	  mPhoneNumber(""),
	  mEmails(""),
	  mAddress(""),
	  mAccountName(""),
	  mAccountType(""),
	  mStructuredAddress(nullptr),
	  mStructuredName(nullptr)
{
	DLOG(INFO) << "ContactObject::ContactObject";
}

ContactObject::~ContactObject()
{

}

ContactAddress* ContactObject::structuredAddress()
{
	return mStructuredAddress.Get();
}

void ContactObject::setStructuredAddress(ContactAddress* structuredAddress)
{
	mStructuredAddress = structuredAddress;
}

ContactName* ContactObject::structuredName()
{
	return mStructuredName.Get();
}

void ContactObject::setStructuredName(ContactName* structuredName)
{
	mStructuredName = structuredName;
}

DEFINE_TRACE(ContactObject) {
	visitor->Trace(mStructuredAddress);
	visitor->Trace(mStructuredName);
}

}


