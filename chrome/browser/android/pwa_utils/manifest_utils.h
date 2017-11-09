#ifndef CHROME_BROWSER_ANDROID_PWA_UTILS_MANIFEST_UTILS_H_
#define CHROME_BROWSER_ANDROID_PWA_UTILS_MANIFEST_UTILS_H_

#include "base/strings/string16.h"
#include "base/time/time.h"
#include "url/gurl.h"
#include "base/android/jni_android.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "chrome/grit/browser_resources.h"
#include "ui/base/resource/resource_bundle.h"

namespace base{
  class DictionaryValue;
  class FilePath;
}

namespace chrome {
namespace android {

class ManifestUtils {
  public:
    static std::string ConvertTimeToExtensionVersion(const base::Time& create_time);
    static std::string GenerateKey(const GURL& app_url);
    static std::string GenerateId(const std::string& key, const std::string& url);
    static std::unique_ptr<base::DictionaryValue> GetDictionaryValue(const base::FilePath& path);
    static bool serializeDictionaryValue(const base::FilePath& path, base::DictionaryValue* value);
    static bool launchPWA(const base::DictionaryValue* value);
    static bool launchShortcut(const base::DictionaryValue* value);
    static bool createWebappDataStorage(std::string& id);
    static void deleteWebappDataStorage(std::string id);

    static bool launchApp(std::string appId);
    static bool uninstallApp(std::string appId);
    
    static base::FilePath GetExtensionListPath();
    static base::FilePath GetExtensionDataFilePath(std::string extensionId);
    static std::unique_ptr<base::DictionaryValue> getDefaultAppFromResource(int resourceId, std::string app_id);
  private:
    ManifestUtils();
    static bool ParsePEMKeyBytes(const std::string& input,
                                  std::string* output);
    static void ConvertHexadecimalToIDAlphabet(std::string* id);
    static std::string GenerateIdFromHex(const std::string& input);
    static std::string GenerateIdInternal(const std::string& input);
};

}  // namespace android
}  // namespace chrome

#endif  // CHROME_BROWSER_ANDROID_PWA_UTILS_MANIFEST_UTILS_H_
