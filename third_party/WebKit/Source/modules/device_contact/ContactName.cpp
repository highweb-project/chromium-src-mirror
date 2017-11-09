/*
 * ContactObject.cpp
 *
 *  Created on: 2015. 12. 11.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_contact/ContactName.h"

namespace blink {

ContactName::ContactName()
	: mFamilyName(""),
	  mGivenName(""),
	  mMiddleName(""),
	  mPrefix(""),
	  mSuffix("")
{
}

ContactName::~ContactName()
{

}

DEFINE_TRACE(ContactName) {
}

}


