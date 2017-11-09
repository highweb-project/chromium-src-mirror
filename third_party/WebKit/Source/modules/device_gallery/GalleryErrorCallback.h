// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GalleryErrorCallback_h
#define GalleryErrorCallback_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

namespace blink {

class GalleryErrorCallback : public GarbageCollectedFinalized<GalleryErrorCallback> {
public:
	virtual ~GalleryErrorCallback() { }
	virtual void handleEvent(int error_code) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif // GalleryErrorCallback_h
