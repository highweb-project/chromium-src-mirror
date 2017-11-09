// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "WebCLAPIBlocker.h"

namespace blink {

WebCLAPIBlocker::WebCLAPIBlocker(WTF::String model)
{
#if defined(OS_ANROID)
	CLLOG(INFO) << "Current device model=" << model.Latin1().data();
	mModelName = model;

	WTF::String key1("NEXUS 7_createCommandQueue_WebCLDevice");
	mBlacklistMap.set(key1, true);
#endif	
}

WebCLAPIBlocker::~WebCLAPIBlocker()
{

}

bool WebCLAPIBlocker::isAPIBlocked(WTF::String api)
{
	WTF::String key(mModelName+"_"+api);

	if(!mBlacklistMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key.Latin1().data();
		return false;
	}
	else
		CLLOG(INFO) << ">> find objct, key=" << key.Latin1().data();

	return true;
}

}


