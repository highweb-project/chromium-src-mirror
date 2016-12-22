// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_API_THIRD_PARTY_BROADCAST_ANDROID_H_
#define CONTENT_BROWSER_DEVICE_API_THIRD_PARTY_BROADCAST_ANDROID_H_


#include <jni.h>

#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"

namespace content {

class ThirdPartyBroadcastAndroid {
 public:
  static bool Register(JNIEnv* env);
  static void SendAndroidBroadcast(const base::string16& d_action, int process_id_, int routing_id_);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_API_THIRD_PARTY_BROADCAST_ANDROID_H_
