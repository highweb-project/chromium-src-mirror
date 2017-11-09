// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GallerySuccessCallback_h
#define GallerySuccessCallback_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "modules/device_gallery/GalleryMediaObject.h"

namespace blink {

class GallerySuccessCallback: public GarbageCollectedFinalized<GallerySuccessCallback> {
public:
	virtual ~GallerySuccessCallback() { }
	virtual void handleEvent(const HeapVector<Member<GalleryMediaObject>>& value) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif // GallerySuccessCallback_h
