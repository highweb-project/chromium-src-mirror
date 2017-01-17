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

  explicit MessagingManagerEmptyImpl(MessagingManagerRequest request)
     : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~MessagingManagerEmptyImpl() override {}

 	void SendMessage(MessageObjectPtr message) override {}
 	void FindMessage(uint32_t requestID, uint32_t target, uint32_t maxItem, const std::string& condition, const FindMessageCallback& callback) override {
    callback.Run(requestID, 9999, base::Optional<std::vector<MessageObjectPtr>>());
  }
 	void AddMessagingListener(uint32_t observerID, const AddMessagingListenerCallback& callback) override {}
 	void RemoveMessagingListener(uint32_t observerID) override {}
 private:
  friend MessagingManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<MessagingManager> binding_;
};

}  // namespace

// static
void MessagingManagerImpl::Create(MessagingManagerRequest request) {
  new MessagingManagerEmptyImpl(std::move(request));
}

}  // namespace device
