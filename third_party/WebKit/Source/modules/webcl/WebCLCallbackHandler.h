// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLCallbackHandleer_h
#define WebCLCallbackHandleer_h

#include "base/memory/weak_ptr.h"
#include "ipc/ipc_listener.h"

#include "platform/heap/Handle.h"
#include <wtf/HashMap.h>

#include "WebCLInclude.h"

namespace blink {

class WebCLCallback;

class WebCLCallbackHandler: public IPC::Listener {
public:
	class CallbackListener {
	public:
#if defined(OS_ANDROID)
		virtual void OnCallback(unsigned eventKey, unsigned callbackKey, unsigned objectType) = 0;
#elif defined(OS_LINUX)
		virtual void OnCallback(cl_point eventKey, unsigned callbackKey, unsigned objectType) = 0;
#endif
		virtual ~CallbackListener() {}
	};

	static WebCLCallbackHandler* create(CallbackListener* listener);

	// IPC::Channel::Listener implementation:
	bool OnMessageReceived(const IPC::Message& message) override;
	void OnChannelError() override;

	void bindToChannel();
	void unbindToChannel();
	int getHandlerId() { return mHandlerId; };
private:
	WebCLCallbackHandler(CallbackListener* listener);
	~WebCLCallbackHandler() override;
#if defined(OS_ANDROID)
	void OnClCallback(unsigned eventKey, unsigned callbackKey, unsigned objectType);
#elif defined(OS_LINUX)
	void OnClCallback(cl_point eventKey, unsigned callbackKey, unsigned objectType);
#endif

	int mHandlerId;
	CallbackListener* mListener;

	base::WeakPtrFactory<WebCLCallbackHandler> weak_ptr_factory_;
	DISALLOW_COPY_AND_ASSIGN(WebCLCallbackHandler);
};

}

#endif /* WebCLCallbackHandleer_h */
