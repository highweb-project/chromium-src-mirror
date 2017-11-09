#include "chrome/browser/device_api/applauncher/applauncher_api_handler_impl.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "content/public/browser/web_contents.h"
#include "base/callback.h"
#include <utility>
#include "extensions/browser/extension_system.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension_set.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_ui_util.h"
#include "extensions/common/constants.h"
#include "chrome/browser/ui/webui/extensions/extension_icon_source.h"
#include "chrome/common/extensions/manifest_handlers/app_launch_info.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/extensions/app_launch_params.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/extensions/application_launch.h"


ApplauncherApiHandlerImpl* ApplauncherApiHandlerImpl::GetInstance()
{
  DLOG(INFO) << "ApplauncherApiHandlerImpl::GetInstance()";
  return base::Singleton<ApplauncherApiHandlerImpl>::get();
}

void ApplauncherApiHandlerImpl::RequestFunction(content::WebContents* web_contents, const content::DeviceApiApplauncherRequest& request)
{
  Profile* profile = Profile::FromBrowserContext(web_contents->GetBrowserContext());
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
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_FAILURE;
  
  if (!profile) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
  }
  ExtensionService* service = extensions::ExtensionSystem::Get(profile)->extension_service();
  if (!service) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
  }
  extensions::ExtensionRegistry* registry = extensions::ExtensionRegistry::Get(profile);
  if (!registry) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
  }

  std::set<std::string> visible_apps;
  const extensions::ExtensionSet& enabled_set = registry->enabled_extensions();
  for(extensions::ExtensionSet::const_iterator it = enabled_set.begin();
      it != enabled_set.end(); ++it) {
    visible_apps.insert((*it)->id());
  }

  const extensions::ExtensionSet& disabled_set = registry->disabled_extensions();
  for(extensions::ExtensionSet::const_iterator it = disabled_set.begin();
      it != disabled_set.end(); ++it) {
    visible_apps.insert((*it)->id());
  }

  const extensions::ExtensionSet& terminated_set = registry->terminated_extensions();
  for(extensions::ExtensionSet::const_iterator it = terminated_set.begin();
      it != terminated_set.end(); ++it) {
    visible_apps.insert((*it)->id());
  }

  for(std::set<std::string>::iterator it = visible_apps.begin();
      it != visible_apps.end(); ++it) {
    const extensions::Extension* extension = service->GetInstalledExtension(*it);
    if (extension && extensions::ui_util::ShouldDisplayInNewTabPage(extension, profile) && 
        extension->id() != extensions::kWebStoreAppId) {
      GURL url = extensions::AppLaunchInfo::GetFullLaunchURL(extension);
      if (url.is_empty()) {
        continue;
      }
      result.applist.push_back(getApplicationInfoFromExtension(extension, url));
    }
  }
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
  request.callback_.Run(result);
}

void ApplauncherApiHandlerImpl::launchApp(Profile* profile, const content::DeviceApiApplauncherRequest& request) {
  content::DeviceApiApplauncherRequestResult result;
  result.functionCode = request.functionCode;
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_FAILURE;
  
  if (request.appId == extensions::kWebStoreAppId) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_INVALID_APP_ID);
  }

  if (!profile) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
    return;
  }
  
  ExtensionService* service = extensions::ExtensionSystem::Get(profile)->extension_service();
  if (!service) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
    return;
  }
  
  const extensions::Extension* extension = service->GetExtensionById(request.appId, false);
  if (!extension) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_NOT_INSTALLED_APP);
    return;
  }

  AppLaunchParams params = CreateAppLaunchParamsUserContainer(
    profile, extension, WindowOpenDisposition::NEW_FOREGROUND_TAB, extensions::SOURCE_NEW_TAB_PAGE);
  content::WebContents* new_contents = OpenApplication(params);
  
  if (new_contents) {
    result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
  }
  request.callback_.Run(result);
}

