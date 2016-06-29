// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef ApplicationInfo_h
#define ApplicationInfo_h

#include "platform/heap/Handle.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/applauncher/platform_applicationinfo.h"

namespace blink {

class ApplicationInfo final : public PlatformApplicationInfo, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	ApplicationInfo() {}
	~ApplicationInfo() {}

	DEFINE_INLINE_TRACE() {
		PlatformApplicationInfo::trace(visitor);
	};
};

} // namespace blink

#endif // ApplicationInfo_h
