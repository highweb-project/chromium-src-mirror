// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef ApplicationInfo_h
#define ApplicationInfo_h

#include "platform/heap/Handle.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "wtf/text/WTFString.h"

namespace blink {

class ApplicationInfo final : public GarbageCollectedFinalized<ApplicationInfo>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	ApplicationInfo() {}
	~ApplicationInfo() {}

	void setName(String name) {
		mName = name;
	}
	void setClassName(String className) {
		mClassName = className;
	}
	void setDataDir(String dataDir) {
		mDataDir = dataDir;
	}
	void setEnabled(bool enabled) {
		mEnabled = enabled;
	}
	void setFlags(long flags) {
		mFlags = flags;
	}
	void setPermission(String permission) {
		mPermission = permission;
	}
	void setProcessName(String processName) {
		mProcessName = processName;
	}
	void setTargetSdkVersion(long targetSdkVersion) {
		mTargetSdkVersion = targetSdkVersion;
	}
	void setTheme(long theme) {
		mTheme = theme;
	}
	void setUid(long uid) {
		mUid = uid;
	}
	void setPackageName(String packageName) {
		mPackageName = packageName;
	}

	String name() {
		return mName;
	}
	String className() {
		return mClassName;
	}
	String dataDir() {
		return mDataDir;
	}
	bool enabled() {
		return mEnabled;
	}
	long flags() {
		return mFlags;
	}
	String permission() {
		return mPermission;
	}
	String processName() {
		return mProcessName;
	}
	long targetSdkVersion() {
		return mTargetSdkVersion;
	}
	long theme() {
		return mTheme;
	}
	long uid() {
		return mUid;
	}
	String packageName() {
		return mPackageName;
	}

	DEFINE_INLINE_TRACE() {
	};

private:
	String mName = "";
	String mClassName = "";
	String mDataDir = "";
	bool mEnabled = false;
	long mFlags = 0;
	String mPermission = "";
	String mProcessName = "";
	long mTargetSdkVersion = 0;
	long mTheme = 0;
	long mUid = 0;
	String mPackageName = "";
};

} // namespace blink

#endif // ApplicationInfo_h
