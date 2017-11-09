// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLKernelArgInfoProvider_h
#define WebCLKernelArgInfoProvider_h


#include "platform/wtf/build_config.h"
#include "WebCLInclude.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/WTFString.h"

#include "platform/heap/Handle.h"

namespace blink {

class WebCLKernel;
class WebCLKernelArgInfo;
class WebCLKernelArgInfoProvider : public GarbageCollectedFinalized<WebCLKernelArgInfoProvider> {
public:
	explicit WebCLKernelArgInfoProvider(WebCLKernel*);
	const HeapVector<WebCLKernelArgInfo>* argumentsInfo() { return &mArgumentInfoVector; };

	unsigned numberOfArguments() { return mArgumentInfoVector.size(); }
	const Vector<unsigned>& requiredArguments() { return mRequiredArgumentVector; }

	void clear();

	DECLARE_TRACE();

private:
	void ensureInfo();
	void parseAndAppendDeclaration(const String& argumentDeclaration);
	String extractAddressQualifier(Vector<String>& declaration);
	String extractAccessQualifier(Vector<String>& declaration);
	String extractType(Vector<String>& declaration);
	String extractName(Vector<String>& declaration);

	Member<WebCLKernel> mKernel;
	HeapVector<WebCLKernelArgInfo> mArgumentInfoVector;
	Vector<unsigned> mRequiredArgumentVector;
};

} // namespace blink

#endif // WebCLKernelArgInfoProvider_h
