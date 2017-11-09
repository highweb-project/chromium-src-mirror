// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ThirdPartyDeviceApi_h
#define ThirdPartyDeviceApi_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Deque.h"
#include "core/dom/ContextLifecycleObserver.h"
#include "core/dom/SuspendableObject.h"


#include "services/device/public/interfaces/devicethirdparty_manager.mojom-blink.h"
#include "SendAndroidBroadcastCallback.h"

namespace blink {

class Document;

class ThirdPartyDeviceApi
	: public GarbageCollectedFinalized<ThirdPartyDeviceApi>
	, public ScriptWrappable, public SuspendableObject {
	DEFINE_WRAPPERTYPEINFO();
	USING_GARBAGE_COLLECTED_MIXIN(ThirdPartyDeviceApi);
public:

	enum function {
		FUNC_SEND_ANDROID_BROADCAST = 1,
	};

	struct functionData {
		int functionCode = -1;
		String action = "";
		functionData(int code) {
			functionCode = code;
		}
	};

	static ThirdPartyDeviceApi* create(Document& document) {
		ThirdPartyDeviceApi* deviceapi = new ThirdPartyDeviceApi(document);
		deviceapi->SuspendIfNeeded();
		return deviceapi;
	}
	virtual ~ThirdPartyDeviceApi();

	void sendAndroidBroadcast(String action, SendAndroidBroadcastCallback* callback);
	void continueFunction();

	ExecutionContext* GetExecutionContext() const {
		return ContextLifecycleObserver::GetExecutionContext();
	}
	void Suspend() override;
	void Resume() override;
	// bool hasPendingActivity() const override;
  void ContextDestroyed(ExecutionContext*) override;

	DECLARE_TRACE();

private:
	ThirdPartyDeviceApi(Document& document);

	void sendAndroidBroadcastInternal();
	void mojoResultCallback(device::mojom::blink::SendAndroidBroadcastCallbackDataPtr result);
	void stopOnBroadcastCallback();

	Deque<functionData*> d_functionData;
	HeapVector<Member<SendAndroidBroadcastCallback>> mCallbackList;
	bool isPending = false;
	device::mojom::blink::DeviceThirdpartyManagerPtr thirdpartyApiManager;
};

} // namespace blink

#endif // ThirdPartyDeviceApi_h
