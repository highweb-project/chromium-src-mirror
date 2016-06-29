// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_GALLERY_GALLERY_MANAGER_IMPL_H_
#define DEVICE_GALLERY_GALLERY_MANAGER_IMPL_H_

#include "device/gallery/devicegallery_export.h"
#include "device/gallery/devicegallery_manager.mojom.h"

namespace device {

class DeviceGalleryManagerImpl {
 public:
  DEVICE_GALLERY_EXPORT static void Create(
      mojo::InterfaceRequest<DeviceGalleryManager> request);
};

}  // namespace device

#endif  // DEVICE_GALLERY_GALLERY_MANAGER_IMPL_H_
