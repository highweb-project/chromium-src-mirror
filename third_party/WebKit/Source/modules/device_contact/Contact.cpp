/*
 * Contact.cpp
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */

#include "platform/wtf/build_config.h"
#include "modules/device_contact/Contact.h"

// #include "base/basictypes.h"
#include "base/bind.h"
#include "base/callback_forward.h"
#include "base/logging.h"
#include "base/rand_util.h"
#include "core/dom/Document.h"
#include "platform/wtf/text/WTFString.h"

#include "public/platform/Platform.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "modules/device_contact/ContactAddress.h"
#include "modules/device_contact/ContactErrorCallback.h"
#include "modules/device_contact/ContactName.h"
#include "modules/device_contact/ContactObject.h"
#include "modules/device_contact/ContactSuccessCallback.h"

#include "platform/mojo/MojoHelper.h"
#include "services/device/public/interfaces/constants.mojom-blink.h"
#include "services/service_manager/public/cpp/connector.h"

namespace blink {

ContactRequestHolder::ContactRequestHolder(ContactOperation operation, int reuqestID)
	: mRequestID(reuqestID),
	  mOperation(operation)
{
}

Contact::Contact(LocalFrame& frame)
	: mClient(DeviceApiPermissionController::From(frame)->client())
{
	WTF::String origin = frame.GetDocument()->Url().StrippedForUseAsReferrer();
	mClient->SetOrigin(origin.Latin1().data());

	mPermissionMap[(int)PermissionType::MODIFY] = false;
	mPermissionMap[(int)PermissionType::VIEW] = false;
}

Contact::~Contact()
{
	if (contactManager.is_bound()) {
		contactManager.reset();
	}
}

void Contact::onPermissionChecked(PermissionResult result)
{
	DLOG(INFO) << "Contact::onPermissionChecked, result=" << result;

	if(mRequestQueue.size()) {
		ContactRequestHolder* holder = mRequestQueue.front();

		if(result != PermissionResult::RESULT_OK) {
			ContactErrorCallback* errorCallback = mContactErrorCBMap.at(holder->mRequestID);
			if(errorCallback != nullptr) {
				errorCallback->handleResult((unsigned)Contact::kPermissionDeniedError);
			}

			delete holder;
			mRequestQueue.pop_front();

			return;
		}
		else {
			if(holder->mOperation == ContactRequestHolder::ContactOperation::FIND) {
				mPermissionMap[(int)PermissionType::VIEW] = false;
			}
			else {
				mPermissionMap[(int)PermissionType::MODIFY] = false;
			}
		}

		if(holder->mOperation == ContactRequestHolder::ContactOperation::FIND) {
			findContactInternal(holder->mRequestID);
		}
		else if(holder->mOperation == ContactRequestHolder::ContactOperation::ADD) {
			addContactInternal(holder->mRequestID);
		}
		else if(holder->mOperation == ContactRequestHolder::ContactOperation::DELETE) {
			deleteContactInternal(holder->mRequestID);
		}
		else if(holder->mOperation == ContactRequestHolder::ContactOperation::UPDATE) {
			updateContactInternal(holder->mRequestID);
		}

		delete holder;
		mRequestQueue.pop_front();
	}
}

// findContact -------------------------------------------

void Contact::findContact(ContactFindOptions findOptions, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback)
{
	DLOG(INFO) << "Contact::findContact";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY] || mPermissionMap[(int)PermissionType::VIEW];
	DLOG(INFO) << ">> find operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);
	mContactFindOptionsMap.Set(requestId, findOptions);
	mContactSuccessCBMap.Set(requestId, successCallback);
	mContactErrorCBMap.Set(requestId, errorCallback);

	if(!isPermitted) {
		ContactRequestHolder* holder = new ContactRequestHolder(
				ContactRequestHolder::ContactOperation::FIND,
				requestId
		);
		mRequestQueue.push_back(holder);

		if(mClient) {
			mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
					PermissionAPIType::CONTACT,
					PermissionOptType::VIEW,
					base::Bind(&Contact::onPermissionChecked, base::Unretained(this))));
		}

		return;
	}

	findContactInternal(requestId);
}

