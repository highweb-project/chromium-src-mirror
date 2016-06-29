// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLOperationHandler_h
#define WebCLOperationHandler_h

#include <wtf/RefCounted.h>

#include "WebCL.h"
#include "WebCLInclude.h"

namespace blink {

class WebCLOperationHandler {
public:
	WebCLOperationHandler();
	~WebCLOperationHandler();

	void startHandling();
	bool canShareOperation();

	void setOperationParameter(WebCL_Operation_Base* paramPtr);
	void setOperationData(void* dataPtr, size_t sizeInBytes);
	void setOperationEvents(cl_point* events, size_t numEvents);

	void sendOperationSignal(int operation);

	void getOperationResult(WebCL_Result_Base* resultPtr);
	void getOperationResultData(void* resultDataPtr, size_t sizeInBytes);

	bool finishHandling();
private:
	bool mIsShared;

	OwnPtr<base::SharedMemory> mSharedData;
	OwnPtr<base::SharedMemory> mSharedOperation;
	OwnPtr<base::SharedMemory> mSharedResult;
	OwnPtr<base::SharedMemory> mSharedEvents;

	char* mSharedDataPtr;
	BaseOperationData* mSharedOperationPtr;
	BaseResultData* mSharedResultPtr;
	cl_point* mSharedEventsPtr;
};

} // namespace blink

#endif // WebCLEvent_h

