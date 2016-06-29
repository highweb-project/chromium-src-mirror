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

#include "core/dom/ActiveDOMObject.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"
#include "public/platform/modules/device_contact/WebDeviceContactResultListener.h"
#include "wtf/text/WTFString.h"

#include "modules/device_contact/ContactFindOptions.h"

namespace blink {

class LocalFrame;

struct WebDeviceContactAddress;
struct WebDeviceContactName;
struct WebDeviceContactObject;

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

class Contact : public GarbageCollectedFinalized<Contact>, public ScriptWrappable, public WebDeviceContactResultListener {
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
		UNKNOWN_ERROR = 0,
		INVALID_ARGUMNET_ERROR = 1,
		TIMEOUT_ERROR = 3,
		PENDING_OPERATION_ERROR = 4,
		IO_ERROR = 5,
		NOT_SUPPORTED_ERROR = 6,
		PERMISSION_DENIED_ERROR = 20,
		MESSAGE_SIZE_EXCEEDED = 30,
		SUCCESS = 40
	};

	// JS API Implementations
	void findContact(ContactFindOptions findOptions, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback);
	void addContact(Member<ContactObject> contact, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback);
	void deleteContact(ContactFindOptions findOptions, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback);
	void updateContact(Member<ContactObject> contact, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback);

	// Internal Implementations
	void onPermissionChecked(PermissionResult result);

	// WebDeviceContactResultListener implements
	void OnContactFindResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results) override;
	void OnContactAddResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results) override;
	void OnContactDeleteResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results) override;
	void OnContactUpdateResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results) override;

	DECLARE_TRACE();

private:
	Contact(LocalFrame& frame);

	void findContactInternal(int requestId);
	void addContactInternal(int requestId);
	void deleteContactInternal(int requestId);
	void updateContactInternal(int requestId);

	unsigned getContactFindTarget(WTF::String targetName);

	WebDeviceContactObject* convertToBlinkContact(ContactObject* scriptContact);
	ContactObject* convertToScriptContact(WebDeviceContactObject* blinkContact);

	WebDeviceApiPermissionCheckClient* mClient;

	ContactPermissionMap mPermissionMap;

	RequestQueue mRequestQueue;

	ContactObjectMap mContactObjectMap;
	ContactFindOptionsMap mContactFindOptionsMap;
	ContactSuccessCBMap mContactSuccessCBMap;
	ContactErrorCBMap mContactErrorCBMap;
};

}

#endif // Contact_h
