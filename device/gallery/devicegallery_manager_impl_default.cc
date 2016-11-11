// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/gallery/devicegallery_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "device/gallery/devicegallery_manager.mojom.h"
#include "device/gallery/devicegallery_ResultCode.mojom.h"

namespace device {

namespace {

class DeviceGalleryManagerEmptyImpl : public DeviceGalleryManager {
 public:
  void findMedia(MojoDeviceGalleryFindOptionsPtr options, const findMediaCallback& callback) override {
    DeviceGallery_ResultCodePtr result(DeviceGallery_ResultCode::New());
    result->resultCode = int32_t(device::device_gallery_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(device::device_gallery_function::FUNC_FIND_MEDIA);
    result->mediaListSize = 0;
    callback.Run(result.Clone());
  }
  void getMedia(MojoDeviceGalleryMediaObjectPtr object, const getMediaCallback& callback) override {
    DeviceGallery_ResultCodePtr result(DeviceGallery_ResultCode::New());
    result->resultCode = int32_t(device::device_gallery_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(device::device_gallery_function::FUNC_FIND_MEDIA);
    result->mediaListSize = 0;
    callback.Run(result.Clone());
  }
  void deleteMedia(MojoDeviceGalleryMediaObjectPtr object, const deleteMediaCallback& callback) override {
    DeviceGallery_ResultCodePtr result(DeviceGallery_ResultCode::New());
    result->resultCode = int32_t(device::device_gallery_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(device::device_gallery_function::FUNC_FIND_MEDIA);
    result->mediaListSize = 0;
    callback.Run(result.Clone());
  }

 private:
  friend DeviceGalleryManagerImpl;

  explicit DeviceGalleryManagerEmptyImpl(
      mojo::InterfaceRequest<DeviceGalleryManager> request)
      : binding_(this, std::move(request)) {}
  ~DeviceGalleryManagerEmptyImpl() override {}

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<DeviceGalleryManager> binding_;
};

}  // namespace

// static
void DeviceGalleryManagerImpl::Create(
    mojo::InterfaceRequest<DeviceGalleryManager> request) {
  new DeviceGalleryManagerEmptyImpl(std::move(request));
}

}  // namespace device
