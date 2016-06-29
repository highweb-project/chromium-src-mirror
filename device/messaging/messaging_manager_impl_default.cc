// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/messaging/messaging_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

namespace {

class MessagingManagerEmptyImpl : public MessagingManager {
 public:
 	void SendMessage(MessageObjectPtr message) override {}
 	void FindMessage(uint32_t requestID, uint32_t target, uint32_t maxItem, const mojo::String& condition, const FindMessageCallback& callback) override {}
 	void AddMessagingListener(uint32_t observerID, const AddMessagingListenerCallback& callback) override {}
 	void RemoveMessagingListener(uint32_t observerID) override {}
 private:
  friend MessagingManagerImpl;

  explicit MessagingManagerEmptyImpl(
      mojo::InterfaceRequest<MessagingManager> request)
      : binding_(this, std::move(request)) {}
  ~MessagingManagerEmptyImpl() override {}

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<MessagingManager> binding_;
};

}  // namespace

// static
void MessagingManagerImpl::Create(
    mojo::InterfaceRequest<MessagingManager> request) {
  new MessagingManagerEmptyImpl(std::move(request));
}

}  // namespace device
