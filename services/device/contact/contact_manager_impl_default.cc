// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/contact/contact_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "services/device/public/interfaces/contact_manager.mojom.h"

namespace device {

namespace {

class ContactManagerEmptyImpl : public mojom::ContactManager {
 public:
  explicit ContactManagerEmptyImpl(mojom::ContactManagerRequest request)
     : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~ContactManagerEmptyImpl() override {}

  void FindContact(
    int32_t requestID, uint32_t target, uint32_t maxItem,
    const std::string& condition, FindContactCallback callback) override {
    base::Optional<std::vector<mojom::ContactObjectPtr>> data;
    std::move(callback).Run(requestID, (unsigned)mojom::contact_ErrorCodeList::NOT_SUPPORT_API, std::move(data));
  }
  void AddContact(int32_t requestID, mojom::ContactObjectPtr contact, AddContactCallback callback) override {
    base::Optional<std::vector<mojom::ContactObjectPtr>> data;
    std::move(callback).Run(requestID, (unsigned)mojom::contact_ErrorCodeList::NOT_SUPPORT_API, std::move(data));
  }
  void DeleteContact(int32_t requestID, uint32_t target,
    uint32_t maxItem, const std::string& condition, DeleteContactCallback callback) override {
    base::Optional<std::vector<mojom::ContactObjectPtr>> data;
    std::move(callback).Run(requestID, (unsigned)mojom::contact_ErrorCodeList::NOT_SUPPORT_API, std::move(data));
  }
  void UpdateContact(int32_t requestID, mojom::ContactObjectPtr contact, UpdateContactCallback callback) override {
    base::Optional<std::vector<mojom::ContactObjectPtr>> data;
    std::move(callback).Run(requestID, (unsigned)mojom::contact_ErrorCodeList::NOT_SUPPORT_API, std::move(data));
  }
 private:
  friend ContactManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<mojom::ContactManager> binding_;
};

}  // namespace

// static
void ContactManagerImpl::Create(mojom::ContactManagerRequest request) {
  new ContactManagerEmptyImpl(std::move(request));
}

}  // namespace device
