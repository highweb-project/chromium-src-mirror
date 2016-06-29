// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef DeviceStorageStatus_h
#define DeviceStorageStatus_h

#include "platform/heap/Handle.h"
#include "wtf/Forward.h"
#include "wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "modules/device_storage/DeviceStorageInfo.h"

namespace blink {

class DeviceStorageInfo;

class DeviceStorageStatus final : public GarbageCollectedFinalized<DeviceStorageStatus>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static DeviceStorageStatus* create() {return new DeviceStorageStatus();};
	virtual ~DeviceStorageStatus();

	void setResultCode(int code);
	int resultCode();

	HeapVector<DeviceStorageInfo>& storageList();

	DEFINE_INLINE_TRACE() {
		visitor->trace(mStorageList);
	}

private:
	DeviceStorageStatus();
	
	int mResultCode = -1;
	HeapVector<DeviceStorageInfo> mStorageList;
};

} // namespace blink

#endif // DeviceStorageStatus_h