// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOMWindowWebCL_h
#define DOMWindowWebCL_h

#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"

namespace blink {

class WebCL;
class DOMWindow;
class LocalDOMWindow;
class ExecutionContext;

class DOMWindowWebCL final : public GarbageCollected<DOMWindowWebCL>, public Supplement<LocalDOMWindow> {
    USING_GARBAGE_COLLECTED_MIXIN(DOMWindowWebCL);
public:
    static DOMWindowWebCL& From(LocalDOMWindow&);
    static WebCL* webcl(DOMWindow&);
    WebCL* webclCreate(ExecutionContext* context) const;

    DECLARE_TRACE();

private:
    explicit DOMWindowWebCL(LocalDOMWindow&);
    static const char* supplementName();

    mutable Member<WebCL> m_webcl;
};

} // namespace blink

#endif // DOMWindowCrypto_h
