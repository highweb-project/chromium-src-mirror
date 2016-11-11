// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceCpu_h
#define DeviceCpu_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/text/WTFString.h"
#include "wtf/ListHashSet.h"
#include "core/dom/Document.h"
#include "modules/EventTargetModules.h"
#include "core/dom/ActiveDOMObject.h"
#include "core/dom/ContextLifecycleObserver.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

#include "device/cpu/devicecpu_manager.mojom-blink.h"
#include "device/cpu/devicecpu_ResultCode.mojom-blink.h"

namespace blink {

class LocalFrame;
class DeviceCpuScriptCallback;
class DeviceCpuStatus;

class DeviceCpu final : public EventTargetWithInlineData, public ActiveScriptWrappable, public ActiveDOMObject {
	DEFINE_WRAPPERTYPEINFO();
	USING_GARBAGE_COLLECTED_MIXIN(DeviceCpu);
public:

	enum ErrorCodeList{
		// Exception code
		SUCCESS = 0,
		FAILURE = -1,
		NOT_ENABLED_PERMISSION = -2,
		NOT_SUPPORT_API = 9999,
	};

	enum function {
		FUNC_GET_CPU_LOAD = 0,
	};

	struct functionData {
		int functionCode = -1;
		//20160419-jphong
		//DeviceCpuScriptCallback* scriptCallback = nullptr;
		functionData(int code) {
			functionCode = code;
		}
	};

	static DeviceCpu* create(Document& document) {
		DeviceCpu* devicecpu = new DeviceCpu(document);
		devicecpu->suspendIfNeeded();
		return devicecpu;
	}
	~DeviceCpu() override;

	void load(DeviceCpuScriptCallback* callback);
	void resultCodeCallback();
	void notifyCallback(DeviceCpuStatus*, DeviceCpuScriptCallback*);
	void notifyError(int, DeviceCpuScriptCallback*);
	void continueFunction();

	void requestPermission();
	void onPermissionChecked(PermissionResult result);

	const WTF::AtomicString& interfaceName() const override { return EventTargetNames::DeviceCpu; }
	ExecutionContext* getExecutionContext() const override { return ContextLifecycleObserver::getExecutionContext(); }

	// ActiveDOMObject implementation.
	void suspend() override;
	void resume() override;
	void stop() override;
	//void contextDestroyed() override;
	bool hasPendingActivity() const override;

	DECLARE_VIRTUAL_TRACE();

private:
	DeviceCpu(Document& document);

	void loadInternal();
	void OnLoadCallback(device::blink::DeviceCpu_ResultCodePtr result);
	void stopOnLoadCallback();

	Deque<functionData*> d_functionData;

	typedef HeapListHashSet<Member<DeviceCpuScriptCallback>> CpuCallbackList;
	typedef CpuCallbackList::const_reverse_iterator CpuCallbackListIterator;
	CpuCallbackList callbackList;
	//ListHashSet<DeviceCpuScriptCallback*> callbackList;
	//20160419-jphong

	WTF::String mOrigin;
	WebDeviceApiPermissionCheckClient* mClient;

	device::blink::DeviceCpuManagerPtr deviceCpuManager;
	Member<DeviceCpuStatus> mLastLoadData = nullptr;

	bool ViewPermissionState = false;
	bool isPending = false;
};

} // namespace blink

#endif // DeviceCpu_h
