/*
 * device_api_contact_manager.cc
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#include "device_api_contact_manager.h"

#include "base/logging.h"
#include "base/rand_util.h"
#include "content/public/common/service_registry.h"
#include "content/public/renderer/render_thread.h"

#include "third_party/WebKit/public/platform/modules/device_contact/WebDeviceContactFindOptions.h"
#include "third_party/WebKit/public/platform/modules/device_contact/WebDeviceContactParameter.h"
#include "third_party/WebKit/public/platform/modules/device_contact/WebDeviceContactResultListener.h"

namespace content {

DeviceApiContactManager::DeviceApiContactManager() {
  if (ServiceRegistry* registry = RenderThread::Get()->GetServiceRegistry()) {
    registry->ConnectToRemoteService(mojo::GetProxy(&monitor_));
  }
}

DeviceApiContactManager::~DeviceApiContactManager() {
}

void DeviceApiContactManager::checkPermission()
{
}

void DeviceApiContactManager::didCheckPermission(bool result)
{
	DLOG(INFO) << "result:" << result;
}

void DeviceApiContactManager::findContact(blink::WebDeviceContactParameter* parameter)
{
	DLOG(INFO) << "eviceApiContactManager::findContact";

	int request_id = base::RandInt(1000,9999);
	find_request_map_[request_id] = parameter;

	blink::WebDeviceContactFindOptions* findOptions = parameter->mFindOptions;
	monitor_->FindContact(
			request_id,
			findOptions->mTarget,
			findOptions->mMaxItem,
			findOptions->mCondition.utf8(),
			base::Bind(&DeviceApiContactManager::didFindContact, base::Unretained(this)));
}

void DeviceApiContactManager::didFindContact(int requestID, unsigned error, mojo::Array<device::ContactObjectPtr> results)
{
	DLOG(INFO) << "DeviceApiContactManager::didFindContact";

	FindRequestsMap::iterator it = find_request_map_.find(requestID);
	if (it == find_request_map_.end()) {
		return;
	}
	blink::WebDeviceContactParameter* current_request = it->second;

	std::vector<blink::WebDeviceContactObject*> result_objs;
	result_objs.clear();

	if(error == 40) {
		size_t results_size = results.size();
		for(size_t i=0; i<results_size; i++) {
			DLOG(INFO) << results[i]->displayName.get();
			result_objs.push_back(convertToBlinkContact(results[i].get()));
		}
	}
	current_request->mListener->OnContactFindResult(current_request->mRequestId, error, result_objs);
}

void DeviceApiContactManager::addContact(blink::WebDeviceContactParameter* parameter)
{
	DLOG(INFO) << "deviceApiContactManager::addContact";

	int request_id = base::RandInt(1000,9999);
	add_request_map_[request_id] = parameter;

	device::ContactObjectPtr parmeterPtr(device::ContactObject::New());
	convertToMojoContact(parmeterPtr.get(), parameter->mContactObject);

	monitor_->AddContact(
			request_id,
			std::move(parmeterPtr),
			base::Bind(&DeviceApiContactManager::didAddContact, base::Unretained(this)));
}

void DeviceApiContactManager::didAddContact(int requestID, unsigned error, mojo::Array<device::ContactObjectPtr> results)
{
	DLOG(INFO) << "DeviceApiContactManager::didAddContact";

	AddRequestsMap::iterator it = add_request_map_.find(requestID);
	if (it == add_request_map_.end()) {
		return;
	}
	blink::WebDeviceContactParameter* current_request = it->second;

	std::vector<blink::WebDeviceContactObject*> result_objs;
	result_objs.clear();

	if(error == 40) {
		size_t results_size = results.size();
		for(size_t i=0; i<results_size; i++) {
			result_objs.push_back(convertToBlinkContact(results[i].get()));
		}
	}
	current_request->mListener->OnContactAddResult(current_request->mRequestId, error, result_objs);
}

void DeviceApiContactManager::deleteContact(blink::WebDeviceContactParameter* parameter)
{
	DLOG(INFO) << "deviceApiContactManager::deleteContact";

	int request_id = base::RandInt(1000,9999);
	delete_request_map_[request_id] = parameter;

	blink::WebDeviceContactFindOptions* findOptions = parameter->mFindOptions;
	monitor_->DeleteContact(
			request_id,
			findOptions->mTarget,
			findOptions->mMaxItem,
			findOptions->mCondition.utf8(),
			base::Bind(&DeviceApiContactManager::didDeleteContact, base::Unretained(this)));
}

void DeviceApiContactManager::didDeleteContact(int requestID, unsigned error, mojo::Array<device::ContactObjectPtr> results)
{
	DLOG(INFO) << "DeviceApiContactManager::didDeleteContact";

	DeleteRequestsMap::iterator it = delete_request_map_.find(requestID);
	if (it == delete_request_map_.end()) {
		return;
	}
	blink::WebDeviceContactParameter* current_request = it->second;

	std::vector<blink::WebDeviceContactObject*> result_objs;
	result_objs.clear();

	if(error == 40) {
		size_t results_size = results.size();
		for(size_t i=0; i<results_size; i++) {
			result_objs.push_back(convertToBlinkContact(results[i].get()));
		}
	}
	current_request->mListener->OnContactDeleteResult(current_request->mRequestId, error, result_objs);
}

void DeviceApiContactManager::updateContact(blink::WebDeviceContactParameter* parameter)
{
	DLOG(INFO) << "deviceApiContactManager::updateContact";

	int request_id = base::RandInt(1000,9999);
	update_request_map_[request_id] = parameter;

	device::ContactObjectPtr parmeterPtr(device::ContactObject::New());
	convertToMojoContact(parmeterPtr.get(), parameter->mContactObject);

	monitor_->UpdateContact(
			request_id,
			std::move(parmeterPtr),
			base::Bind(&DeviceApiContactManager::didUpdateContact, base::Unretained(this)));
}

void DeviceApiContactManager::didUpdateContact(int requestID, unsigned error, mojo::Array<device::ContactObjectPtr> results)
{
	DLOG(INFO) << "DeviceApiContactManager::didUpdateContact";

	UpdateRequestsMap::iterator it = update_request_map_.find(requestID);
	if (it == update_request_map_.end()) {
		return;
	}
	blink::WebDeviceContactParameter* current_request = it->second;

	std::vector<blink::WebDeviceContactObject*> result_objs;
	result_objs.clear();

	if(error == 40) {
		size_t results_size = results.size();
		for(size_t i=0; i<results_size; i++) {
			result_objs.push_back(convertToBlinkContact(results[i].get()));
		}
	}
	current_request->mListener->OnContactUpdateResult(current_request->mRequestId, error, result_objs);
}

blink::WebDeviceContactObject* DeviceApiContactManager::convertToBlinkContact(device::ContactObject* mojoContact)
{
	blink::WebDeviceContactObject* blinkContact = new blink::WebDeviceContactObject(
			blink::WebString::fromUTF8(mojoContact->id.get()),
			blink::WebString::fromUTF8(mojoContact->displayName.get()),
			blink::WebString::fromUTF8(mojoContact->phoneNumber.get()),
			blink::WebString::fromUTF8(mojoContact->emails.get()),
			blink::WebString::fromUTF8(mojoContact->address.get()),
			blink::WebString::fromUTF8(mojoContact->accountName.get()),
			blink::WebString::fromUTF8(mojoContact->accountType.get()));

	size_t category_length_ = mojoContact->categories.size();
	for(size_t i=0; i<category_length_; i++)
		blinkContact->mCategories.push_back(blink::WebString::fromUTF8(mojoContact->categories[i].get()));

	blinkContact->mStructuredAddress = blink::WebDeviceContactAddress();
	blinkContact->mStructuredAddress.mType = blink::WebString::fromUTF8(mojoContact->structuredAddress->type);
	blinkContact->mStructuredAddress.mStreetAddress = blink::WebString::fromUTF8(mojoContact->structuredAddress->streetAddress);
	blinkContact->mStructuredAddress.mLocality = blink::WebString::fromUTF8(mojoContact->structuredAddress->locality);
	blinkContact->mStructuredAddress.mRegion = blink::WebString::fromUTF8(mojoContact->structuredAddress->region);
	blinkContact->mStructuredAddress.mCountry = blink::WebString::fromUTF8(mojoContact->structuredAddress->country);
	blinkContact->mStructuredAddress.mPostalcode = blink::WebString::fromUTF8(mojoContact->structuredAddress->postalCode);

	blinkContact->mStructuredName = blink::WebDeviceContactName();
	blinkContact->mStructuredName.mFamilyName = blink::WebString::fromUTF8(mojoContact->structuredName->familyName);
	blinkContact->mStructuredName.mGivenName = blink::WebString::fromUTF8(mojoContact->structuredName->givenName);
	blinkContact->mStructuredName.mMiddleName = blink::WebString::fromUTF8(mojoContact->structuredName->middleName);
	blinkContact->mStructuredName.mHonorificPrefix = blink::WebString::fromUTF8(mojoContact->structuredName->honorificPrefix);
	blinkContact->mStructuredName.mHonorificSuffix = blink::WebString::fromUTF8(mojoContact->structuredName->honorificSuffix);

	return blinkContact;
}

void DeviceApiContactManager::convertToMojoContact(device::ContactObject* mojoContact, blink::WebDeviceContactObject* blinkContact)
{
	mojoContact->structuredAddress = device::ContactAddress::New();
	mojoContact->structuredName = device::ContactName::New();

	mojoContact->id = blinkContact->mId.utf8();
	mojoContact->displayName = blinkContact->mDisplayName.utf8();
	mojoContact->phoneNumber = blinkContact->mPhoneNumber.utf8();
	mojoContact->emails = blinkContact->mEmails.utf8();
	mojoContact->address = blinkContact->mAddress.utf8();
	mojoContact->accountName = blinkContact->mAccountName.utf8();
	mojoContact->accountType = blinkContact->mAccountType.utf8();
	if(blinkContact->mCategories.size() > 0) {
		size_t size = blinkContact->mCategories.size();
		mojoContact->categories = mojo::Array<mojo::String>::New(size);
		for(size_t i=0; i<size; i++)
			mojoContact->categories[i] = blinkContact->mCategories[i].utf8();
	}

	mojoContact->structuredAddress->type = blinkContact->mStructuredAddress.mType.utf8();
	mojoContact->structuredAddress->streetAddress = blinkContact->mStructuredAddress.mStreetAddress.utf8();
	mojoContact->structuredAddress->locality = blinkContact->mStructuredAddress.mLocality.utf8();
	mojoContact->structuredAddress->region = blinkContact->mStructuredAddress.mRegion.utf8();
	mojoContact->structuredAddress->country = blinkContact->mStructuredAddress.mCountry.utf8();
	mojoContact->structuredAddress->postalCode = blinkContact->mStructuredAddress.mPostalcode.utf8();

	mojoContact->structuredName->familyName = blinkContact->mStructuredName.mFamilyName.utf8();
	mojoContact->structuredName->givenName = blinkContact->mStructuredName.mGivenName.utf8();
	mojoContact->structuredName->middleName = blinkContact->mStructuredName.mMiddleName.utf8();
	mojoContact->structuredName->honorificPrefix = blinkContact->mStructuredName.mHonorificPrefix.utf8();
	mojoContact->structuredName->honorificSuffix = blinkContact->mStructuredName.mHonorificSuffix.utf8();
}

}  // namespace content
