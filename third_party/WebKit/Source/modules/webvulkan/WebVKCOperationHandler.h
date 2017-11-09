// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebVKCOperationHandler_h
#define WebVKCOperationHandler_h

#include "platform/wtf/RefCounted.h"

#include "WebVKC.h"
#include "WebVKCInclude.h"

namespace blink {

class WebVKCOperationHandler {
public:
	WebVKCOperationHandler();
	~WebVKCOperationHandler();

	void startHandling();
	bool canShareOperation();

	void setOperationParameter(WebVKC_Operation_Base* paramPtr);
	void setOperationData(void* dataPtr, size_t sizeInBytes);

	void sendOperationSignal(int operation);

	void getOperationResult(WebVKC_Result_Base* resultPtr);
	void getOperationResultData(void* resultDataPtr, size_t sizeInBytes);

	bool finishHandling();
private:
	bool mIsShared;

	std::unique_ptr<base::SharedMemory> mSharedData;
	std::unique_ptr<base::SharedMemory> mSharedOperation;
	std::unique_ptr<base::SharedMemory> mSharedResult;

	char* mSharedDataPtr;
	BaseVKCOperationData* mSharedOperationPtr;
	BaseVKCResultData* mSharedResultPtr;
};

} // namespace blink

#endif
