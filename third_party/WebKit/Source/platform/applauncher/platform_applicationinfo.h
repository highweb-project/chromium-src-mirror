// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLINK_PLATFORM_APPLAUNCHER_PLATFORM_APPLICATIONINFO_H_
#define BLINK_PLATFORM_APPLAUNCHER_PLATFORM_APPLICATIONINFO_H_

#include "platform/PlatformExport.h"
#include "platform/heap/Handle.h"
#include "wtf/text/WTFString.h"

namespace blink {

class PLATFORM_EXPORT PlatformApplicationInfo : public GarbageCollectedFinalized<PlatformApplicationInfo> {
public:

  PlatformApplicationInfo();
	virtual ~PlatformApplicationInfo();

  void setName(String name);
  void setClassName(String className);
  void setDataDir(String dataDir);
  void setEnabled(bool enabled);
  void setFlags(long flags);
  void setPermission(String permission);
  void setProcessName(String processName);
  void setTargetSdkVersion(long targetSdkVersion);
  void setTheme(long theme);
  void setUid(long uid);
  void setPackageName(String packageName);

  String name();
  String className();
  String dataDir();
  bool enabled();
  long flags();
  String permission();
  String processName();
  long targetSdkVersion();
  long theme();
  long uid();
  String packageName();

	DECLARE_VIRTUAL_TRACE();

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

#endif // BLINK_PLATFORM_APPLAUNCHER_PLATFORM_APPLICATIONINFO_H_