void Contact::findContactInternal(int requestId)
{
	DLOG(INFO) << "Contact::findContactInternal";
	if (!contactManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&contactManager));
	}

	ContactFindOptions findOptions = mContactFindOptionsMap.at(requestId);
	contactManager->FindContact(requestId, getContactFindTarget(findOptions.target()),
		findOptions.maxItem(), findOptions.condition(),
		ConvertToBaseCallback(WTF::Bind(&Contact::OnContactResultsWithFindOption, WrapPersistent(this))));
}

// findContact -------------------------------------------

// addContact --------------------------------------------

void Contact::addContact(Member<ContactObject> contact, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback)
{
	DLOG(INFO) << "Contact::addContact";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY];
	DLOG(INFO) << ">> add operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);

	mContactObjectMap.Set(requestId, contact.Get());
	mContactSuccessCBMap.Set(requestId, successCallback);
	mContactErrorCBMap.Set(requestId, errorCallback);

	if(!isPermitted) {
		ContactRequestHolder* holder = new ContactRequestHolder(
				ContactRequestHolder::ContactOperation::ADD,
				requestId
		);

		mRequestQueue.push_back(holder);

		if(mClient) {
			mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
					PermissionAPIType::CONTACT,
					PermissionOptType::MODIFY,
					base::Bind(&Contact::onPermissionChecked, base::Unretained(this))));
		}

		return;
	}

	addContactInternal(requestId);
}

void Contact::addContactInternal(int requestId)
{
	if (!contactManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&contactManager));
	}

	ContactObject* object = mContactObjectMap.at(requestId);
	device::mojom::blink::ContactObjectPtr parameterPtr(device::mojom::blink::ContactObject::New());
	convertScriptContactToMojo(parameterPtr.get(), object);
	contactManager->AddContact(requestId, std::move(parameterPtr),
	ConvertToBaseCallback(WTF::Bind(&Contact::OnContactResultsWithObject, WrapPersistent(this))));
}

// addContact --------------------------------------------

// deleteContact --------------------------------------------

void Contact::deleteContact(ContactFindOptions findOptions, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback)
{
	DLOG(INFO) << "Contact::deleteContact";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY];
	DLOG(INFO) << ">> find operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);

	mContactFindOptionsMap.Set(requestId, findOptions);
	mContactSuccessCBMap.Set(requestId, successCallback);
	mContactErrorCBMap.Set(requestId, errorCallback);

	if(!isPermitted) {
		ContactRequestHolder* holder = new ContactRequestHolder(
				ContactRequestHolder::ContactOperation::DELETE,
				requestId
		);
		mRequestQueue.push_back(holder);

		if(mClient) {
			mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
					PermissionAPIType::CONTACT,
					PermissionOptType::MODIFY,
					base::Bind(&Contact::onPermissionChecked, base::Unretained(this))));
		}

		return;
	}

	deleteContactInternal(requestId);
}

void Contact::deleteContactInternal(int requestId)
{
	DLOG(INFO) << "Contact::deleteContactInternal";
	if (!contactManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&contactManager));
	}
	ContactFindOptions findOptions = mContactFindOptionsMap.at(requestId);
	contactManager->DeleteContact(requestId, getContactFindTarget(findOptions.target()),
		findOptions.maxItem(), findOptions.condition(),
		ConvertToBaseCallback(WTF::Bind(&Contact::OnContactResultsWithFindOption, WrapPersistent(this))));
}

// deleteContact --------------------------------------------

// updateContact --------------------------------------------

void Contact::updateContact(Member<ContactObject> contact, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback)
{
	DLOG(INFO) << "Contact::updateContact";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY];
	DLOG(INFO) << ">> update operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);

	mContactObjectMap.Set(requestId, contact.Get());
	mContactSuccessCBMap.Set(requestId, successCallback);
	mContactErrorCBMap.Set(requestId, errorCallback);

	if(!isPermitted) {
		ContactRequestHolder* holder = new ContactRequestHolder(
				ContactRequestHolder::ContactOperation::UPDATE,
				requestId
		);

		mRequestQueue.push_back(holder);

		if(mClient) {
			mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
					PermissionAPIType::CONTACT,
					PermissionOptType::MODIFY,
					base::Bind(&Contact::onPermissionChecked, base::Unretained(this))));
		}

		return;
	}

	updateContactInternal(requestId);
}

