// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/storage/devicestorage_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

namespace {

class DeviceStorageManagerEmptyImpl : public mojom::DeviceStorageManager {
 public:

  explicit DeviceStorageManagerEmptyImpl(mojom::DeviceStorageManagerRequest request)
     : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~DeviceStorageManagerEmptyImpl() override {}

  void getDeviceStorage(getDeviceStorageCallback callback) override {
    mojom::DeviceStorage_ResultCodePtr result(mojom::DeviceStorage_ResultCode::New());
    result->resultCode = int32_t(mojom::device_storage_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(mojom::device_storage_function::FUNC_GET_DEVICE_STORAGE);
    callback.Run(result.Clone());
  }

 private:
  friend DeviceStorageManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<mojom::DeviceStorageManager> binding_;
};

}  // namespace

// static
void DeviceStorageManagerImpl::Create(mojom::DeviceStorageManagerRequest request) {
  new DeviceStorageManagerEmptyImpl(std::move(request));
}

}  // namespace device
