// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "platform/wtf/build_config.h"
#include "modules/device_api/ThirdPartyDeviceApi.h"

#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"

#include "platform/mojo/MojoHelper.h"
#include "services/device/public/interfaces/constants.mojom-blink.h"
#include "services/service_manager/public/cpp/connector.h"

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
	if (callback != nullptr && !action.IsEmpty()) {
    mCallbackList.push_back(callback);
  } else {
    return;
  }
	isPending = true;
	functionData* data = new functionData(function::FUNC_SEND_ANDROID_BROADCAST);
  data->action = action;
	d_functionData.push_back(data);
	data = nullptr;
	
	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void ThirdPartyDeviceApi::sendAndroidBroadcastInternal() {
	if (!thirdpartyApiManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&thirdpartyApiManager));
	}
	if (!isPending) {
		stopOnBroadcastCallback();
		return;
	}

	thirdpartyApiManager->sendAndroidBroadcast(d_functionData.front()->action,
		ConvertToBaseCallback(WTF::Bind(&ThirdPartyDeviceApi::mojoResultCallback, WrapPersistent(this))));
}
void ThirdPartyDeviceApi::mojoResultCallback(device::mojom::blink::SendAndroidBroadcastCallbackDataPtr result) {
	DCHECK(result.get());

	ExecutionContext* context = GetExecutionContext();
	if (context->IsContextSuspended() || context->IsContextDestroyed()) {
		stopOnBroadcastCallback();
		return;
	}

	if (mCallbackList.size() > 0) {
    mCallbackList.at(0).Get()->onResult(result->action);
		mCallbackList.erase(0);
	}
	if (d_functionData.size() > 0) {
		d_functionData.pop_front();
	}
	continueFunction();
}

void ThirdPartyDeviceApi::Suspend()
{
	isPending = false;
	stopOnBroadcastCallback();
}

void ThirdPartyDeviceApi::Resume()
{
	isPending = true;
	continueFunction();
}

void ThirdPartyDeviceApi::ContextDestroyed(ExecutionContext* context)
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
		switch(d_functionData.front()->functionCode) {
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
	visitor->Trace(mCallbackList);
	SuspendableObject::Trace(visitor);
}

} // namespace blink