void ApplauncherApiHandlerImpl::removeApp(Profile* profile, const content::DeviceApiApplauncherRequest& request) {
  content::DeviceApiApplauncherRequestResult result;
  result.functionCode = request.functionCode;
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_FAILURE;
  
  if (request.appId == extensions::kWebStoreAppId) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_INVALID_APP_ID);
  }

  if (!profile) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
    return;
  }
  
  ExtensionService* service = extensions::ExtensionSystem::Get(profile)->extension_service();
  if (!service) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
    return;
  }

  const extensions::Extension* extension = service->GetInstalledExtension(request.appId);
  if (!extension) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_NOT_INSTALLED_APP);
    return;
  }

  if (!extensions::ExtensionSystem::Get(profile)->
          management_policy()->UserMayModifySettings(extension, NULL)) {
    LOG(ERROR) << "Attempt to uninstall an app that is non-usermanagable "
               << "was made. app id : " << extension->id();
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_NOT_ENABLED_PERMISSION);
    return;
  }

  service->UninstallExtension(
      request.appId, extensions::UNINSTALL_REASON_USER_INITIATED, 
      base::Bind(&base::DoNothing), nullptr);

  result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
  request.callback_.Run(result);
}

void ApplauncherApiHandlerImpl::getApplicationInfo(
    Profile* profile, const content::DeviceApiApplauncherRequest& request) {
  content::DeviceApiApplauncherRequestResult result;
  result.functionCode = request.functionCode;
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_FAILURE;
  
  if (request.appId == extensions::kWebStoreAppId) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_INVALID_APP_ID);
  }

  if (!profile) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
    return;
  }
  
  ExtensionService* service = extensions::ExtensionSystem::Get(profile)->extension_service();
  if (!service) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_FAILURE);
    return;
  }

  const extensions::Extension* extension = service->GetInstalledExtension(request.appId);
  if (!extension) {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_NOT_INSTALLED_APP);
    return;
  }

  if (extension && extensions::ui_util::ShouldDisplayInNewTabPage(extension, profile) && 
      extension->id() != extensions::kWebStoreAppId) {
    GURL url = extensions::AppLaunchInfo::GetFullLaunchURL(extension);
    if (url.is_empty()) {
      throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_INVALID_APP_ID);
      return;
    }
    result.applist.push_back(getApplicationInfoFromExtension(extension, url));
  } else {
    throwErrorMessage(request, content::applauncher_code_list::APPLAUNCHER_INVALID_APP_ID);
    return;
  }
  result.resultCode = content::applauncher_code_list::APPLAUNCHER_SUCCESS;
  request.callback_.Run(result);
}

void ApplauncherApiHandlerImpl::throwErrorMessage(const content::DeviceApiApplauncherRequest& request, int32_t error_message) {
  content::DeviceApiApplauncherRequestResult result;
  result.resultCode = error_message;
  result.functionCode = request.functionCode;
  request.callback_.Run(result);
}

ApplauncherApiHandlerImpl::ApplauncherApiHandlerImpl()
{
}

ApplauncherApiHandlerImpl::~ApplauncherApiHandlerImpl()
{
}

content::AppLauncher_ApplicationInfo ApplauncherApiHandlerImpl::getApplicationInfoFromExtension(
      const extensions::Extension* extension, GURL url) {
  content::AppLauncher_ApplicationInfo info;
  if (extension->manifest() && extension->manifest()->HasKey("icon_url")) {
    std::string icon_url;
    extension->manifest()->GetString("icon_url", &icon_url);
    info.iconUrl = icon_url;
  } else {
    GURL icon = extensions::ExtensionIconSource::GetIconURL(
                  extension, extension_misc::EXTENSION_ICON_LARGE,
                  ExtensionIconSet::MATCH_BIGGER, false);
    info.iconUrl = icon.spec();
  }
  info.id = extension->id();
  info.name = extension->name();
  info.version = extension->GetVersionForDisplay();
  info.url = url.spec();

  return info;
}