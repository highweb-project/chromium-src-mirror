// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_NTP_APP_LAUNCHER_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_NTP_APP_LAUNCHER_HANDLER_H_

#include <memory>
#include <set>
#include <string>

#include "apps/metrics_names.h"
#include "base/macros.h"
#include "base/task/cancelable_task_tracker.h"
#include "components/favicon/core/favicon_service.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/sync/model/string_ordinal.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "base/values.h"

class PrefChangeRegistrar;
class Profile;

namespace favicon_base {
struct FaviconImageResult;
}

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace highweb_pwa {
extern const char kHighwebPwaAppId[];
}

// The handler for Javascript messages related to the "apps" view.
class AppLauncherHandler
    : public content::WebUIMessageHandler,
      public content::NotificationObserver
      {
 public:
  explicit AppLauncherHandler();
  ~AppLauncherHandler() override;

  // Populate a dictionary with the information from an extension.
  static void CreateAppInfo(
    base::DictionaryValue* data,
      base::DictionaryValue* value);

  // Registers values (strings etc.) for the page.
  static void GetLocalizedValues(Profile* profile,
      base::DictionaryValue* values);

  // WebUIMessageHandler:
  void RegisterMessages() override;

  // content::NotificationObserver:
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // Populate the given dictionary with all installed app info.
  void FillAppDictionary(base::DictionaryValue* value);

  // Create a dictionary value for the given extension. May return null, e.g. if
  // the given extension is not an app.
  std::unique_ptr<base::DictionaryValue> GetAppInfo(base::DictionaryValue* data);

  // Handles the "launchApp" message with unused |args|.
  void HandleGetApps(const base::ListValue* args);

  // Handles the "launchApp" message with |args| containing [extension_id,
  // source] with optional [url, disposition], |disposition| defaulting to
  // CURRENT_TAB.
  void HandleLaunchApp(const base::ListValue* args);

  // Handles the "uninstallApp" message with |args| containing [extension_id]
  // and an optional bool to not confirm the uninstall when true, defaults to
  // false.
  void HandleUninstallApp(const base::ListValue* args);

  void UnHandleMessage(const base::ListValue* args);

 private:

  // Called when an app is removed (unloaded or uninstalled). Updates the UI.
  void AppRemoved(const base::DictionaryValue* value, bool is_uninstall);

  // We monitor changes to the extension system so that we can reload the apps
  // when necessary.
  content::NotificationRegistrar registrar_;

  // The ids of apps to show on the NTP.
  std::set<std::string> visible_apps_;

  // True if we have executed HandleGetApps() at least once.
  bool has_loaded_apps_;

  DISALLOW_COPY_AND_ASSIGN(AppLauncherHandler);
};

#endif  // CHROME_BROWSER_UI_WEBUI_NTP_APP_LAUNCHER_HANDLER_H_
