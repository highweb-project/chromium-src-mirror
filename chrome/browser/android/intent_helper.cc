// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/intent_helper.h"

#include <jni.h>

#include "base/android/context_utils.h"
#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "jni/IntentHelper_jni.h"

// sendAndroidBroadcast
#include "base/logging.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/common/frame_messages.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF16ToJavaString;

// sendAndroidBroadcast
static void OnReceiveAndroidBroadcast(JNIEnv* env, const JavaParamRef<jclass>& jcaller, const JavaParamRef<jstring>& j_action, jint process_id_, jint routing_id_) {
  std::string action(base::android::ConvertJavaStringToUTF8(env, j_action));
  content::RenderFrameHost* host = content::RenderFrameHost::FromID(process_id_, routing_id_);

  if(!host) {
  } else {
    host->Send(new FrameMsg_SendAndroidBroadcastResponse(routing_id_, action));
  }
}

namespace chrome {
namespace android {

void SendEmail(const base::string16& d_email,
               const base::string16& d_subject,
               const base::string16& d_body,
               const base::string16& d_chooser_title,
               const base::string16& d_file_to_attach) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> j_email = ConvertUTF16ToJavaString(env, d_email);
  ScopedJavaLocalRef<jstring> j_subject =
      ConvertUTF16ToJavaString(env, d_subject);
  ScopedJavaLocalRef<jstring> j_body = ConvertUTF16ToJavaString(env, d_body);
  ScopedJavaLocalRef<jstring> j_chooser_title =
      ConvertUTF16ToJavaString(env, d_chooser_title);
  ScopedJavaLocalRef<jstring> j_file_to_attach =
      ConvertUTF16ToJavaString(env, d_file_to_attach);
  Java_IntentHelper_sendEmail(env,
                              base::android::GetApplicationContext(),
                              j_email.obj(),
                              j_subject.obj(),
                              j_body.obj(),
                              j_chooser_title.obj(),
                              j_file_to_attach.obj());
}

void OpenDateAndTimeSettings() {
  JNIEnv* env = AttachCurrentThread();
  Java_IntentHelper_openDateAndTimeSettings(env,
      base::android::GetApplicationContext());
}

// sendAndroidBroadcast
void SendAndroidBroadcastJNI(const base::string16& d_action, int process_id_, int routing_id_) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> j_action = ConvertUTF16ToJavaString(env, d_action);
  Java_IntentHelper_sendAndroidBroadcast(env,
                              base::android::GetApplicationContext(),
                              j_action.obj(), process_id_, routing_id_);
}

bool RegisterIntentHelper(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace android
}  // namespace chrome
