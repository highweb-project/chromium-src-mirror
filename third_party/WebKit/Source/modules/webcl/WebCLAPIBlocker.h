// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLAPIBlocker_H
#define WebCLAPIBlocker_H

#include "platform/heap/Handle.h"
#include "wtf/HashMap.h"
#include "wtf/text/StringBuilder.h"
#include "wtf/text/StringHash.h"
#include "wtf/text/WTFString.h"

#include "WebCLInclude.h"

#define WEBCL_BLOCKED_API_KEY(model, api) \
	#model"_"#api

#define WEBCL_BLOCKED_API_ARG1(api, arg1) \
	#api"_"#arg1

namespace blink {

typedef HashMap<WTF::String, bool> WebCLBlacklistMap;

class WebCLAPIBlocker {
public:
	WebCLAPIBlocker(WTF::String model);
	~WebCLAPIBlocker();

	bool isAPIBlocked(WTF::String api);
private:
	WTF::String mModelName;

	WebCLBlacklistMap mBlacklistMap;
};

}

#endif
