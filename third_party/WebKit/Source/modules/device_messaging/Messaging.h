/*
 * Messaging.h
 *
 *  Created on: 2015. 12. 21.
 *      Author: azureskybox
 */

#ifndef Messaging_h
#define Messaging_h

#include <map>
#include <deque>
#include <vector>

#include "bindings/core/v8/ScriptWrappable.h"
#include "core/dom/ActiveDOMObject.h"
#include "core/frame/DOMWindowLifecycleObserver.h"
#include "core/events/EventTarget.h"
#include "platform/heap/Handle.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"
#include "wtf/text/WTFString.h"
#include "core/dom/ActiveDOMObject.h"
#include "core/dom/ContextLifecycleObserver.h"

#include "modules/device_messaging/MessageFindOptions.h"

#include "device/messaging/messaging_manager.mojom-blink.h"

namespace blink {

class LocalFrame;

class MessageObject;
class MessagingErrorCallback;
class MessagingSuccessCallback;

struct WebDeviceMessageObject;

struct MessagingRequestHolder {
public:
	enum MessagingOperation {
		SEND = 0,
		FIND = 1,
		OBSERVE = 2
	};

	MessagingRequestHolder(MessagingOperation operation, int requestID)
	{
		mRequestID = requestID;
		mOperation = operation;
	}

	int mRequestID;
	MessagingOperation mOperation;
};

typedef std::map<int, bool> MessagingPermissionMap;
typedef std::deque<MessagingRequestHolder*> MessagingRequestQueue;
typedef HashMap<int, MessageFindOptions> MessageFindOptionsMap;
typedef HeapHashMap<int, Member<MessageObject>> MessageObjectMap;
typedef HeapHashMap<int, Member<MessagingSuccessCallback>> MessagingSuccessCBMap;
typedef HeapHashMap<int, Member<MessagingErrorCallback>> MessagingErrorCBMap;

class Messaging
		: public EventTargetWithInlineData
		, public ActiveScriptWrappable
		, public ActiveDOMObject {
    USING_GARBAGE_COLLECTED_MIXIN(Messaging);
	DEFINE_WRAPPERTYPEINFO();
public:
	static Messaging* create(LocalFrame& frame)
	{
		Messaging* messaging = new Messaging(frame);
		return messaging;
	}
	virtual ~Messaging() override;

	enum PermissionType{
		VIEW = 0,
		MODIFY = 1
	};

	enum {
		UNKNOWN_ERROR = 0,
		INVALID_ARGUMNET_ERROR = 1,
		TIMEOUT_ERROR = 3,
		PENDING_OPERATION_ERROR = 4,
		IO_ERROR = 5,
		NOT_SUPPORTED_ERROR = 6,
		PERMISSION_DENIED_ERROR = 20,
		MESSAGE_SIZE_EXCEEDED = 30,
		SUCCESS = 40,
		NOT_SUPPORT_API = 9999,
	};

    // EventTarget implementation.
    const AtomicString& interfaceName() const override;
    ExecutionContext* getExecutionContext() const override;

    // ActiveDOMObject implementation.
    bool hasPendingActivity() const override;
    void suspend() override;
    void resume() override;
    void stop() override;

	// JS API Implementations
	void sendMessage(Member<MessageObject> message);
	void findMessage(MessageFindOptions findOptions, MessagingSuccessCallback* successCallback, MessagingErrorCallback* errorCallback);

	// Internal Implementations
	void onPermissionChecked(PermissionResult result);

    EventListener* onmessagereceived();
    void setOnmessagereceived(EventListener* listener);

	DECLARE_VIRTUAL_TRACE();
private:
	Messaging(LocalFrame& frame);

	void convertBlinkMessageToMojo(MessageObject* scriptMessage, device::blink::MessageObject* mojoObject);
	MessageObject* convertMojoToScriptMessage(device::blink::MessageObject* mojoObject);

	void findMessageInternal(int requestId);
	// void sendMessageInternal(int requestId);
	void findMessageResult(uint32_t requestID, uint32_t error, mojo::WTFArray<device::blink::MessageObjectPtr> results);
	void messageReceivedCallback(uint32_t observerId, device::blink::MessageObjectPtr message);

	device::blink::MessagingManagerPtr messageManager;

	unsigned getMessageFindTarget(WTF::String targetName);

	int mObserverID;
	bool mIsListening;

	WebDeviceApiPermissionCheckClient* mClient;

	MessagingPermissionMap mPermissionMap;

	MessagingRequestQueue mRequestQueue;

	MessageFindOptionsMap mMessageFindOptionsMap;
	MessagingSuccessCBMap mMessagingSuccessCBMap;
	MessagingErrorCBMap mMessagingErrorCBMap;
};

}

#endif // Messaging_h
