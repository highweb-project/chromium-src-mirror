// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarError_h
#define CalendarError_h

#include "platform/heap/Handle.h"
#include "wtf/Forward.h"
#include "wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "bindings/core/v8/ScriptWrappable.h"

namespace blink {

class CalendarError final : public GarbageCollectedFinalized<CalendarError>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarError* create() {return new CalendarError();};
	~CalendarError() {};

	static const unsigned short UNKNOWN_ERROR = 0;
  static const unsigned short INVALID_ARGUMENT_ERROR = 1;
  static const unsigned short TIMEOUT_ERROR = 2;
  static const unsigned short PENDING_OPERATION_ERROR = 3;
  static const unsigned short IO_ERROR = 4;
  static const unsigned short NOT_SUPPORTED_ERROR = 5;
  static const unsigned short PERMISSION_DENIED_ERROR = 20;
  static const unsigned short SUCCESS = 99;
	static const unsigned short NOT_SUPPORT_API = 9999;

	unsigned short mCode;

	unsigned short code() {
		return mCode;
	}

	void setCode(unsigned short code) {
		mCode = code;
	}

	DECLARE_TRACE();

private:
	CalendarError() {};
};

} // namespace blink

#endif // CalendarError_h
