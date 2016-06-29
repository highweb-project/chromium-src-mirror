// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DEVICESTORAGE_DISPATCHER_H_
#define CONTENT_RENDERER_DEVICESTORAGE_DISPATCHER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "device/storage/devicestorage_ResultCode.mojom.h"
#include "device/storage/devicestorage_manager.mojom.h"

#include "third_party/WebKit/public/platform/modules/device_systeminformation/WebDeviceStorageListener.h"
#include "third_party/WebKit/public/platform/modules/device_systeminformation/WebDeviceStorageStatus.h"

namespace blink {
class WebDeviceStorageStatus;
class WebDeviceStorageListener;
}

namespace content {

class CONTENT_EXPORT DeviceStorageDispatcher {
	public:
		explicit DeviceStorageDispatcher();
		~DeviceStorageDispatcher();
		void getDeviceStorage(blink::WebDeviceStorageListener* mCallback);
		void resultCodeCallback(device::DeviceStorage_ResultCodePtr resultCodePtr);
	private:
		device::DeviceStorageManagerPtr manager_;
		blink::WebDeviceStorageListener* listener_;
		blink::WebDeviceStorageStatus webDeviceStoragestatus;

		DISALLOW_COPY_AND_ASSIGN(DeviceStorageDispatcher);
};

}  // namespace content

#endif  // CONTENT_RENDERER_DEVICESTORAGE_DISPATCHER_H_
