// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ntp/android_app_launcher_handler.h"

#include <stddef.h>

#include <vector>

#include "base/android/path_utils.h"
#include "base/command_line.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram_macros.h"
#include "base/values.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/common/chrome_switches.h"
#include "content/public/browser/notification_service.h"

#include "chrome/browser/android/pwa_utils/manifest_utils.h"

using content::WebContents;

namespace highweb_pwa {
const char kHighwebPwaAppId[] = "jdccbhhlabdfagkgmocpancoeciocojb";
}

namespace {

// The purpose of this enum is to track which page on the NTP is showing.
// The lower 10 bits of kNtpShownPage are used for the index within the page
// group, and the rest of the bits are used for the page group ID (defined
// here).
static const int kPageIdOffset = 10;
enum {
  INDEX_MASK = (1 << kPageIdOffset) - 1,
  APPS_PAGE_ID = 2 << kPageIdOffset,
};

void RecordAppLauncherPromoHistogram(
      apps::AppLauncherPromoHistogramValues value) {
  DCHECK_LT(value, apps::APP_LAUNCHER_PROMO_MAX);
  UMA_HISTOGRAM_ENUMERATION(
      "Apps.AppLauncherPromo", value, apps::APP_LAUNCHER_PROMO_MAX);
}

}  // namespace

AppLauncherHandler::AppLauncherHandler() {
    RecordAppLauncherPromoHistogram(apps::APP_LAUNCHER_PROMO_ALREADY_INSTALLED);
    RecordAppLauncherPromoHistogram(apps::APP_LAUNCHER_PROMO_SHOWN);
}

AppLauncherHandler::~AppLauncherHandler() {
}

void AppLauncherHandler::CreateAppInfo(
    base::DictionaryValue* data,
    base::DictionaryValue* value) {
  value->Clear();

  value->SetBoolean(
      "kioskMode",
      base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kKioskMode));
  std::string id;
  std::string short_name;
  std::string name;
  std::string version;
  std::string description;
  std::string web_url;
  std::string scoped_url;
  std::string icon_url;
  std::string icon_color;
  int orientation;
  int display;
  int source;
  std::string background_color;

  data->GetString("id", &id);
  data->GetString("short_name", &short_name);
  data->GetString("name", &name);
  data->GetString("version", &version);
  data->GetString("description", &description);
  data->GetString("icon_url", &icon_url);
  base::DictionaryValue* app;
  data->GetDictionary("app", &app);
  if (app) {
    app->GetString("background_color", &background_color);
    app->GetString("icon_color", &icon_color);
    base::DictionaryValue* launch;
    app->GetDictionary("launch", &launch);
    if (launch) {
      launch->GetString("web_url", &web_url);
      launch->GetString("scoped_url", &scoped_url);
    }
    app->GetInteger("display", &display);
    app->GetInteger("orientation", &orientation);
    app->GetInteger("source", &source);
  }

  if (short_name.empty()) {
    value->SetString("title", name);
  } else {
    value->SetString("title", short_name);
  }
  value->SetString("direction", "ltr");
  value->SetString("full_name", name);
  value->SetString("full_name_direction", "ltr");
  bool enabled = true;

  value->SetString("id", id);
  value->SetString("name", name);
  value->SetBoolean("enabled", enabled);
  value->SetBoolean("kioskEnabled",
                   false);
  value->SetBoolean("kioskOnly",
                   false);
  value->SetBoolean("offlineEnabled",
                   false);
  value->SetString("version", version);
  value->SetString("description", description);
  value->SetString("optionsUrl", "");
  value->SetString("homepageUrl", "");
  value->SetString("detailsUrl", "");
  // bool icon_big_exists = true;
  // value->SetString("icon_big", icon_url);
  // value->SetBoolean("icon_big_exists", icon_big_exists);
  // bool icon_small_exists = false;
  // value->SetString("icon_small", "");
  // value->SetBoolean("icon_small_exists", icon_small_exists);
  value->SetString("icon", icon_url);
  value->SetBoolean("packagedApp", false);
  value->SetInteger("launch_container", 2);
  value->SetInteger("launch_type", 1);
  value->SetBoolean("is_component", true);
  value->SetBoolean("is_webstore", false);
  value->SetInteger("page_index", 0);
  value->SetString("app_launch_ordinal", version);
  
  bool disable = true;
  if (id == highweb_pwa::kHighwebPwaAppId) {
    disable = false;
  }
  value->SetBoolean("mayDisable", disable);
}

