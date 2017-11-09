// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "WebCL.h"
#include "WebCLBuffer.h"
#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLDevice.h"
#include "WebCLKernel.h"
#include "WebCLMemoryObject.h"
#include "WebCLMemoryUtil.h"
#include "WebCLProgram.h"

namespace blink {

const char* programSource = \
    "__kernel void init(__global char* buffer, unsigned offset, unsigned count) "\
    "{                                                                          "\
    "    unsigned i = get_global_id(0);                                         "\
    "    unsigned displacedI = i + offset;                                      "\
    "    if (displacedI < count)                                                "\
    "        buffer[displacedI] = (char)(0);                                    "\
    "}                                                                          "\
    "__kernel void init16(__global char16* buffer, unsigned count)              "\
    "{                                                                          "\
    "    unsigned i = get_global_id(0);                                         "\
    "    if (i < count)                                                         "\
    "        buffer[i] = (char16)(0);                                           "\
    "}                                                                          ";

WebCLMemoryUtil::WebCLMemoryUtil(WebCL* context, WebCLContext* clContext)
    : m_context(context),
	  m_ClContext(clContext)
{
}

WebCLMemoryUtil::~WebCLMemoryUtil()
{
  m_pendingBuffers.clear();
  m_queues.clear();
}

void WebCLMemoryUtil::ensureMemory(WebCLMemoryObject* memoryObject, WebCLCommandQueue* commandQueue, ExceptionState& es)
{
    cl_int err = CL_SUCCESS;
    // The program and kernels are used to intialize OpenCL memory to 0.
    // Every created OpenCL memory should call this function to intialize.
    // TODO(junmin-zhu): Move intialization from buffer creation to buffer operations, such as: enqueueRead/Write/Copy* and enqueueNDRangeKernel function
    // after the third_party WebCL-validator integrated.
    if (!m_program.Get()) {
    	cl_program clProgramId = webcl_clCreateProgramWithSource(webcl_channel_, m_ClContext->getClContext(), 1, (const char**)&programSource, nullptr, &err);
    	if (err != CL_SUCCESS) {
            WebCLException::throwException(err, es);
            return;
        }

		m_program = WebCLProgram::create(m_context.Get(), clProgramId);
		m_program->setSource(String(programSource));
		m_program->setCLContext(m_ClContext.Get());
		m_ClContext->addCLProgram((cl_obj_key)clProgramId, m_program);

		m_program->build(m_ClContext->getDevices(es), g_empty_string, nullptr, es);

        cl_kernel clKernelId16 = webcl_clCreateKernel(webcl_channel_, clProgramId, "init16", &err);
        if (err != CL_SUCCESS) {
            WebCLException::throwException(err, es);
            return;
        }
        m_kernelChar16 = WebCLKernel::create(m_context.Get(), clKernelId16, m_program.Get(), String("init16"));
        m_ClContext->addCLKernel((cl_obj_key)clKernelId16, m_kernelChar16.Get());

        cl_kernel clKernelId = webcl_clCreateKernel(webcl_channel_, clProgramId, "init", &err);
        if (err != CL_SUCCESS) {
            WebCLException::throwException(err, es);
            return;
        }
        m_kernelChar = WebCLKernel::create(m_context.Get(), clKernelId, m_program.Get(), String("init"));
        m_ClContext->addCLKernel((cl_obj_key)clKernelId, m_kernelChar.Get());

    }

    unsigned count = memoryObject->getSize() / 16;
    if (count) {
        m_kernelChar16->setArg(0, (WebCLBuffer*)memoryObject, es);
        if (es.HadException())
            return;

        m_kernelChar16->setArg(1, &count, es);
        if (es.HadException())
            return;

        Vector<double> globalWorkSize;
        globalWorkSize.push_back(count);
        Vector<double> globalWorkOffset;
        Vector<double> localWorkSize;

        commandQueue->enqueueNDRangeKernel(m_kernelChar16.Get(), globalWorkSize.size(), nullptr, globalWorkSize, nullptr, es);
    }

    unsigned remainingBytes = memoryObject->getSize() % 16;
    if (remainingBytes) {
        m_kernelChar->setArg(0, (WebCLBuffer*)memoryObject, es);
        if (es.HadException())
            return;

        unsigned offset = count * 16;
        m_kernelChar->setArg(1, &offset, es);
        if (es.HadException())
            return;

        unsigned totalSize = memoryObject->getSize();
        m_kernelChar->setArg(2, &totalSize, es);
        if (es.HadException())
            return;

        Vector<double> globalWorkSize;
        globalWorkSize.push_back(remainingBytes);
        Vector<double> globalWorkOffset;
        Vector<double> localWorkSize;

        commandQueue->enqueueNDRangeKernel(m_kernelChar.Get(), globalWorkSize.size(), nullptr, globalWorkSize, nullptr, es);
    }

    commandQueue->finish(es);
}

void WebCLMemoryUtil::commandQueueCreated(WebCLCommandQueue* queue, ExceptionState& es)
{
	CLLOG(INFO) << "WebCLMemoryUtil::commandQueueCreated";

    if (!queue)
        return;

    m_queues.push_back(queue);
    processPendingMemoryList(es);
}

void WebCLMemoryUtil::bufferCreated(WebCLBuffer* buffer, ExceptionState& es)
{
	CLLOG(INFO) << "WebCLMemoryUtil::bufferCreated";

    if (!buffer)
        return;

    initializeOrQueueMemoryObject(buffer, es);
}

void WebCLMemoryUtil::processPendingMemoryList(ExceptionState& es)
{
	CLLOG(INFO) << "WebCLMemoryUtil::processPendingMemoryList";

    if (!m_pendingBuffers.size())
        return;

    WebCLCommandQueue* queue = validCommandQueue();
    if (queue) {
		size_t bufferSize = m_pendingBuffers.size();
		for(size_t i=0; i<bufferSize; i++) {
			if (m_pendingBuffers[i] && !m_pendingBuffers[i]->getCLMem())
				 ensureMemory(m_pendingBuffers[i].Get(), queue, es);
		}
    }

    m_pendingBuffers.clear();
}

WebCLCommandQueue* WebCLMemoryUtil::validCommandQueue() const
{
	CLLOG(INFO) << "WebCLMemoryUtil::validCommandQueue";

    size_t queueSize = m_queues.size();
    for(size_t i=0; i<queueSize; i++) {
        if (m_queues[i] && m_queues[i]->getClComandQueue())
            return m_queues[i].Get();
    }

    return nullptr;
}

void WebCLMemoryUtil::initializeOrQueueMemoryObject(WebCLBuffer* buffer, ExceptionState& es)
{
	CLLOG(INFO) << "WebCLMemoryUtil::initializeOrQueueMemoryObject";

    WebCLCommandQueue* queue = validCommandQueue();
    if (!queue) {
        m_pendingBuffers.push_back(buffer);
        return;
    }

    ensureMemory(buffer, queue, es);
}

DEFINE_TRACE(WebCLMemoryUtil) {
	visitor->Trace(m_context);
  visitor->Trace(m_ClContext);
	visitor->Trace(m_program);
	visitor->Trace(m_kernelChar);
	visitor->Trace(m_kernelChar16);
	visitor->Trace(m_pendingBuffers);
	visitor->Trace(m_queues);
}

} // namespace blink
