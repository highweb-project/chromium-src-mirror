// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "modules/device_api/ThirdPartyDeviceApi.h"

#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"

#include "platform/mojo/MojoHelper.h"
#include "public/platform/InterfaceProvider.h"

namespace blink {

ThirdPartyDeviceApi::ThirdPartyDeviceApi(Document& document)
	: SuspendableObject((ExecutionContext*)&document)
{
	d_functionData.clear();
}

ThirdPartyDeviceApi::~ThirdPartyDeviceApi()
{
	DLOG(INFO) << "~ThirdPartyDeviceApi";
	stopOnBroadcastCallback();
	if (thirdpartyApiManager.is_bound()) {
		thirdpartyApiManager.reset();
	}
	d_functionData.clear();
	mCallbackList.clear();
}

void ThirdPartyDeviceApi::sendAndroidBroadcast(String action, SendAndroidBroadcastCallback* callback) {
	if (callback != nullptr && !action.isEmpty()) {
    mCallbackList.append(callback);
  } else {
    return;
  }
	isPending = true;
	functionData* data = new functionData(function::FUNC_SEND_ANDROID_BROADCAST);
  data->action = action;
	d_functionData.append(data);
	data = nullptr;
	
	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void ThirdPartyDeviceApi::sendAndroidBroadcastInternal() {
	if (!thirdpartyApiManager.is_bound()) {
		Platform::current()->interfaceProvider()->getInterface(mojo::GetProxy(&thirdpartyApiManager));
	}
	if (!isPending) {
		stopOnBroadcastCallback();
		return;
	}

	thirdpartyApiManager->sendAndroidBroadcast(d_functionData.first()->action,
		convertToBaseCallback(WTF::bind(&ThirdPartyDeviceApi::mojoResultCallback, wrapPersistent(this))));
}
void ThirdPartyDeviceApi::mojoResultCallback(device::blink::SendAndroidBroadcastCallbackDataPtr result) {
	DCHECK(result.get());

	Document* document = toDocument(getExecutionContext());
	if (document->activeDOMObjectsAreSuspended() || document->isContextDestroyed()) {
		stopOnBroadcastCallback();
		return;
	}

	if (mCallbackList.size() > 0) {
    mCallbackList.at(0).get()->onResult(result->action);
		mCallbackList.remove(0);
	}
	if (d_functionData.size() > 0) {
		d_functionData.removeFirst();
	}
	continueFunction();
}

void ThirdPartyDeviceApi::suspend()
{
	isPending = false;
	stopOnBroadcastCallback();
}

void ThirdPartyDeviceApi::resume()
{
	isPending = true;
	continueFunction();
}

void ThirdPartyDeviceApi::contextDestroyed()
{
	isPending = false;
	stopOnBroadcastCallback();
  d_functionData.clear();
}

void ThirdPartyDeviceApi::stopOnBroadcastCallback() {
	if (thirdpartyApiManager.is_bound()) {
		thirdpartyApiManager.reset();
	}
	isPending = false;
}


void ThirdPartyDeviceApi::continueFunction() {
	if (d_functionData.size() > 0) {
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_SEND_ANDROID_BROADCAST : {
				sendAndroidBroadcastInternal();
				break;
			}
			default: {
				break;
			}
		}
	}
}

DEFINE_TRACE(ThirdPartyDeviceApi)
{
	visitor->trace(mCallbackList);
	SuspendableObject::trace(visitor);
}

} // namespace blink
