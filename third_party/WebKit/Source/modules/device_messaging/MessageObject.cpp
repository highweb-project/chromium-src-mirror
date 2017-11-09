/*
 * ContactObject.cpp
 *
 *  Created on: 2015. 12. 11.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_messaging/MessageObject.h"

#include "base/logging.h"

namespace blink {

MessageObject::MessageObject()
	: mID(""),
	  mType(""),
	  mTo(""),
	  mFrom(""),
	  mTitle(""),
	  mBody(""),
	  mDate("")
{
	DLOG(INFO) << "MessageObject::MessageObject";
}

MessageObject::~MessageObject()
{

}

DEFINE_TRACE(MessageObject) {
}

}


