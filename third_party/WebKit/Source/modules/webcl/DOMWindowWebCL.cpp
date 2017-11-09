// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "modules/webcl/DOMWindowWebCL.h"

#include "core/frame/LocalDOMWindow.h"
#include "modules/webcl/WebCL.h"

namespace blink {

DOMWindowWebCL::DOMWindowWebCL(LocalDOMWindow& window)
    : Supplement<LocalDOMWindow>(window)
{
}

const char* DOMWindowWebCL::supplementName()
{
    return "DOMWindowWebCL";
}

DOMWindowWebCL& DOMWindowWebCL::From(LocalDOMWindow& window)
{
	DOMWindowWebCL* supplement = static_cast<DOMWindowWebCL*>(Supplement<LocalDOMWindow>::From(window, supplementName()));
    if (!supplement) {
        supplement = new DOMWindowWebCL(window);
        ProvideTo(window, supplementName(), supplement);
    }
    return *supplement;
}

WebCL* DOMWindowWebCL::webcl(DOMWindow& window)
{
	return DOMWindowWebCL::From(ToLocalDOMWindow(window)).webclCreate(window.GetExecutionContext());
}

WebCL* DOMWindowWebCL::webclCreate(ExecutionContext* context) const
{
    if (!m_webcl && GetSupplementable()->GetFrame())
    	m_webcl = WebCL::create(context);
    return m_webcl.Get();
}

DEFINE_TRACE(DOMWindowWebCL)
{
	visitor->Trace(m_webcl);
	Supplement<LocalDOMWindow>::Trace(visitor);
}

} // namespace blink