void AppLauncherHandler::RegisterMessages() {
  registrar_.Add(this, chrome::NOTIFICATION_APP_INSTALLED_TO_NTP,
      content::NotificationService::AllSources());
  registrar_.Add(this, chrome::NOTIFICATION_APP_UNINSTALL_TO_NTP,
      content::NotificationService::AllSources());

  // Some tests don't have a local state.
  web_ui()->RegisterMessageCallback("getApps",
      base::Bind(&AppLauncherHandler::HandleGetApps,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("launchApp",
      base::Bind(&AppLauncherHandler::HandleLaunchApp,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("uninstallApp",
      base::Bind(&AppLauncherHandler::HandleUninstallApp,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("setLaunchType",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("createAppShortcut",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("showAppInfo",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("reorderApps",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("setPageIndex",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("saveAppPageName",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("generateAppForLink",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("stopShowingAppLauncherPromo",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("onLearnMore",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback("pageSelected",
      base::Bind(&AppLauncherHandler::UnHandleMessage,
                 base::Unretained(this)));
}

void AppLauncherHandler::Observe(int type,
                                 const content::NotificationSource& source,
                                 const content::NotificationDetails& details) {
  if (type == chrome::NOTIFICATION_APP_INSTALLED_TO_NTP) {
    base::DictionaryValue* added_app = content::Details<base::DictionaryValue>(details).ptr();
    base::DictionaryValue added_app_info;
    CreateAppInfo(added_app, &added_app_info);
    added_app = nullptr;
    web_ui()->CallJavascriptFunctionUnsafe("ntp.appAdded", added_app_info);
  } else if(type == chrome::NOTIFICATION_APP_UNINSTALL_TO_NTP) {
    base::DictionaryValue* removed_app = content::Details<base::DictionaryValue>(details).ptr();
    AppRemoved(removed_app, true);
  }
}

void AppLauncherHandler::FillAppDictionary(base::DictionaryValue* dictionary) {
  std::unique_ptr<base::ListValue> list = base::MakeUnique<base::ListValue>();

  std::unique_ptr<base::DictionaryValue> manifest;
  for (std::set<std::string>::iterator it = visible_apps_.begin();
    it != visible_apps_.end(); ++it) {
    base::FilePath manifest_path = chrome::android::ManifestUtils::GetExtensionDataFilePath((std::string)*it);
    manifest = chrome::android::ManifestUtils::GetDictionaryValue(manifest_path);
    if (manifest) {
      list->Append(GetAppInfo(manifest.get()));
    } else if(*it == highweb_pwa::kHighwebPwaAppId) {
      manifest = chrome::android::ManifestUtils::getDefaultAppFromResource(IDR_HIGHWEB_PWA_MANIFEST, *it);
      if (manifest) {
        list->Append(GetAppInfo(manifest.get()));
      } else {
        LOG(WARNING) << "DefaultApp from resource manifest is null ";
      }
    } else {
      LOG(WARNING) << "manifest not available " << manifest_path.value();
    }
  }
  manifest.reset();

  dictionary->Set("appPageNames", list->CreateDeepCopy());
  dictionary->Set("apps", std::move(list));
}

std::unique_ptr<base::DictionaryValue> AppLauncherHandler::GetAppInfo(
  base::DictionaryValue* data)
{  std::unique_ptr<base::DictionaryValue> app_info(new base::DictionaryValue());
  CreateAppInfo(data, app_info.get());
  return app_info;
}

void AppLauncherHandler::HandleGetApps(const base::ListValue* args) {
  visible_apps_.insert(highweb_pwa::kHighwebPwaAppId);
  base::FilePath extensionlist_path = chrome::android::ManifestUtils::GetExtensionListPath();
  std::unique_ptr<base::DictionaryValue> extensionList = chrome::android::ManifestUtils::GetDictionaryValue(extensionlist_path);
  if (extensionList && !extensionList->empty()) {
    for(base::DictionaryValue::Iterator it(*extensionList);
      !it.IsAtEnd(); it.Advance()) {
        bool enabled = false;
        if (it.value().IsType(base::Value::Type::BOOLEAN)) {
          it.value().GetAsBoolean(&enabled);
        }
        if (enabled) {
          std::string key = it.key();
          visible_apps_.insert(key);
        }
    }
  }
  extensionList.reset();

  base::DictionaryValue dictionary;
  FillAppDictionary(&dictionary);
  web_ui()->CallJavascriptFunctionUnsafe("ntp.getAppsCallback", dictionary);
  
  has_loaded_apps_ = true;
}

void AppLauncherHandler::HandleLaunchApp(const base::ListValue* args) {
  std::string app_id;
  args->GetString(0, &app_id);
  if (!chrome::android::ManifestUtils::launchApp(app_id)) {
    LOG(ERROR) << "launchApp " << app_id << " launch fail";
  }
}

void AppLauncherHandler::HandleUninstallApp(const base::ListValue* args) {
  std::string extension_id;
  CHECK(args->GetString(0, &extension_id));
  if (chrome::android::ManifestUtils::uninstallApp(extension_id)) {
    std::unique_ptr<base::DictionaryValue> extension(new base::DictionaryValue());
    extension->SetString("id", extension_id);
    AppRemoved(extension.get(), true);
    extension.reset();
  }
}

void AppLauncherHandler::AppRemoved(const base::DictionaryValue* value, bool is_uninstall) {
  if (!value) {
    return;
  }
  web_ui()->CallJavascriptFunctionUnsafe(
    "ntp.appRemoved", *value, base::Value(is_uninstall),
    base::Value(true));
}

void AppLauncherHandler::UnHandleMessage(const base::ListValue* args) {
  LOG(WARNING) << "UnhandleMessage";
}