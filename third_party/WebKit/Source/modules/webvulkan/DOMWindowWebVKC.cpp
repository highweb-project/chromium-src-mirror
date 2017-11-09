// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "modules/webvulkan/DOMWindowWebVKC.h"

#include "core/frame/LocalDOMWindow.h"
#include "modules/webvulkan/WebVKC.h"

namespace blink {

DOMWindowWebVKC::DOMWindowWebVKC(LocalDOMWindow& window)
    : Supplement<LocalDOMWindow>(window)
{
}

const char* DOMWindowWebVKC::supplementName()
{
    return "DOMWindowWebVKC";
}

DOMWindowWebVKC& DOMWindowWebVKC::From(LocalDOMWindow& window)
{
	DOMWindowWebVKC* supplement = static_cast<DOMWindowWebVKC*>(Supplement<LocalDOMWindow>::From(window, supplementName()));
    if (!supplement) {
        supplement = new DOMWindowWebVKC(window);
        ProvideTo(window, supplementName(), supplement);
    }
    return *supplement;
}

WebVKC* DOMWindowWebVKC::webvkc(DOMWindow& window)
{
	return DOMWindowWebVKC::From(ToLocalDOMWindow(window)).webvkcCreate(window.GetExecutionContext());
}

WebVKC* DOMWindowWebVKC::webvkcCreate(ExecutionContext* context) const
{
    if (!m_webvkc && GetSupplementable()->GetFrame())
    	m_webvkc = WebVKC::create(context);
    return m_webvkc.Get();
}

DEFINE_TRACE(DOMWindowWebVKC)
{
	visitor->Trace(m_webvkc);
	Supplement<LocalDOMWindow>::Trace(visitor);
}

} // namespace blink
