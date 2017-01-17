// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "modules/webvulkan/DOMWindowWebVKC.h"

#include "core/frame/LocalDOMWindow.h"
#include "modules/webvulkan/WebVKC.h"

namespace blink {

DOMWindowWebVKC::DOMWindowWebVKC(LocalDOMWindow& window)
    : DOMWindowProperty(window.frame())
{
}

const char* DOMWindowWebVKC::supplementName()
{
    return "DOMWindowWebVKC";
}

DOMWindowWebVKC& DOMWindowWebVKC::from(LocalDOMWindow& window)
{
	DOMWindowWebVKC* supplement = static_cast<DOMWindowWebVKC*>(Supplement<LocalDOMWindow>::from(window, supplementName()));
    if (!supplement) {
        supplement = new DOMWindowWebVKC(window);
        provideTo(window, supplementName(), supplement);
    }
    return *supplement;
}

WebVKC* DOMWindowWebVKC::webvkc(DOMWindow& window)
{
	return DOMWindowWebVKC::from(toLocalDOMWindow(window)).webvkcCreate(window.getExecutionContext());
}

WebVKC* DOMWindowWebVKC::webvkcCreate(ExecutionContext* context) const
{
    if (!m_webvkc && frame())
    	m_webvkc = WebVKC::create(context);
    return m_webvkc.get();
}

DEFINE_TRACE(DOMWindowWebVKC)
{
	visitor->trace(m_webvkc);
	Supplement<LocalDOMWindow>::trace(visitor);
	DOMWindowProperty::trace(visitor);
}

} // namespace blink
