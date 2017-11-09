/*
 * MessageObject.h
 *
 *  Created on: 2015. 12. 21.
 *      Author: azureskybox
 */

#ifndef MessageObject_h
#define MessageObject_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ContactAddress;
class ContactName;

class MessageObject : public GarbageCollectedFinalized<MessageObject>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static MessageObject* Create()
	{
		MessageObject* messageObject = new MessageObject();
		return messageObject;
	}
	virtual ~MessageObject();

	// JS API Implementations
	WTF::String id() { return mID; };
	void setId(WTF::String id) { mID = id; };
	bool hasId() { return mID.length() > 0; };

	WTF::String type() const { return mType; }
	void setType(const WTF::String type) { mType = type; };
	bool hasType() { return mType.length() > 0; };

	WTF::String to() { return mTo; };
	void setTo(WTF::String to) { mTo = to; };
	bool hasTo() { return mTo.length() > 0; };

	WTF::String from() { return mFrom; };
	void setFrom(WTF::String from) { mFrom = from; };
	bool hasFrom() { return mFrom.length() > 0; };

	WTF::String title() { return mTitle; };
	void setTitle(WTF::String title) { mTitle = title; };
	bool hasTitle() { return mTitle.length() > 0; };

	WTF::String body() { return mBody; };
	void setBody(WTF::String body) { mBody = body; };
	bool hasBody() { return mBody.length() > 0; };

	WTF::String date() { return mDate; };
	void setDate(WTF::String date) { mDate = date; };
	bool hasDate() { return mDate.length() > 0; };


	DECLARE_TRACE();

private:
	MessageObject();

	WTF::String mID;
	WTF::String mType;
	WTF::String mTo;
	WTF::String mFrom;
	WTF::String mTitle;
	WTF::String mBody;
	WTF::String mDate;
};

}



#endif // MessageObject_h */
