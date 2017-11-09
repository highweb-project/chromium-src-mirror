// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "WebCLCallbackHandler.h"

#include "gpu/ipc/client/gpu_channel_host.h"
#include "gpu/ipc/common/gpu_messages.h"

#include "base/rand_util.h"

#include "WebCLInclude.h"
#include "WebCLEvent.h"
#include "WebCLCallback.h"
#include "WebCL.h"

namespace blink {

WebCLCallbackHandler* WebCLCallbackHandler::create(CallbackListener* listener)
{
	return new WebCLCallbackHandler(listener);
}

WebCLCallbackHandler::WebCLCallbackHandler(CallbackListener* listener)
	:  mHandlerId(-1),
	   weak_ptr_factory_(this)

{
	mListener = listener;
}

WebCLCallbackHandler::~WebCLCallbackHandler()
{
}

bool WebCLCallbackHandler::OnMessageReceived(const IPC::Message& message)
{
	IPC_BEGIN_MESSAGE_MAP(WebCLCallbackHandler, message)
	#if defined(ENABLE_HIGHWEB_WEBCL)
		IPC_MESSAGE_HANDLER(OpenCLChannelMsg_Callback,
						OnClCallback);
	#endif
	IPC_END_MESSAGE_MAP()

	return true;
}

void WebCLCallbackHandler::OnChannelError()
{
}

void WebCLCallbackHandler::bindToChannel()
{
	CLLOG(INFO) << "WebCLCallbackHandler::bindToChannel";

	if (webcl_channel_ && mHandlerId < 0) {
		mHandlerId = webcl_channel_->GenerateRouteID();
		webcl_channel_->AddRoute(mHandlerId, weak_ptr_factory_.GetWeakPtr());

		CLLOG(INFO) << ">> mHandlerId=" << mHandlerId;
	}
}

void WebCLCallbackHandler::unbindToChannel()
{
	CLLOG(INFO) << "WebCLCallbackHandler::unbindToChannel";

	if (webcl_channel_) {
		CLLOG(INFO) << "WebCLCallbackHandler::~WebCLCallbackHandler";

		webcl_channel_->RemoveRoute(mHandlerId);
		mHandlerId = -1;
	}
}

#if defined(OS_ANDROID)
void WebCLCallbackHandler::OnClCallback(unsigned eventKey, unsigned callbackKey, unsigned objectType)
#elif defined(OS_LINUX)
void WebCLCallbackHandler::OnClCallback(cl_point eventKey, unsigned callbackKey, unsigned objectType)
#endif
{
	CLLOG(INFO) << "callback id=" << callbackKey << ", event key=" << eventKey << ", objectType=" << objectType;

	if(mListener != nullptr) {
		mListener->OnCallback(eventKey, callbackKey, objectType);
	}
}

}
