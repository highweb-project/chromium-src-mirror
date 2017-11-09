/*
 * Contact.h
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */

#ifndef Contact_h
#define Contact_h

#include <map>
#include <deque>
#include <vector>

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"
#include "platform/wtf/text/WTFString.h"

#include "modules/device_contact/ContactFindOptions.h"
#include "services/device/public/interfaces/contact_manager.mojom-blink.h"

namespace blink {

class LocalFrame;

class ContactAddress;
class ContactErrorCallback;
class ContactName;
class ContactObject;
class ContactSuccessCallback;

class ContactRequestHolder {
public:
	enum ContactOperation {
		FIND = 0,
		ADD = 1,
		DELETE = 2,
		UPDATE = 3
	};

	ContactRequestHolder(ContactOperation operation, int reuqestID);

	int mRequestID;
	ContactOperation mOperation;
};

typedef std::map<int, bool> ContactPermissionMap;
typedef std::deque<ContactRequestHolder*> RequestQueue;

typedef HashMap<int, ContactFindOptions> ContactFindOptionsMap;
typedef HeapHashMap<int, Member<ContactObject>> ContactObjectMap;
typedef HeapHashMap<int, Member<ContactSuccessCallback>> ContactSuccessCBMap;
typedef HeapHashMap<int, Member<ContactErrorCallback>> ContactErrorCBMap;

class Contact : public GarbageCollectedFinalized<Contact>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static Contact* create(LocalFrame& frame)
	{
		Contact* contact = new Contact(frame);
		return contact;
	}
	virtual ~Contact();

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
		kSuccess = 40
	};

	// JS API Implementations
	void findContact(ContactFindOptions findOptions, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback);
	void addContact(Member<ContactObject> contact, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback);
	void deleteContact(ContactFindOptions findOptions, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback);
	void updateContact(Member<ContactObject> contact, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback);

	// Internal Implementations
	void onPermissionChecked(PermissionResult result);

	DECLARE_TRACE();

private:
	Contact(LocalFrame& frame);

	void findContactInternal(int requestId);
	void addContactInternal(int requestId);
	void deleteContactInternal(int requestId);
	void updateContactInternal(int requestId);

	unsigned getContactFindTarget(WTF::String targetName);

	void OnContactResultsWithFindOption(int32_t requestId, uint32_t error, WTF::Optional<WTF::Vector<device::mojom::blink::ContactObjectPtr>> result);
	void OnContactResultsWithObject(int32_t requestId, uint32_t error, WTF::Optional<WTF::Vector<device::mojom::blink::ContactObjectPtr>> result);

	//change Mojo location
	ContactObject* convertMojoToScriptContact(device::mojom::blink::ContactObject* mojoContact);
	void convertScriptContactToMojo(device::mojom::blink::ContactObject* mojoContact, ContactObject* blinkContact);

	WebDeviceApiPermissionCheckClient* mClient;

	ContactPermissionMap mPermissionMap;

	RequestQueue mRequestQueue;

	ContactObjectMap mContactObjectMap;
	ContactFindOptionsMap mContactFindOptionsMap;
	ContactSuccessCBMap mContactSuccessCBMap;
	ContactErrorCBMap mContactErrorCBMap;

	device::mojom::blink::ContactManagerPtr contactManager;
};

}

#endif // Contact_h
