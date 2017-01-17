// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_CALENDAR_DEVICE_CALENDAR_EXPORT_H_
#define DEVICE_CALENDAR_DEVICE_CALENDAR_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(DEVICE_CALENDAR_IMPLEMENTATION)
#define DEVICE_CALENDAR_EXPORT __declspec(dllexport)
#else
#define DEVICE_CALENDAR_EXPORT __declspec(dllimport)
#endif  // defined(DEVICE_CALENDAR_IMPLEMENTATION)

#else // defined(WIN32)
#if defined(DEVICE_CALENDAR_IMPLEMENTATION)
#define DEVICE_CALENDAR_EXPORT __attribute__((visibility("default")))
#else
#define DEVICE_CALENDAR_EXPORT
#endif
#endif

#else // defined(COMPONENT_BUILD)
#define DEVICE_CALENDAR_EXPORT
#endif

#endif  // DEVICE_CALENDAR_DEVICE_CALENDAR_EXPORT_H_
