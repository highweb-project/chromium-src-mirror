// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MediaContent_h
#define MediaContent_h

#include "core/fileapi/Blob.h"
#include "platform/wtf/text/WTFString.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"

namespace blink {

class MediaContent final : public GarbageCollectedFinalized<MediaContent>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static MediaContent* Create() {return new MediaContent();}
    virtual ~MediaContent() {m_blob = nullptr;}

    Blob* blob() const { return m_blob; }
    void setBlob(Blob* value) { m_blob = value; }

    String uri() const { return m_uri; }
    void setUri(String value) { m_uri = value; }

    DEFINE_INLINE_TRACE() {visitor->Trace(m_blob);}

private:
    MediaContent(){setBlob(nullptr);setUri("");}

    Member<Blob> m_blob;
    String m_uri;
};

} // namespace blink

#endif // MediaContent_h