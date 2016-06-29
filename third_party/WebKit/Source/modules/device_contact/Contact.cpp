/*
 * Contact.cpp
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */

#include "wtf/build_config.h"
#include "modules/device_contact/Contact.h"

// #include "base/basictypes.h"
#include "base/bind.h"
#include "base/callback_forward.h"
#include "base/logging.h"
#include "base/rand_util.h"
#include "core/dom/Document.h"
#include "wtf/text/WTFString.h"

#include "public/platform/Platform.h"
#include "public/platform/WebString.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
#include "public/platform/modules/device_contact/WebDeviceContactAddress.h"
#include "public/platform/modules/device_contact/WebDeviceContactFindOptions.h"
#include "public/platform/modules/device_contact/WebDeviceContactName.h"
#include "public/platform/modules/device_contact/WebDeviceContactObject.h"
#include "public/platform/modules/device_contact/WebDeviceContactParameter.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "modules/device_contact/ContactAddress.h"
#include "modules/device_contact/ContactErrorCallback.h"
#include "modules/device_contact/ContactName.h"
#include "modules/device_contact/ContactObject.h"
#include "modules/device_contact/ContactSuccessCallback.h"

namespace blink {

ContactRequestHolder::ContactRequestHolder(ContactOperation operation, int reuqestID)
	: mRequestID(reuqestID),
	  mOperation(operation)
{
}

Contact::Contact(LocalFrame& frame)
	: mClient(DeviceApiPermissionController::from(frame)->client())
{
	WTF::String origin = frame.document()->url().strippedForUseAsReferrer();
	mClient->SetOrigin(origin.latin1().data());

	mPermissionMap[(int)PermissionType::MODIFY] = false;
	mPermissionMap[(int)PermissionType::VIEW] = false;
}

Contact::~Contact()
{
}

void Contact::onPermissionChecked(PermissionResult result)
{
	DLOG(INFO) << "Contact::onPermissionChecked, result=" << result;

	if(mRequestQueue.size()) {
		ContactRequestHolder* holder = mRequestQueue.front();

		if(result != PermissionResult::RESULT_OK) {
			ContactErrorCallback* errorCallback = mContactErrorCBMap.get(holder->mRequestID);
			if(errorCallback != nullptr) {
				errorCallback->handleResult((unsigned)Contact::PERMISSION_DENIED_ERROR);
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
	mContactFindOptionsMap.set(requestId, findOptions);
	mContactSuccessCBMap.set(requestId, successCallback);
	mContactErrorCBMap.set(requestId, errorCallback);

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

	WebDeviceContactParameter* parameter = new WebDeviceContactParameter(this);
	parameter->mRequestId = requestId;
	ContactFindOptions findOptions = mContactFindOptionsMap.get(requestId);
	parameter->mFindOptions = new WebDeviceContactFindOptions(
			getContactFindTarget(findOptions.target()),
			findOptions.maxItem(),
			WebString(findOptions.condition()));

	Platform::current()->findContact(parameter);
}

void Contact::OnContactFindResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results)
{
	DLOG(INFO) << "Contact::OnContactFindResult, result size=" << results.size();

	ContactSuccessCallback* successCB = mContactSuccessCBMap.get(requestId);
	ContactErrorCallback* errorCB = mContactErrorCBMap.get(requestId);

	if(error != Contact::SUCCESS) {
		errorCB->handleResult((unsigned)error);
		return;
	}
	else {
		HeapVector<Member<ContactObject>> resultToReturn;
		ContactObject* tmpObj = nullptr;
		size_t resultSize = results.size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertToScriptContact(results[i]);
			resultToReturn.append(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mContactFindOptionsMap.remove(requestId);
	mContactSuccessCBMap.remove(requestId);
	mContactErrorCBMap.remove(requestId);
}

// findContact -------------------------------------------

// addContact --------------------------------------------

void Contact::addContact(Member<ContactObject> contact, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback)
{
	DLOG(INFO) << "Contact::addContact";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY];
	DLOG(INFO) << ">> add operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);

	mContactObjectMap.set(requestId, contact.get());
	mContactSuccessCBMap.set(requestId, successCallback);
	mContactErrorCBMap.set(requestId, errorCallback);

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
	WebDeviceContactParameter* parameter = new WebDeviceContactParameter(this);
	parameter->mContactObject = convertToBlinkContact(mContactObjectMap.get(requestId));
	parameter->mRequestId = requestId;

	Platform::current()->addContact(parameter);
}

void Contact::OnContactAddResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results)
{
	DLOG(INFO) << "Contact::OnContactAddResult, result size=" << results.size();

	ContactSuccessCallback* successCB = mContactSuccessCBMap.get(requestId);
	ContactErrorCallback* errorCB = mContactErrorCBMap.get(requestId);

	if(error != Contact::SUCCESS) {
		errorCB->handleResult((unsigned)error);
		return;
	}
	else {
		HeapVector<Member<ContactObject>> resultToReturn;
		ContactObject* tmpObj = nullptr;
		size_t resultSize = results.size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertToScriptContact(results[i]);
			resultToReturn.append(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mContactObjectMap.remove(requestId);
	mContactSuccessCBMap.remove(requestId);
	mContactErrorCBMap.remove(requestId);
}

// addContact --------------------------------------------

// deleteContact --------------------------------------------

void Contact::deleteContact(ContactFindOptions findOptions, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback)
{
	DLOG(INFO) << "Contact::deleteContact";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY];
	DLOG(INFO) << ">> find operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);

	mContactFindOptionsMap.set(requestId, findOptions);
	mContactSuccessCBMap.set(requestId, successCallback);
	mContactErrorCBMap.set(requestId, errorCallback);

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

	WebDeviceContactParameter* parameter = new WebDeviceContactParameter(this);
	parameter->mRequestId = requestId;
	ContactFindOptions findOptions = mContactFindOptionsMap.get(requestId);
	parameter->mFindOptions = new WebDeviceContactFindOptions(
			getContactFindTarget(findOptions.target()),
			findOptions.maxItem(),
			WebString(findOptions.condition()));

	Platform::current()->deleteContact(parameter);
}

void Contact::OnContactDeleteResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results)
{
	DLOG(INFO) << "Contact::OnContactDeleteResult, error=" << error;

	ContactSuccessCallback* successCB = mContactSuccessCBMap.get(requestId);
	ContactErrorCallback* errorCB = mContactErrorCBMap.get(requestId);

	if(error != Contact::SUCCESS) {
		errorCB->handleResult((unsigned)error);
		return;
	}
	else {
		HeapVector<Member<ContactObject>> resultToReturn;
		ContactObject* tmpObj = nullptr;
		size_t resultSize = results.size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertToScriptContact(results[i]);
			resultToReturn.append(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mContactFindOptionsMap.remove(requestId);
	mContactSuccessCBMap.remove(requestId);
	mContactErrorCBMap.remove(requestId);
}

// deleteContact --------------------------------------------

// updateContact --------------------------------------------

void Contact::updateContact(Member<ContactObject> contact, ContactSuccessCallback* successCallback, ContactErrorCallback* errorCallback)
{
	DLOG(INFO) << "Contact::updateContact";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY];
	DLOG(INFO) << ">> update operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);

	mContactObjectMap.set(requestId, contact.get());
	mContactSuccessCBMap.set(requestId, successCallback);
	mContactErrorCBMap.set(requestId, errorCallback);

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

	addContactInternal(requestId);
}

void Contact::updateContactInternal(int requestId)
{
	WebDeviceContactParameter* parameter = new WebDeviceContactParameter(this);
	parameter->mContactObject = convertToBlinkContact(mContactObjectMap.get(requestId));
	parameter->mRequestId = requestId;

	Platform::current()->updateContact(parameter);
}

void Contact::OnContactUpdateResult(int requestId, unsigned error, std::vector<WebDeviceContactObject*> results)
{
	DLOG(INFO) << "Contact::OnContactUpdateResult, result size=" << results.size();

	ContactSuccessCallback* successCB = mContactSuccessCBMap.get(requestId);
	ContactErrorCallback* errorCB = mContactErrorCBMap.get(requestId);

	if(error != Contact::SUCCESS) {
		errorCB->handleResult((unsigned)error);
		return;
	}
	else {
		HeapVector<Member<ContactObject>> resultToReturn;
		ContactObject* tmpObj = nullptr;
		size_t resultSize = results.size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertToScriptContact(results[i]);
			resultToReturn.append(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mContactObjectMap.remove(requestId);
	mContactSuccessCBMap.remove(requestId);
	mContactErrorCBMap.remove(requestId);
}

// updateContact --------------------------------------------

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

WebDeviceContactObject* Contact::convertToBlinkContact(ContactObject* scriptContact)
{
	WebDeviceContactObject* blinkContact = new WebDeviceContactObject();
	if(scriptContact->hasId())
		blinkContact->mId = WebString(scriptContact->id());
	if(scriptContact->hasDisplayName())
			blinkContact->mDisplayName = WebString(scriptContact->displayName());
	if(scriptContact->hasPhoneNumber())
			blinkContact->mPhoneNumber = WebString(scriptContact->phoneNumber());
	if(scriptContact->hasEmails())
			blinkContact->mEmails = WebString(scriptContact->emails());
	if(scriptContact->hasAddress())
			blinkContact->mAddress = WebString(scriptContact->address());
	if(scriptContact->hasAccountName())
			blinkContact->mAccountName = WebString(scriptContact->accountName());
	if(scriptContact->hasAccountType())
			blinkContact->mAccountType = WebString(scriptContact->accountType());
	if(scriptContact->hasCategories()) {
		size_t size = scriptContact->categories().size();
		for(size_t i=0; i<size; i++)
			blinkContact->mCategories.push_back(WebString(scriptContact->categories()[i]));
	}

	if(scriptContact->hasStructuredAddress()) {
		blinkContact->mStructuredAddress = WebDeviceContactAddress();
		if(scriptContact->structuredAddress()->hasType())
			blinkContact->mStructuredAddress.mType = WebString(scriptContact->structuredAddress()->type());
		if(scriptContact->structuredAddress()->hasStreetAddress())
			blinkContact->mStructuredAddress.mStreetAddress = scriptContact->structuredAddress()->streetAddress();
		if(scriptContact->structuredAddress()->hasLocality())
			blinkContact->mStructuredAddress.mLocality = scriptContact->structuredAddress()->locality();
		if(scriptContact->structuredAddress()->hasRegion())
			blinkContact->mStructuredAddress.mRegion = scriptContact->structuredAddress()->region();
		if(scriptContact->structuredAddress()->hasPostalCode())
			blinkContact->mStructuredAddress.mPostalcode = scriptContact->structuredAddress()->postalCode();
		if(scriptContact->structuredAddress()->hasCountry())
			blinkContact->mStructuredAddress.mCountry = scriptContact->structuredAddress()->country();
	}

	if(scriptContact->hasStructuredName()) {
		blinkContact->mStructuredName = WebDeviceContactName();
		if(scriptContact->structuredName()->hasFamilyName())
			blinkContact->mStructuredName.mFamilyName = scriptContact->structuredName()->familyName();
		if(scriptContact->structuredName()->hasGivenName())
			blinkContact->mStructuredName.mGivenName = scriptContact->structuredName()->givenName();
		if(scriptContact->structuredName()->hasMiddleName())
			blinkContact->mStructuredName.mMiddleName = scriptContact->structuredName()->middleName();
		if(scriptContact->structuredName()->hasPrefix())
			blinkContact->mStructuredName.mHonorificPrefix = scriptContact->structuredName()->prefix();
		if(scriptContact->structuredName()->hasSuffix())
			blinkContact->mStructuredName.mHonorificSuffix = scriptContact->structuredName()->suffix();
	}
	return blinkContact;
}

ContactObject* Contact::convertToScriptContact(WebDeviceContactObject* blinkContact)
{
	ContactObject* scriptContact = ContactObject::create();
	scriptContact->setStructuredAddress(ContactAddress::create());
	scriptContact->setStructuredName(ContactName::create());

	scriptContact->setId(WTF::String(blinkContact->mId.utf8().c_str()));
	scriptContact->setDisplayName(WTF::String(blinkContact->mDisplayName.utf8().c_str()));
	scriptContact->setPhoneNumber(WTF::String(blinkContact->mPhoneNumber.utf8().c_str()));
	scriptContact->setEmails(WTF::String(blinkContact->mEmails.utf8().c_str()));
	scriptContact->setAddress(WTF::String(blinkContact->mAddress.utf8().c_str()));
	scriptContact->setAccountName(WTF::String(blinkContact->mAccountName.utf8().c_str()));
	scriptContact->setAccountType(WTF::String(blinkContact->mAccountType.utf8().c_str()));
	size_t size = blinkContact->mCategories.size();
	if(size > 0) {
		WTF::Vector<WTF::String> categories;
		for(size_t i=0; i<size; i++)
			categories.append(blinkContact->mCategories[i]);

		scriptContact->setCategories(categories);
	}

	scriptContact->structuredAddress()->setType(WTF::String(blinkContact->mStructuredAddress.mType.utf8().c_str()));
	scriptContact->structuredAddress()->setStreetAddress(WTF::String(blinkContact->mStructuredAddress.mStreetAddress.utf8().c_str()));
	scriptContact->structuredAddress()->setLocality(WTF::String(blinkContact->mStructuredAddress.mLocality.utf8().c_str()));
	scriptContact->structuredAddress()->setRegion(WTF::String(blinkContact->mStructuredAddress.mRegion.utf8().c_str()));
	scriptContact->structuredAddress()->setPostalCode(WTF::String(blinkContact->mStructuredAddress.mPostalcode.utf8().c_str()));
	scriptContact->structuredAddress()->setCountry(WTF::String(blinkContact->mStructuredAddress.mCountry.utf8().c_str()));

	scriptContact->structuredName()->setFamilyName(WTF::String(blinkContact->mStructuredName.mFamilyName.utf8().c_str()));
	scriptContact->structuredName()->setGivenName(WTF::String(blinkContact->mStructuredName.mGivenName.utf8().c_str()));
	scriptContact->structuredName()->setMiddleName(WTF::String(blinkContact->mStructuredName.mMiddleName.utf8().c_str()));
	scriptContact->structuredName()->setPrefix(WTF::String(blinkContact->mStructuredName.mHonorificPrefix.utf8().c_str()));
	scriptContact->structuredName()->setSuffix(WTF::String(blinkContact->mStructuredName.mHonorificSuffix.utf8().c_str()));

	return scriptContact;
}

DEFINE_TRACE(Contact) {
	visitor->trace(mContactObjectMap);
	visitor->trace(mContactSuccessCBMap);
	visitor->trace(mContactErrorCBMap);
}

}
