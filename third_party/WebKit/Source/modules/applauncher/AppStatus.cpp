// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "AppStatus.h"
#include "base/logging.h"

namespace blink {

	HeapVector<Member<ApplicationInfo>>& AppStatus::appList() {
		return mAppList;
	}

	AppStatus::~AppStatus() {
		mAppInfo = nullptr;
		mAppList.clear();
	}

	AppStatus::AppStatus() {
	}

	ApplicationInfo* AppStatus::appInfo() {
		return mAppInfo.get();
	}

	int AppStatus::resultCode() {
		return mResultCode;
	}

	void AppStatus::setAppInfo(ApplicationInfo* info) {
		mAppInfo = info;
	}

	void AppStatus::setResultCode(int resultCode) {
		mResultCode = resultCode;
	}

} // namespace blink
