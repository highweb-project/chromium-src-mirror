// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_MESSAGING_DEVICE_MESSAGING_EXPORT_H_
#define DEVICE_MESSAGING_DEVICE_MESSAGING_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(DEVICE_MESSAGING_IMPLEMENTATION)
#define DEVICE_MESSAGING_EXPORT __declspec(dllexport)
#else
#define DEVICE_MESSAGING_EXPORT __declspec(dllimport)
#endif  // defined(DEVICE_MESSAGING_IMPLEMENTATION)

#else // defined(WIN32)
#if defined(DEVICE_MESSAGING_IMPLEMENTATION)
#define DEVICE_MESSAGING_EXPORT __attribute__((visibility("default")))
#else
#define DEVICE_MESSAGING_EXPORT
#endif
#endif

#else // defined(COMPONENT_BUILD)
#define DEVICE_MESSAGING_EXPORT
#endif

#endif  // DEVICE_MESSAGING_DEVICE_MESSAGING_EXPORT_H_
