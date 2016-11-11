// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOMWindowWebVKC_h
#define DOMWindowWebVKC_h

#include "core/frame/DOMWindowProperty.h"
#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"

namespace blink {

class WebVKC;
class DOMWindow;
class ExecutionContext;

class DOMWindowWebVKC final : public GarbageCollected<DOMWindowWebVKC>, public Supplement<LocalDOMWindow>, public DOMWindowProperty {
    USING_GARBAGE_COLLECTED_MIXIN(DOMWindowWebVKC);
public:
    static DOMWindowWebVKC& from(LocalDOMWindow&);
    static WebVKC* webvkc(DOMWindow&);
    WebVKC* webvkcCreate(ExecutionContext* context) const;

    DECLARE_TRACE();

private:
    explicit DOMWindowWebVKC(LocalDOMWindow&);
    static const char* supplementName();

    mutable Member<WebVKC> m_webvkc;
};

} // namespace blink

#endif // DOMWindowCrypto_h
