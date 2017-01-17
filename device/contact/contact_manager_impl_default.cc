// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/contact/contact_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "device/contact/contact_manager.mojom.h"
#include "mojo/public/cpp/bindings/array.h"

namespace device {

namespace {

class ContactManagerEmptyImpl : public ContactManager {
 public:
  //result code list
   const unsigned UNKNOWN_ERROR = 0;
   const unsigned INVALID_ARGUMENT_ERROR = 1;
   const unsigned TIMEOUT_ERROR = 3;
   const unsigned PENDING_OPERATION_ERROR = 4;
   const unsigned IO_ERROR = 5;
   const unsigned NOT_SUPPORTED_ERROR = 6;
   const unsigned PERMISSION_DENIED_ERROR = 20;
   const unsigned MESSAGE_SIZE_EXCEEDED = 30;
   const unsigned SUCCESS = 40;
   const unsigned NOT_SUPPORT_API = 9999;

  explicit ContactManagerEmptyImpl(ContactManagerRequest request)
     : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~ContactManagerEmptyImpl() override {}

  void FindContact(
    int32_t requestID, uint32_t target, uint32_t maxItem,
    const std::string& condition, const FindContactCallback& callback) override {
    base::Optional<std::vector<ContactObjectPtr>> data;
    callback.Run(requestID, NOT_SUPPORT_API, std::move(data));
  }
  void AddContact(int32_t requestID, ContactObjectPtr contact, const AddContactCallback& callback) override {
    base::Optional<std::vector<ContactObjectPtr>> data;
    callback.Run(requestID, NOT_SUPPORT_API, std::move(data));
  }
  void DeleteContact(int32_t requestID, uint32_t target,
    uint32_t maxItem, const std::string& condition, const DeleteContactCallback& callback) override {
    base::Optional<std::vector<ContactObjectPtr>> data;
    callback.Run(requestID, NOT_SUPPORT_API, std::move(data));
  }
  void UpdateContact(int32_t requestID, ContactObjectPtr contact, const UpdateContactCallback& callback) override {
    base::Optional<std::vector<ContactObjectPtr>> data;
    callback.Run(requestID, NOT_SUPPORT_API, std::move(data));
  }
 private:
  friend ContactManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<ContactManager> binding_;
};

}  // namespace

// static
void ContactManagerImpl::Create(ContactManagerRequest request) {
  new ContactManagerEmptyImpl(std::move(request));
}

}  // namespace device
