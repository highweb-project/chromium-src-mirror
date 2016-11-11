// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "wtf/build_config.h"
#include "WebVKCOperationHandler.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/memory/shared_memory.h"
#include "base/process/process_handle.h"
#include "public/platform/Platform.h"

namespace blink {
namespace {
static int kMaxBufferSize = 1024 * 1024 * 20;
}

WebVKCOperationHandler::WebVKCOperationHandler()
	: mIsShared(false)
{
}

WebVKCOperationHandler::~WebVKCOperationHandler()
{
	mIsShared = false;
}

void WebVKCOperationHandler::startHandling()
{
	VKCLOG(INFO) << "SharedMemory startHandling";
	if(!mIsShared) {
		base::SharedMemoryHandle dataHandle;
		base::SharedMemoryHandle operationHandle;
		base::SharedMemoryHandle resultHandle;

#if defined(OS_ANDROID)
		mSharedData = adoptPtr(new base::SharedMemory());
		mSharedData->CreateAndMapAnonymous(kMaxBufferSize);
#elif defined(OS_LINUX)
		// mSharedData = adoptPtr(content::RenderThread::Get()->HostAllocateSharedMemoryBuffer(kMaxBufferSize).release());
		mSharedData = adoptPtr(Platform::current()->getSharedMemoryForWebVKC(kMaxBufferSize));
		mSharedData->Map(kMaxBufferSize);
#endif
		mSharedData->ShareToProcess(base::GetCurrentProcessHandle(), &dataHandle);
		mSharedDataPtr = static_cast<char*>(mSharedData->memory());

#if defined(OS_ANDROID)
		mSharedOperation = adoptPtr(new base::SharedMemory());
		mSharedOperation->CreateAndMapAnonymous(sizeof(BaseVKCOperationData));
#elif defined(OS_LINUX)
		// mSharedOperation = adoptPtr(content::RenderThread::Get()->HostAllocateSharedMemoryBuffer(sizeof(BaseOperationData)).release());
		 mSharedOperation = adoptPtr(Platform::current()->getSharedMemoryForWebVKC(sizeof(BaseVKCOperationData)));
		mSharedOperation->Map(sizeof(BaseVKCOperationData));
#endif
		mSharedOperation->ShareToProcess(base::GetCurrentProcessHandle(), &operationHandle);
		mSharedOperationPtr = static_cast<BaseVKCOperationData*>(mSharedOperation->memory());

#if defined(OS_ANDROID)
		mSharedResult = adoptPtr(new base::SharedMemory());
		mSharedResult->CreateAndMapAnonymous(sizeof(BaseVKCResultData));
#elif defined(OS_LINUX)
		// mSharedResult = adoptPtr(content::RenderThread::Get()->HostAllocateSharedMemoryBuffer(sizeof(BaseResultData)).release());
		mSharedResult = adoptPtr(Platform::current()->getSharedMemoryForWebVKC(sizeof(BaseVKCResultData)));
		mSharedResult->Map(sizeof(BaseVKCResultData));
#endif
		mSharedResult->ShareToProcess(base::GetCurrentProcessHandle(), &resultHandle);
		mSharedResultPtr = static_cast<BaseVKCResultData*>(mSharedResult->memory());

		mIsShared = webvkc_SetSharedHandles(webvkc_channel_, dataHandle, operationHandle, resultHandle);
		VKCLOG(INFO) << "isShared : " << mIsShared;
	}
}

bool WebVKCOperationHandler::canShareOperation()
{
	return mIsShared;
}

void WebVKCOperationHandler::setOperationParameter(WebVKC_Operation_Base* paramPtr)
{
	DCHECK(paramPtr);

	switch(paramPtr->operation_type) {
		case VULKAN_OPERATION_TYPE::VKC_WRITE_BUFFER:
		case VULKAN_OPERATION_TYPE::VKC_READ_BUFFER: {
			WebVKC_RW_Buffer_Operation* tmp = static_cast<WebVKC_RW_Buffer_Operation*>(paramPtr);
			mSharedOperationPtr->uint_01 = tmp->index;
			mSharedOperationPtr->uint_02 = tmp->bufferByteSize;
			mSharedOperationPtr->point_01 = tmp->vkcDevice;
			mSharedOperationPtr->point_02 = tmp->vkcMemory;
		}
		break;
	}
}

void WebVKCOperationHandler::setOperationData(void* dataPtr, size_t sizeInBytes)
{
	DCHECK(dataPtr);
	DCHECK(sizeInBytes<(size_t)kMaxBufferSize);

	memcpy(mSharedDataPtr, dataPtr, sizeInBytes);
}

void WebVKCOperationHandler::getOperationResult(WebVKC_Result_Base* resultPtr)
{
	switch(resultPtr->operation_type) {
		case VULKAN_OPERATION_TYPE::VKC_WRITE_BUFFER:
		case VULKAN_OPERATION_TYPE::VKC_READ_BUFFER: {
			resultPtr->result = mSharedResultPtr->result_01;
		}
		break;
	}
}

void WebVKCOperationHandler::sendOperationSignal(int operation)
{
	DCHECK(mIsShared);

	webvkc_TriggerSharedOperation(webvkc_channel_, operation);
}

void WebVKCOperationHandler::getOperationResultData(void* resultDataPtr, size_t sizeInBytes)
{
	memcpy(resultDataPtr, mSharedDataPtr, sizeInBytes);
}

bool WebVKCOperationHandler::finishHandling()
{
	bool result = false;
	if(mIsShared) {
		result = webvkc_ClearSharedHandles(webvkc_channel_);

		mSharedData->Unmap();
		mSharedOperation->Unmap();
		mSharedResult->Unmap();

		mSharedData->Close();
		mSharedOperation->Close();
		mSharedResult->Close();

		mSharedData.reset();
		mSharedOperation.reset();
		mSharedResult.reset();

		mIsShared = false;
	}
	return result;
}

}
