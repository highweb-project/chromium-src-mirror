// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "WebCLAPIBlocker.h"

namespace blink {

WebCLAPIBlocker::WebCLAPIBlocker(WTF::String model)
{
#if defined(OS_ANROID)
	CLLOG(INFO) << "Current device model=" << model.latin1().data();
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

	if(!mBlacklistMap.contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key.latin1().data();
		return false;
	}
	else
		CLLOG(INFO) << ">> find objct, key=" << key.latin1().data();

	return true;
}

}


