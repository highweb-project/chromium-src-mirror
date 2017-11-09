// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef AppStatus_h
#define AppStatus_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "ApplicationInfo.h"

namespace blink {

class ApplicationInfo;

class AppStatus final : public GarbageCollectedFinalized<AppStatus>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static AppStatus* create() {return new AppStatus();};
	~AppStatus();

	ApplicationInfo* appInfo();
	void setAppInfo(ApplicationInfo*);
	HeapVector<Member<ApplicationInfo>>& appList();
	int resultCode();
	void setResultCode(int resultCode);

	DEFINE_INLINE_TRACE() {
		visitor->Trace(mAppList);
		visitor->Trace(mAppInfo);
	};

private:
	AppStatus();

	int mResultCode;

	HeapVector<Member<ApplicationInfo>> mAppList;
	Member<ApplicationInfo> mAppInfo;
};

} // namespace blink

#endif // AppStatus_h
