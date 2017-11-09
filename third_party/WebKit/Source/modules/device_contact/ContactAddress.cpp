/*
 * ContactObject.cpp
 *
 *  Created on: 2015. 12. 11.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_contact/ContactAddress.h"

namespace blink {

ContactAddress::ContactAddress()
	: mType(""),
	  mStreetAddress(""),
	  mLocality(""),
	  mRegion(""),
	  mPostalCode(""),
	  mCountry("")
{
}

ContactAddress::~ContactAddress()
{

}

DEFINE_TRACE(ContactAddress) {
}

}


