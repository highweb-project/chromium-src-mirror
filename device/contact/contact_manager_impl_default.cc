// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/contact/contact_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

namespace {

class ContactManagerEmptyImpl : public ContactManager {
 public:
  void FindContact(int32_t requestID, uint32_t target, uint32_t maxItem, const mojo::String& condition, const FindContactCallback& callback) override {}
  void AddContact(int32_t requestID, ContactObjectPtr contact, const AddContactCallback& callback) override {}
  void DeleteContact(int32_t requestID, uint32_t target, uint32_t maxItem, const mojo::String& condition, const DeleteContactCallback& callback) override {}
  void UpdateContact(int32_t requestID, ContactObjectPtr contact, const UpdateContactCallback& callback) override {}
 private:
  friend ContactManagerImpl;

  explicit ContactManagerEmptyImpl(
      mojo::InterfaceRequest<ContactManager> request)
      : binding_(this, std::move(request)) {}
  ~ContactManagerEmptyImpl() override {}

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<ContactManager> binding_;
};

}  // namespace

// static
void ContactManagerImpl::Create(
    mojo::InterfaceRequest<ContactManager> request) {
  new ContactManagerEmptyImpl(std::move(request));
}

}  // namespace device
