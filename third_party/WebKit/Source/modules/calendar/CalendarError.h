// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarError_h
#define CalendarError_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"

namespace blink {

class CalendarError final : public GarbageCollectedFinalized<CalendarError>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarError* create() {return new CalendarError();};
	~CalendarError() {};

	static const unsigned short kUnknownError = 0;
  static const unsigned short kInvalidArgumentError = 1;
  static const unsigned short kTimeoutError = 2;
  static const unsigned short kPendingOperationError = 3;
  static const unsigned short kIoError = 4;
  static const unsigned short kNotSupportedError = 5;
  static const unsigned short kPermissionDeniedError = 20;
  static const unsigned short kSuccess = 99;
	static const unsigned short kNotSupportApi = 9999;

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