void Contact::updateContactInternal(int requestId)
{
	if (!contactManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&contactManager));
	}

	ContactObject* object = mContactObjectMap.at(requestId);
	device::mojom::blink::ContactObjectPtr parameterPtr(device::mojom::blink::ContactObject::New());
	convertScriptContactToMojo(parameterPtr.get(), object);
	contactManager->UpdateContact(requestId, std::move(parameterPtr),
	ConvertToBaseCallback(WTF::Bind(&Contact::OnContactResultsWithObject, WrapPersistent(this))));
}

// updateContact --------------------------------------------

void Contact::OnContactResultsWithObject(int32_t requestId, uint32_t error, WTF::Optional<WTF::Vector<device::mojom::blink::ContactObjectPtr>> result) {
	DLOG(INFO) << "Contact::OnContactResultsWithObject, result =" << result.has_value();

	ContactSuccessCallback* successCB = mContactSuccessCBMap.at(requestId);
	ContactErrorCallback* errorCB = mContactErrorCBMap.at(requestId);

	if(error != Contact::kSuccess) {
		errorCB->handleResult((unsigned)error);
		return;
	} else if (result.has_value()){
		HeapVector<Member<ContactObject>> resultToReturn;
		ContactObject* tmpObj = nullptr;
		size_t resultSize = result.value().size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertMojoToScriptContact(result.value()[i].get());
			resultToReturn.push_back(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mContactObjectMap.erase(requestId);
	mContactSuccessCBMap.erase(requestId);
	mContactErrorCBMap.erase(requestId);
}

void Contact::OnContactResultsWithFindOption(int32_t requestId, uint32_t error, WTF::Optional<WTF::Vector<device::mojom::blink::ContactObjectPtr>> result) {
	DLOG(INFO) << "OnContactOnContactResultsWithFindOptionDeleteResults " << requestId << ", " << error;

	ContactSuccessCallback* successCB = mContactSuccessCBMap.at(requestId);
	ContactErrorCallback* errorCB = mContactErrorCBMap.at(requestId);

	if(error != Contact::kSuccess) {
		errorCB->handleResult((unsigned)error);
		return;
	}
	else if (result.has_value()){
		HeapVector<Member<ContactObject>> resultToReturn;
		ContactObject* tmpObj = nullptr;
		size_t resultSize = result.value().size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertMojoToScriptContact(result.value()[i].get());
			resultToReturn.push_back(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mContactFindOptionsMap.erase(requestId);
	mContactSuccessCBMap.erase(requestId);
	mContactErrorCBMap.erase(requestId);
}

unsigned Contact::getContactFindTarget(WTF::String targetName)
{
	if(targetName == "name")
		return 0;
	else if(targetName == "phone")
		return 1;
	else if(targetName == "email")
		return 2;
	else
		return 99;
}

ContactObject* Contact::convertMojoToScriptContact(device::mojom::blink::ContactObject* mojoContact) {
	DLOG(INFO) << "Contact::convertMojoToScriptContact";
	ContactObject* scriptContact = ContactObject::Create();
	scriptContact->setStructuredAddress(ContactAddress::Create());
	scriptContact->setStructuredName(ContactName::Create());

	scriptContact->setId(mojoContact->id);
	scriptContact->setDisplayName(mojoContact->displayName);
	scriptContact->setPhoneNumber(mojoContact->phoneNumber);
	scriptContact->setEmails(mojoContact->emails);
	scriptContact->setAddress(mojoContact->address);
	scriptContact->setAccountName(mojoContact->accountName);
	scriptContact->setAccountType(mojoContact->accountType);

	if(mojoContact->categories.has_value()) {
		WTF::Vector<WTF::String> categories = WTF::Vector<WTF::String>(mojoContact->categories.value());
		scriptContact->setCategories(categories);
	}

	scriptContact->structuredAddress()->setType(mojoContact->structuredAddress->type);
	scriptContact->structuredAddress()->setStreetAddress(mojoContact->structuredAddress->streetAddress);
	scriptContact->structuredAddress()->setLocality(mojoContact->structuredAddress->locality);
	scriptContact->structuredAddress()->setRegion(mojoContact->structuredAddress->region);
	scriptContact->structuredAddress()->setPostalCode(mojoContact->structuredAddress->postalCode);
	scriptContact->structuredAddress()->setCountry(mojoContact->structuredAddress->country);

	scriptContact->structuredName()->setFamilyName(mojoContact->structuredName->familyName);
	scriptContact->structuredName()->setGivenName(mojoContact->structuredName->givenName);
	scriptContact->structuredName()->setMiddleName(mojoContact->structuredName->middleName);
	scriptContact->structuredName()->setPrefix(mojoContact->structuredName->honorificPrefix);
	scriptContact->structuredName()->setSuffix(mojoContact->structuredName->honorificSuffix);

	return scriptContact;
}

void Contact::convertScriptContactToMojo(device::mojom::blink::ContactObject* mojoContact, ContactObject* blinkContact)
{
	mojoContact->structuredAddress = device::mojom::blink::ContactAddress::New();
	mojoContact->structuredName = device::mojom::blink::ContactName::New();

	mojoContact->id = blinkContact->id();
	mojoContact->displayName = blinkContact->displayName();
	mojoContact->phoneNumber = blinkContact->phoneNumber();
	mojoContact->emails = blinkContact->emails();
	mojoContact->address = blinkContact->address();
	mojoContact->accountName = blinkContact->accountName();
	mojoContact->accountType = blinkContact->accountType();
	size_t size = blinkContact->categories().size();
	if(size > 0) {
		mojoContact->categories = WTF::Vector<WTF::String>(blinkContact->categories());
	} else {
		mojoContact->categories = WTF::Optional<WTF::Vector<WTF::String>>();
	}

	if (blinkContact->structuredAddress() != nullptr) {
		mojoContact->structuredAddress->type = blinkContact->structuredAddress()->type();
		mojoContact->structuredAddress->streetAddress = blinkContact->structuredAddress()->streetAddress();
		mojoContact->structuredAddress->locality = blinkContact->structuredAddress()->locality();
		mojoContact->structuredAddress->region = blinkContact->structuredAddress()->region();
		mojoContact->structuredAddress->country = blinkContact->structuredAddress()->country();
		mojoContact->structuredAddress->postalCode = blinkContact->structuredAddress()->postalCode();
	} else {
		mojoContact->structuredAddress->type = "";
		mojoContact->structuredAddress->streetAddress = "";
		mojoContact->structuredAddress->locality = "";
		mojoContact->structuredAddress->region = "";
		mojoContact->structuredAddress->country = "";
		mojoContact->structuredAddress->postalCode = "";
	}

	if (blinkContact->structuredName() != nullptr) {
		mojoContact->structuredName->familyName = blinkContact->structuredName()->familyName();
		mojoContact->structuredName->givenName = blinkContact->structuredName()->givenName();
		mojoContact->structuredName->middleName = blinkContact->structuredName()->middleName();
		mojoContact->structuredName->honorificPrefix = blinkContact->structuredName()->prefix();
		mojoContact->structuredName->honorificSuffix = blinkContact->structuredName()->suffix();
	} else {
		mojoContact->structuredName->familyName = "";
		mojoContact->structuredName->givenName = "";
		mojoContact->structuredName->middleName = "";
		mojoContact->structuredName->honorificPrefix = "";
		mojoContact->structuredName->honorificSuffix = "";
	}

}

DEFINE_TRACE(Contact) {
	visitor->Trace(mContactObjectMap);
	visitor->Trace(mContactSuccessCBMap);
	visitor->Trace(mContactErrorCBMap);
}

}
