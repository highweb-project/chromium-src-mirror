// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_ROUTER_DISCOVERY_DISCOVERY_NETWORK_LIST_WIFI_H_
#define CHROME_BROWSER_MEDIA_ROUTER_DISCOVERY_DISCOVERY_NETWORK_LIST_WIFI_H_

#include <string>

// TODO(crbug.com/713378): Implement for Mac.
bool MaybeGetWifiSSID(const std::string& if_name, std::string* ssid);

#endif  // CHROME_BROWSER_MEDIA_ROUTER_DISCOVERY_DISCOVERY_NETWORK_LIST_WIFI_H_
