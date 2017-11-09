/*
 * MessagingEvent.cpp
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_messaging/MessagingEvent.h"

#include "modules/device_messaging/MessageObject.h"

namespace blink {

MessagingEvent::~MessagingEvent()
{
}

MessagingEvent::MessagingEvent()
	: Event(EventTypeNames::messagereceived, true, false)
{
    mID = "";
	mType = "";
	mTo = "";
	mFrom = "";
	mTitle = "";
	mBody = "";
	mDate = "";
}

MessagingEvent::MessagingEvent(const AtomicString& eventType)
    : Event(eventType, true, false)
{
}

const AtomicString& MessagingEvent::InterfaceName() const
{
	return EventNames::MessagingEvent;
}

MessageObject* MessagingEvent::value()
{
	MessageObject* message = MessageObject::Create();
	message->setId(mID);
	message->setType(mType);
	message->setTo(mTo);
	message->setFrom(mFrom);
	message->setTitle(mTitle);
	message->setBody(mBody);
	message->setDate(mDate);

	return message;
}

}



