#include "manifest_utils.h"

#include "base/logging.h"
#include "base/base64.h"
#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/android/path_utils.h"
#include "base/strings/stringprintf.h"
#include "base/files/file_util.h"
#include "base/values.h"
#include "base/json/json_file_value_serializer.h"
#include "crypto/sha2.h"

#include "jni/ManifestUtils_jni.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "ui/gfx/android/java_bitmap.h"

#include "base/threading/thread_restrictions.h"
#include "third_party/WebKit/public/platform/WebDisplayMode.h"

#include "chrome/browser/ui/webui/ntp/android_app_launcher_handler.h"
#include "base/json/json_string_value_serializer.h"

using base::Time;
using base::DictionaryValue;
using base::android::ScopedJavaLocalRef;

namespace chrome{
namespace android{

ManifestUtils::ManifestUtils() {
}
    
std::string ManifestUtils::ConvertTimeToExtensionVersion(const Time& create_time) {
  Time::Exploded create_time_exploded;
  create_time.UTCExplode(&create_time_exploded);

  double micros = static_cast<double>(
      (create_time_exploded.millisecond * Time::kMicrosecondsPerMillisecond) +
      (create_time_exploded.second * Time::kMicrosecondsPerSecond) +
      (create_time_exploded.minute * Time::kMicrosecondsPerMinute) +
      (create_time_exploded.hour * Time::kMicrosecondsPerHour));
  double day_fraction = micros / Time::kMicrosecondsPerDay;
  double stamp = day_fraction * std::numeric_limits<uint16_t>::max();

  // Ghetto-round, since VC++ doesn't have round().
  stamp = stamp >= (floor(stamp) + 0.5) ? (stamp + 1) : stamp;

  return base::StringPrintf(
      "%i.%02i.%02i.%05i", create_time_exploded.year, create_time_exploded.month,
      create_time_exploded.day_of_month, static_cast<uint16_t>(stamp));
}

std::string ManifestUtils::GenerateKey(const GURL& app_url) {
  char raw[crypto::kSHA256Length] = {0};
  std::string key;
  crypto::SHA256HashString(app_url.spec().c_str(), raw,
                           crypto::kSHA256Length);
  base::Base64Encode(base::StringPiece(raw, crypto::kSHA256Length), &key);
  return key;
}

bool ManifestUtils::ParsePEMKeyBytes(const std::string& input,
                                 std::string* output) {
  DCHECK(output);
  if (!output)
    return false;
  if (input.length() == 0)
    return false;

  std::string working = input;
  std::string kKeyBeginHeaderMarker = "-----BEGIN";
  std::string kKeyBeginFooterMarker = "-----END";
  if (base::StartsWith(working, kKeyBeginHeaderMarker,
                       base::CompareCase::SENSITIVE)) {
    working = base::CollapseWhitespaceASCII(working, true);
    size_t header_pos = working.find(kKeyBeginFooterMarker,
      sizeof(kKeyBeginHeaderMarker) - 1);
    if (header_pos == std::string::npos)
      return false;
    size_t start_pos = header_pos + sizeof(kKeyBeginFooterMarker) - 1;
    size_t end_pos = working.rfind(kKeyBeginFooterMarker);
    if (end_pos == std::string::npos)
      return false;
    if (start_pos >= end_pos)
      return false;

    working = working.substr(start_pos, end_pos - start_pos);
    if (working.length() == 0)
      return false;
  }

  return base::Base64Decode(working, output);
}

void ManifestUtils::ConvertHexadecimalToIDAlphabet(std::string* id) {
  for (size_t i = 0; i < id->size(); ++i) {
    int val;
    if (base::HexStringToInt(
            base::StringPiece(id->begin() + i, id->begin() + i + 1), &val)) {
      (*id)[i] = val + 'a';
    } else {
      (*id)[i] = 'a';
    }
  }
}

std::string ManifestUtils::GenerateId(const std::string& key, const std::string& url) {
  std::string extensionKeyBytes;
  if (!ManifestUtils::ParsePEMKeyBytes(key, &extensionKeyBytes)) {
    LOG(WARNING) << "PEM fail. key : " << key;
    return ManifestUtils::GenerateIdInternal(url);
  } else {
    return ManifestUtils::GenerateIdInternal(extensionKeyBytes);
  }
}

std::string ManifestUtils::GenerateIdInternal(const std::string& input) {
  uint8_t hash[16];
  crypto::SHA256HashString(input, hash, sizeof(hash));
  return GenerateIdFromHex(base::HexEncode(hash, sizeof(hash)));
}

std::string ManifestUtils::GenerateIdFromHex(const std::string& input) {
  std::string output = base::ToLowerASCII(input);
  ConvertHexadecimalToIDAlphabet(&output);
  return output;
}

std::unique_ptr<DictionaryValue> ManifestUtils::GetDictionaryValue(const base::FilePath& path) {
  base::ThreadRestrictions::SetIOAllowed(true);
  if (PathExists(path) && !path.empty()) {
    JSONFileValueDeserializer deserializer(path);
    return DictionaryValue::From(deserializer.Deserialize(nullptr, nullptr));
  } else {
    return nullptr;
  }
}

bool ManifestUtils::serializeDictionaryValue(const base::FilePath& path, DictionaryValue* value) {
  base::ThreadRestrictions::SetIOAllowed(true);
  if (path.empty() || (value == nullptr)) {
    LOG(WARNING) << "path.empty or value is nullptr : " << path.value() << "\n-" << *value;
    return false;
  }
  base::CreateDirectory(path.DirName());
  JSONFileValueSerializer serializer(path);
  if (!serializer.Serialize(*value)) {
      return false;
  } else {
    return true;
  }
  return true;
}

bool ManifestUtils::launchApp(std::string appId) {
  bool result = false;
  bool enabled;
  std::unique_ptr<base::DictionaryValue> extension;
  std::unique_ptr<base::DictionaryValue> extensionList;

  bool isHighwebDefaultAppId = (appId == highweb_pwa::kHighwebPwaAppId);
  
  base::FilePath extensionlist_path = chrome::android::ManifestUtils::GetExtensionListPath();
  extensionList = chrome::android::ManifestUtils::GetDictionaryValue(extensionlist_path);

  if (!extensionList && !isHighwebDefaultAppId) {
    LOG(WARNING) << "extensionList not loaded";
    return result;
  }
  if (extensionList && extensionList->HasKey(appId)) {
    extensionList->GetBoolean(appId, &enabled);
    if (enabled) {
      base::FilePath extension_path = chrome::android::ManifestUtils::GetExtensionDataFilePath(appId);
      extension = chrome::android::ManifestUtils::GetDictionaryValue(extension_path);
    }
  } else if(isHighwebDefaultAppId) {
    enabled = false;
    extension = getDefaultAppFromResource(IDR_HIGHWEB_PWA_MANIFEST, highweb_pwa::kHighwebPwaAppId);
  } else {
    LOG(WARNING) << "extensionList not has id " << appId;
    return result;
  } 
    
  if (extension) {
    int displayMode;
    base::DictionaryValue* apps;
    extension->GetDictionary("app", &apps);
    if (apps) {
      apps->GetInteger("display", &displayMode);
      if (displayMode == blink::kWebDisplayModeStandalone ||
        displayMode == blink::kWebDisplayModeFullscreen) {
          chrome::android::ManifestUtils::launchPWA(extension.get());
      } else {
        chrome::android::ManifestUtils::launchShortcut(extension.get());
      }
      result = true;
    }
  } else {
    LOG(WARNING) << "extension json file not available";
  }
  extension.reset();
  extensionList.reset();
  return result;
}

bool ManifestUtils::uninstallApp(std::string appId) {
  bool result = false;
  bool isHighwebDefaultAppId = (appId == highweb_pwa::kHighwebPwaAppId);

  base::FilePath extensionList_path = chrome::android::ManifestUtils::GetExtensionListPath();
  std::unique_ptr<base::DictionaryValue> extensionList = chrome::android::ManifestUtils::GetDictionaryValue(extensionList_path);
  if (!extensionList) {
    LOG(WARNING) << "ExtensionList not available";
    return result;
  }
  
  bool extension_enabled = true;
  base::FilePath extensionFolder_path;
  if (extensionList->HasKey(appId)) {
    extensionList->GetBoolean(appId, &extension_enabled);
    extensionFolder_path = chrome::android::ManifestUtils::GetExtensionDataFilePath(appId).DirName();
  } else if (isHighwebDefaultAppId) {
    extension_enabled = false;
    LOG(WARNING) << "isDefaultApp";
    result = false;
    extensionList.reset();
    return result;
  }

  if (!extension_enabled) {
    LOG(WARNING) << "Extension not enabled";
    return result;
  } else if (!DeleteFile(extensionFolder_path, true)) {
    LOG(WARNING) << "ExtensionFolder delete fail " << extensionFolder_path.value();
    return result;
  } else {
    chrome::android::ManifestUtils::deleteWebappDataStorage(appId);
    if (extensionList->Remove(appId, NULL)) {
      chrome::android::ManifestUtils::serializeDictionaryValue(extensionList_path, extensionList.get());
      result = true;
    } else {
      LOG(WARNING) << "extensionlist remove fail " << appId;
    }
    extensionList.reset();
  }
  return result;
}

bool ManifestUtils::launchPWA(const base::DictionaryValue* value) {
  JNIEnv* env = base::android::AttachCurrentThread();
  std::string id;
  std::string short_name;
  std::string name;
  std::string web_url;
  std::string scoped_url;
  std::string icon_url;
  std::string icon_color;
  int orientation;
  int display;
  int source;
  std::string background_color;

  value->GetString("id", &id);
  value->GetString("short_name", &short_name);
  value->GetString("name", &name);
  value->GetString("icons", &icon_url);
  const base::DictionaryValue* app;
  value->GetDictionary("app", &app);
  app->GetString("background_color", &background_color);
  app->GetString("icon_color", &icon_color);
  const base::DictionaryValue* launch;
  app->GetDictionary("launch", &launch);
  launch->GetString("web_url", &web_url);
  launch->GetString("scoped_url", &scoped_url);
  app->GetInteger("display", &display);
  app->GetInteger("orientation", &orientation);
  app->GetInteger("source", &source);
  
  ScopedJavaLocalRef<jstring> java_webapp_id =
      base::android::ConvertUTF8ToJavaString(env, id);
  ScopedJavaLocalRef<jstring> java_url =
      base::android::ConvertUTF8ToJavaString(env, web_url);
  ScopedJavaLocalRef<jstring> java_scope_url =
      base::android::ConvertUTF8ToJavaString(env, scoped_url);
  ScopedJavaLocalRef<jstring> java_user_title =
      base::android::ConvertUTF8ToJavaString(env, short_name);
  ScopedJavaLocalRef<jstring> java_name =
      base::android::ConvertUTF8ToJavaString(env, name);
  ScopedJavaLocalRef<jstring> java_short_name =
      base::android::ConvertUTF8ToJavaString(env, short_name);
  ScopedJavaLocalRef<jstring> java_best_icon_url =
      base::android::ConvertUTF8ToJavaString(env, icon_url);
  ScopedJavaLocalRef<jstring> java_icon_color =
      base::android::ConvertUTF8ToJavaString(env, icon_color);
  ScopedJavaLocalRef<jstring> java_background_color =
      base::android::ConvertUTF8ToJavaString(env, background_color);

  Java_ManifestUtils_launchPWA(env, java_webapp_id, java_url, java_scope_url,
                java_user_title, java_name, java_short_name, java_best_icon_url,
                display, orientation, source, java_icon_color, java_background_color);


  return true;
}

bool ManifestUtils::launchShortcut(const base::DictionaryValue* value) {
  JNIEnv* env = base::android::AttachCurrentThread();

  std::string web_url;
  int source;
  const base::DictionaryValue* app;
  value->GetDictionary("app", &app);
  const base::DictionaryValue* launch;
  app->GetDictionary("launch", &launch);
  launch->GetString("web_url", &web_url);
  app->GetInteger("source", &source);

  ScopedJavaLocalRef<jstring> java_url =
      base::android::ConvertUTF8ToJavaString(env, web_url);

  Java_ManifestUtils_launchShortcut(env, java_url, source);
  return true;
}

base::FilePath ManifestUtils::GetExtensionListPath() {
  base::FilePath data_path;
  base::android::GetDataDirectory(&data_path);
  return data_path.AppendASCII("highweb_extension/list.json");
}
base::FilePath ManifestUtils::GetExtensionDataFilePath(std::string extensionId) {
  base::FilePath data_path;
  base::android::GetDataDirectory(&data_path);
  return data_path.AppendASCII("highweb_extension/" + extensionId + "/manifest.json");
}

bool ManifestUtils::createWebappDataStorage(std::string& id) {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> java_webapp_id =
      base::android::ConvertUTF8ToJavaString(env, id);
  Java_ManifestUtils_createWebappDataStorage(env, java_webapp_id);

  return true;
}

void ManifestUtils::deleteWebappDataStorage(std::string id) {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> java_id =
      base::android::ConvertUTF8ToJavaString(env, id);

  Java_ManifestUtils_deleteWebappDataStorage(env, java_id);
}

std::unique_ptr<base::DictionaryValue> ManifestUtils::getDefaultAppFromResource(int resourceId, std::string app_id) {
  base::StringPiece manifest_contents =
  ResourceBundle::GetSharedInstance().GetRawDataResource(
      resourceId);
  if (!manifest_contents.empty()) {
    JSONStringValueDeserializer deserializer(manifest_contents);
    int error_code = 0;
    std::string error_str;
    std::unique_ptr<base::DictionaryValue> manifest = base::DictionaryValue::From(deserializer.Deserialize(&error_code, &error_str));
    if (manifest) {
      manifest->SetString("id", app_id);
      return manifest;
    } 
    LOG(WARNING) << "manifest deserializer fail " << error_code << ", " << error_str;
  }
  return nullptr;
}

}
}