// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef DeviceGalleryStatus_h
#define DeviceGalleryStatus_h

#include "platform/heap/Handle.h"
#include "wtf/Forward.h"
#include "wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "GalleryMediaObject.h"

namespace blink {

class DeviceGalleryStatus final : public GarbageCollectedFinalized<DeviceGalleryStatus> {
public:
	static DeviceGalleryStatus* create() {return new DeviceGalleryStatus();};
	virtual ~DeviceGalleryStatus();

	void setFunctionCode(int function);
	int getFunctionCode(){return mFunction;}
	void setResultCode(int resultCode);
	int getResultCode(){return mResultCode;}

	HeapVector<Member<GalleryMediaObject>>& mediaObjectList();

	DEFINE_INLINE_TRACE() {visitor->trace(data);};

private:
	DeviceGalleryStatus();
	
	int mFunction;
	int mResultCode;
	HeapVector<Member<GalleryMediaObject>> data;
};

} // namespace blink

#endif // DeviceGalleryStatus_h