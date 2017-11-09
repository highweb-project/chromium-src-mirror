// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/gallery/devicegallery_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "services/device/public/interfaces/devicegallery_manager.mojom.h"
#include "services/device/public/interfaces/devicegallery_ResultCode.mojom.h"

namespace device {

namespace {

class DeviceGalleryManagerEmptyImpl : public mojom::DeviceGalleryManager {
 public:

  explicit DeviceGalleryManagerEmptyImpl(mojom::DeviceGalleryManagerRequest request)
     : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~DeviceGalleryManagerEmptyImpl() override {}

  void findMedia(mojom::MojoDeviceGalleryFindOptionsPtr options, findMediaCallback callback) override {
    mojom::DeviceGallery_ResultCodePtr result(mojom::DeviceGallery_ResultCode::New());
    result->resultCode = int32_t(mojom::device_gallery_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(mojom::device_gallery_function::FUNC_FIND_MEDIA);
    result->mediaListSize = 0;
    std::move(callback).Run(result.Clone());
  }
  void getMedia(mojom::MojoDeviceGalleryMediaObjectPtr object, getMediaCallback callback) override {
    mojom::DeviceGallery_ResultCodePtr result(mojom::DeviceGallery_ResultCode::New());
    result->resultCode = int32_t(mojom::device_gallery_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(mojom::device_gallery_function::FUNC_FIND_MEDIA);
    result->mediaListSize = 0;
    std::move(callback).Run(result.Clone());
  }
  void deleteMedia(mojom::MojoDeviceGalleryMediaObjectPtr object, deleteMediaCallback callback) override {
    mojom::DeviceGallery_ResultCodePtr result(mojom::DeviceGallery_ResultCode::New());
    result->resultCode = int32_t(mojom::device_gallery_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(mojom::device_gallery_function::FUNC_FIND_MEDIA);
    result->mediaListSize = 0;
    std::move(callback).Run(result.Clone());
  }

 private:
  friend DeviceGalleryManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<mojom::DeviceGalleryManager> binding_;
};

}  // namespace

// static
void DeviceGalleryManagerImpl::Create(mojom::DeviceGalleryManagerRequest request) {
  new DeviceGalleryManagerEmptyImpl(std::move(request));
}

}  // namespace device
