// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_api/third_party_broadcast_android.h"

#include "base/android/context_utils.h"
#include "base/android/jni_android.h"
#include "jni/ThirdPartyBroadcast_jni.h"

#include "base/android/jni_string.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/common/frame_messages.h"

#include "base/logging.h"

using base::android::ConvertUTF16ToJavaString;

namespace content {

static void ReceiveAndroidBroadcast(JNIEnv* env, const base::android::JavaParamRef<jclass>& jcaller, const base::android::JavaParamRef<jstring>& j_action, jint process_id_, jint routing_id_) {
  DLOG(INFO) << "ReceiveAndroidBroadcast";
  std::string action(base::android::ConvertJavaStringToUTF8(env, j_action));
  content::RenderFrameHost* host = content::RenderFrameHost::FromID(process_id_, routing_id_);

  if(!host) {
  } else {
    host->Send(new FrameMsg_SendAndroidBroadcastResponse(routing_id_, action));
  }
}

bool ThirdPartyBroadcastAndroid::Register(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

// static
void ThirdPartyBroadcastAndroid::SendAndroidBroadcast(const base::string16& d_action, int process_id_, int routing_id_) {
  DLOG(INFO) << "content::sendAndroidBroadcast";
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> j_action = ConvertUTF16ToJavaString(env, d_action);
  Java_ThirdPartyBroadcast_SendAndroidBroadcast(
      env, base::android::GetApplicationContext(), j_action.obj(), process_id_, routing_id_);
}

}  // namespace content
