// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLMemoryUtil_h
#define WebCLMemoryUtil_h

#include "platform/heap/Handle.h"

#include "WebCLInclude.h"
#include "core/dom/custom/WebCL/WebCLException.h"

namespace blink {

class WebCL;
class WebCLBuffer;
class WebCLCommandQueue;
class WebCLContext;
class WebCLMemoryObject;
class WebCLKernel;
class WebCLObject;
class WebCLProgram;

// WebCLMemoryUtil is a helper class to intialize the OpenCL memory to 0.
// It leverages the OpenCL kernel function to do it.
class WebCLMemoryUtil : public GarbageCollectedFinalized<WebCLMemoryUtil> {
public:
    explicit WebCLMemoryUtil(WebCL*, WebCLContext*);
    ~WebCLMemoryUtil();

    void bufferCreated(WebCLBuffer*, ExceptionState&);
    void commandQueueCreated(WebCLCommandQueue*, ExceptionState&);

	DECLARE_TRACE();
private:
    void ensureMemory(WebCLMemoryObject*, WebCLCommandQueue*, ExceptionState&);
    void processPendingMemoryList(ExceptionState&);
    WebCLCommandQueue* validCommandQueue() const;
    void initializeOrQueueMemoryObject(WebCLBuffer*, ExceptionState&);

    Member<WebCL> m_context;
    Member<WebCLContext> m_ClContext;
    Member<WebCLProgram> m_program;
    Member<WebCLKernel> m_kernelChar;
    Member<WebCLKernel> m_kernelChar16;

    HeapVector<Member<WebCLBuffer>> m_pendingBuffers;
    HeapVector<Member<WebCLCommandQueue>> m_queues;
};

} // namespace blink

#endif // WebCLMemoryUtil_h
