// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef ApplicationInfo_h
#define ApplicationInfo_h

#include "platform/heap/Handle.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ApplicationInfo final : public GarbageCollectedFinalized<ApplicationInfo>, public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();
public:
  ApplicationInfo() {}
  ~ApplicationInfo() {}

  void setName(String name) {
    mName = name;
  }
  void setId(String id) {
    mId = id;
  }
  void setUrl(String url) {
    mUrl = url;
  }
  void setVersion(String version) {
    mVersion = version;
  }
  void setIconUrl(String iconUrl) {
    mIconUrl = iconUrl;
  }

  String name() {
    return mName;
  }
  String id() {
    return mId;
  }
  String url() {
    return mUrl;
  }
  String version() {
    return mVersion;
  }
  String iconUrl() {
    return mIconUrl;
  }

  DEFINE_INLINE_TRACE() {
  };

private:
  String mName = "";
  String mId = "";
  String mUrl = "";
  String mVersion = "";
  String mIconUrl = "";
};

} // namespace blink

#endif // ApplicationInfo_h
