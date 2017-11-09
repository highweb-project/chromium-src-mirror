#ifndef MessagingEvent_h
#define MessagingEvent_h

#include "modules/EventModules.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class MessageObject;

class MessagingEvent final : public Event {
	DEFINE_WRAPPERTYPEINFO();
public:
	~MessagingEvent() override;

	static MessagingEvent* create() {
		return new MessagingEvent();
	}
	static MessagingEvent* create(const AtomicString& eventType)
	{
		return new MessagingEvent(eventType);
	}

	MessageObject* value();

	const AtomicString& InterfaceName() const override;

	void setID(WTF::String id) 		{ mID = id; };
	void setType(WTF::String type) 	{ mType = type; };
	void setTo(WTF::String to) 		{ mTo = to; };
	void setFrom(WTF::String from) 	{ mFrom = from; };
	void setTitle(WTF::String title) 	{ mTitle = title; };
	void setBody(WTF::String body) 	{ mBody = body; };
	void setDate(WTF::String date) 	{ mDate = date; };

private:
	MessagingEvent();
	MessagingEvent(const AtomicString& eventType);

	WTF::String mID;
	WTF::String mType;
	WTF::String mTo;
	WTF::String mFrom;
	WTF::String mTitle;
	WTF::String mBody;
	WTF::String mDate;
};

DEFINE_TYPE_CASTS(MessagingEvent, Event, event, event->InterfaceName() == EventNames::MessagingEvent, event.InterfaceName() == EventNames::MessagingEvent);

}

#endif // MessagingEvent_h
