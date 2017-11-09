#include "chrome/browser/android/device_api/android_applauncher_api_handler_impl.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/values.h"
#include "content/public/browser/web_contents.h"
#include "base/callback.h"
#include <utility>
#include "chrome/browser/android/pwa_utils/manifest_utils.h"
#include "content/public/browser/notification_service.h"
#include "chrome/browser/chrome_notification_types.h"

#include "chrome/browser/ui/webui/ntp/android_app_launcher_handler.h"

ApplauncherApiHandlerImpl* ApplauncherApiHandlerImpl::GetInstance()
{
  return base::Singleton<ApplauncherApiHandlerImpl>::get();
}

void ApplauncherApiHandlerImpl::RequestFunction(Profile* profile, const content::DeviceApiApplauncherRequest& request)
{
  switch(request.functionCode) {
    case content::applauncher_function::APPLAUNCHER_FUNC_GET_APP_LIST: {
      getAppList(profile, request);
      break;
    }
    case content::applauncher_function::APPLAUNCHER_FUNC_LAUNCH_APP: {
      launchApp(profile, request);
      break;
    }
    case content::applauncher_function::APPLAUNCHER_FUNC_REMOVE_APP: {
      removeApp(profile, request);
      break;
    }
    case content::applauncher_function::APPLAUNCHER_FUNC_GET_APPLICATION_INFO: {
      getApplicationInfo(profile, request);
      break;
    }
    default: {
      throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_NOT_SUPPORT_API);
      break;
    }
  }
}

void ApplauncherApiHandlerImpl::getAppList(Profile* profile, const content::DeviceApiApplauncherRequest& request) {
  content::DeviceApiApplauncherRequestResult result;
  result.functionCode = request.functionCode;
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_NOT_INSTALLED_APP;

  base::FilePath extensionlist_path = chrome::android::ManifestUtils::GetExtensionListPath();
  std::unique_ptr<base::DictionaryValue> extensionList = chrome::android::ManifestUtils::GetDictionaryValue(extensionlist_path);
  std::unique_ptr<base::DictionaryValue> defaultApp = 
      chrome::android::ManifestUtils::getDefaultAppFromResource(IDR_HIGHWEB_PWA_MANIFEST, highweb_pwa::kHighwebPwaAppId);
  if (extensionList && !extensionList->empty()) {
    if (!extensionList->HasKey(highweb_pwa::kHighwebPwaAppId)) {
      content::AppLauncher_ApplicationInfo Defaultinfo;
      if (defaultApp && getAppInfo(defaultApp.get(), &Defaultinfo)) {
        result.applist.push_back(Defaultinfo);
      }
    }
    for(base::DictionaryValue::Iterator it(*extensionList);
        !it.IsAtEnd(); it.Advance()) {
      bool enabled = false;
      if (it.value().IsType(base::Value::Type::BOOLEAN)) {
        it.value().GetAsBoolean(&enabled);
      }
      if (enabled) {
        base::FilePath manifest_path = chrome::android::ManifestUtils::GetExtensionDataFilePath(it.key());
        std::unique_ptr<base::DictionaryValue> manifest = chrome::android::ManifestUtils::GetDictionaryValue(manifest_path);
        content::AppLauncher_ApplicationInfo info;
        if (getAppInfo(manifest.get(), &info)) {
          result.applist.push_back(info);
        } else {
          LOG(WARNING) << "manifest not available " << manifest_path.value();
          result.resultCode = content::applauncher_code_list::APPLAUNCHER_FAILURE;
          break;
        }
        manifest.reset();
      }
    }
  } else {
    content::AppLauncher_ApplicationInfo Defaultinfo;
    if (defaultApp && getAppInfo(defaultApp.get(), &Defaultinfo)) {
      result.applist.push_back(Defaultinfo);
    }
  }
  extensionList.reset();
  defaultApp.reset();
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
  request.callback_.Run(result);
}

