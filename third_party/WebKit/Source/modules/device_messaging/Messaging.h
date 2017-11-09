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

#include "platform/bindings/ActiveScriptWrappable.h"
#include "core/events/EventTarget.h"
#include "platform/heap/Handle.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"
#include "platform/wtf/text/WTFString.h"
#include "core/dom/ContextLifecycleObserver.h"
#include "core/dom/SuspendableObject.h"

#include "modules/device_messaging/MessageFindOptions.h"

#include "services/device/public/interfaces/messaging_manager.mojom-blink.h"

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
		:
		public EventTargetWithInlineData,
		public ActiveScriptWrappable<Messaging>,
		public SuspendableObject
		 {
			 DEFINE_WRAPPERTYPEINFO();
    USING_GARBAGE_COLLECTED_MIXIN(Messaging);

public:
	static Messaging* create(LocalFrame& frame)
	{
		Messaging* messaging = new Messaging(frame);
		return messaging;
	}
	virtual ~Messaging();

	enum PermissionType{
		VIEW = 0,
		MODIFY = 1
	};

	enum {
		kUnknownError = 0,
		kInvalidArgumnetError = 1,
		kTimeoutError = 3,
		kPendingOperationError = 4,
		kIoError = 5,
		kNotSupportedError = 6,
		kPermissionDeniedError = 20,
		kMessageSizeExceeded = 30,
		kSuccess = 40,
		kNotSupportApi = 9999,
	};

    // EventTarget implementation.
    const AtomicString& InterfaceName() const override;
    ExecutionContext* GetExecutionContext() const override;

    bool HasPendingActivity() const override;
    void Suspend() override;
    void Resume() override;
    void ContextDestroyed(ExecutionContext*) override;

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

	void convertBlinkMessageToMojo(MessageObject* scriptMessage, device::mojom::blink::MessageObject* mojoObject);
	MessageObject* convertMojoToScriptMessage(device::mojom::blink::MessageObject* mojoObject);

	void findMessageInternal(int requestId);
	// void sendMessageInternal(int requestId);
	void findMessageResult(uint32_t requestID, uint32_t error, WTF::Optional<WTF::Vector<device::mojom::blink::MessageObjectPtr>> results);
	void messageReceivedCallback(uint32_t observerId, device::mojom::blink::MessageObjectPtr message);

	device::mojom::blink::MessagingManagerPtr messageManager;

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
