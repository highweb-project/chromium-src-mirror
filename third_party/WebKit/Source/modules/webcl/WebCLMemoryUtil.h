// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLMemoryUtil_h
#define WebCLMemoryUtil_h

#include "platform/heap/Handle.h"

#include "WebCLInclude.h"
#include "WebCLException.h"

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
class WebCLMemoryUtil {
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

    Persistent<WebCL> m_context;
    Persistent<WebCLContext> m_ClContext;
    Persistent<WebCLProgram> m_program;
    Persistent<WebCLKernel> m_kernelChar;
    Persistent<WebCLKernel> m_kernelChar16;

    Vector<Persistent<WebCLBuffer>> m_pendingBuffers;
    Vector<Persistent<WebCLCommandQueue>> m_queues;
};

} // namespace blink

#endif // WebCLMemoryUtil_h