void ApplauncherApiHandlerImpl::launchApp(Profile* profile, const content::DeviceApiApplauncherRequest& request) {
  content::DeviceApiApplauncherRequestResult result;
  result.functionCode = request.functionCode;
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_NOT_INSTALLED_APP;

  if (chrome::android::ManifestUtils::launchApp(request.appId)) {
    result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
  }

  request.callback_.Run(result);
}

void ApplauncherApiHandlerImpl::removeApp(Profile* profile, const content::DeviceApiApplauncherRequest& request) {
  content::DeviceApiApplauncherRequestResult result;
  result.functionCode = request.functionCode;
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_NOT_INSTALLED_APP;

  if (chrome::android::ManifestUtils::uninstallApp(request.appId)) {
    std::unique_ptr<base::DictionaryValue> appValue(new base::DictionaryValue());
    appValue->SetString("id", request.appId);
    if (request.appId != highweb_pwa::kHighwebPwaAppId) {
      content::NotificationService::current()->Notify(chrome::NOTIFICATION_APP_UNINSTALL_TO_NTP,
        content::NotificationService::AllSources(), content::Details<base::DictionaryValue>(appValue.get()));
    }
    appValue.reset();
    result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
  }

  request.callback_.Run(result);
}

void ApplauncherApiHandlerImpl::getApplicationInfo(
    Profile* profile,
    const content::DeviceApiApplauncherRequest& request) {
  content::DeviceApiApplauncherRequestResult result;
  result.functionCode = request.functionCode;
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_NOT_INSTALLED_APP;

  base::FilePath extensionlist_path = chrome::android::ManifestUtils::GetExtensionListPath();
  std::unique_ptr<base::DictionaryValue> extensionList = chrome::android::ManifestUtils::GetDictionaryValue(extensionlist_path);
  content::AppLauncher_ApplicationInfo info;
  
  if (extensionList && extensionList->HasKey(request.appId)) {
    bool enabled = false;
    extensionList->GetBoolean(request.appId, &enabled);
    if (enabled) {
      base::FilePath extension_path = chrome::android::ManifestUtils::GetExtensionDataFilePath(request.appId);
      std::unique_ptr<base::DictionaryValue> extension = chrome::android::ManifestUtils::GetDictionaryValue(extension_path);
      if (getAppInfo(extension.get(), &info)) {
        result.applist.push_back(info);
        result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
      }
      extension.reset();
    }
    extensionList.reset();
  } else if(request.appId == highweb_pwa::kHighwebPwaAppId) {
    std::unique_ptr<base::DictionaryValue> defaultApp = 
        chrome::android::ManifestUtils::getDefaultAppFromResource(IDR_HIGHWEB_PWA_MANIFEST, highweb_pwa::kHighwebPwaAppId);
    if (getAppInfo(defaultApp.get(), &info)) {
      result.applist.push_back(info);
      result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
    }
  } else {
    LOG(WARNING) << "extensionList not loaded";
  }
  request.callback_.Run(result);
}

void ApplauncherApiHandlerImpl::throwErrorMessage(const content::DeviceApiApplauncherRequest& request, int32_t error_message) {
  content::DeviceApiApplauncherRequestResult result;
  result.resultCode = error_message;
  result.functionCode = request.functionCode;
  request.callback_.Run(result);
}

bool ApplauncherApiHandlerImpl::getAppInfo(base::DictionaryValue* value, content::AppLauncher_ApplicationInfo* info) {
  if (value && !value->empty() && info) {
    value->GetString("id", &(info->id));
    value->GetString("name", &(info->name));
    value->GetString("version", &(info->version));
    value->GetString("icon_url", &(info->iconUrl));

    base::DictionaryValue* app;
    value->GetDictionary("app", &app);
    if (app) {
      base::DictionaryValue* launch;
      app->GetDictionary("launch", &launch);
      if (launch) {
        launch->GetString("web_url", &(info->url));
      }
      launch = nullptr;
    }
    app = nullptr;

    return true;
  } 
  LOG(WARNING) << "value not available";
  return false;
}

ApplauncherApiHandlerImpl::ApplauncherApiHandlerImpl()
{
}

ApplauncherApiHandlerImpl::~ApplauncherApiHandlerImpl()
{
}
