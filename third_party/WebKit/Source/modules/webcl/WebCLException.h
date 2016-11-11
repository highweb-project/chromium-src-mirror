// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef WebCLException_h
#define WebCLException_h

#include "wtf/build_config.h"
#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "core/dom/ExceptionCode.h"
#include "wtf/text/WTFString.h"

namespace blink {

class WebCLException final : public GarbageCollectedFinalized<WebCLException>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static const int WebCLExceptionOffset = WebCLError;
	static const int WebCLExceptionMax = 54;

	enum WebCLExceptionCode {
		SUCCESS 								 = WebCLExceptionOffset,
		DEVICE_NOT_FOUND                         =  WebCLExceptionOffset + 1,
		DEVICE_NOT_AVAILABLE                     =  WebCLExceptionOffset + 2,
		COMPILER_NOT_AVAILABLE                   =  WebCLExceptionOffset + 3,
		MEM_OBJECT_ALLOCATION_FAILURE            =  WebCLExceptionOffset + 4,
		OUT_OF_RESOURCES                         =  WebCLExceptionOffset + 5,
		OUT_OF_HOST_MEMORY                       =  WebCLExceptionOffset + 6,
		PROFILING_INFO_NOT_AVAILABLE             =  WebCLExceptionOffset + 7,
		MEM_COPY_OVERLAP                         =  WebCLExceptionOffset + 8,
		IMAGE_FORMAT_MISMATCH                    =  WebCLExceptionOffset + 9,
		IMAGE_FORMAT_NOT_SUPPORTED               =  WebCLExceptionOffset + 10,
		BUILD_PROGRAM_FAILURE                    =  WebCLExceptionOffset + 11,
		MAP_FAILURE                              =  WebCLExceptionOffset + 12,
		MISALIGNED_SUB_BUFFER_OFFSET             =  WebCLExceptionOffset + 13,
		EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST =  WebCLExceptionOffset + 14,

		INVALID_VALUE                            =  WebCLExceptionOffset + 15,
		INVALID_DEVICE_TYPE                      =  WebCLExceptionOffset + 16,
		INVALID_PLATFORM                         =  WebCLExceptionOffset + 17,
		INVALID_DEVICE                           =  WebCLExceptionOffset + 18,
		INVALID_CONTEXT                          =  WebCLExceptionOffset + 19,
		INVALID_QUEUE_PROPERTIES                 =  WebCLExceptionOffset + 20,
		INVALID_COMMAND_QUEUE                    =  WebCLExceptionOffset + 21,
		INVALID_HOST_PTR                         =  WebCLExceptionOffset + 22,
		INVALID_MEM_OBJECT                       =  WebCLExceptionOffset + 23,
		INVALID_IMAGE_FORMAT_DESCRIPTOR          =  WebCLExceptionOffset + 24,
		INVALID_IMAGE_SIZE                       =  WebCLExceptionOffset + 25,
		INVALID_SAMPLER                          =  WebCLExceptionOffset + 26,
		INVALID_BINARY                           =  WebCLExceptionOffset + 27,
		INVALID_BUILD_OPTIONS                    =  WebCLExceptionOffset + 28,
		INVALID_PROGRAM                          =  WebCLExceptionOffset + 29,
		INVALID_PROGRAM_EXECUTABLE               =  WebCLExceptionOffset + 30,
		INVALID_KERNEL_NAME                      =  WebCLExceptionOffset + 31,
		INVALID_KERNEL_DEFINITION                =  WebCLExceptionOffset + 32,
		INVALID_KERNEL                           =  WebCLExceptionOffset + 33,
		INVALID_ARG_INDEX                        =  WebCLExceptionOffset + 34,
		INVALID_ARG_VALUE                        =  WebCLExceptionOffset + 35,
		INVALID_ARG_SIZE                         =  WebCLExceptionOffset + 36,
		INVALID_KERNEL_ARGS                      =  WebCLExceptionOffset + 37,
		INVALID_WORK_DIMENSION                   =  WebCLExceptionOffset + 38,
		INVALID_WORK_GROUP_SIZE                  =  WebCLExceptionOffset + 39,
		INVALID_WORK_ITEM_SIZE                   =  WebCLExceptionOffset + 40,
		INVALID_GLOBAL_OFFSET                    =  WebCLExceptionOffset + 41,
		INVALID_EVENT_WAIT_LIST                  =  WebCLExceptionOffset + 42,
		INVALID_EVENT                            =  WebCLExceptionOffset + 43,
		INVALID_OPERATION                        =  WebCLExceptionOffset + 44,
		INVALID_GL_OBJECT                        =  WebCLExceptionOffset + 45,
		INVALID_BUFFER_SIZE                      =  WebCLExceptionOffset + 46,
		INVALID_MIP_LEVEL                        =  WebCLExceptionOffset + 47,
		INVALID_GLOBAL_WORK_SIZE                 =  WebCLExceptionOffset + 48,
		INVALID_PROPERTY                         =  WebCLExceptionOffset + 49,
		FAILURE 								 =  WebCLExceptionOffset + 50,
		WEBCL_EXTENSION_NOT_ENABLED				 =  WebCLExceptionOffset + 51,
		WEBCL_API_NOT_PERMITTED					 =  WebCLExceptionOffset + 52,
		INVALID_GL_SHAREGROUP_REFERENCE_KHR		 =  WebCLExceptionOffset + 53
	};

	static WebCLException* create(ExceptionCode, const String& sanitizedMessage = String(), const String& unsanitizedMessage = String());

	unsigned short code() const { return m_code; }
	String name() const { return m_name; }
	String message() const { return m_sanitizedMessage; }

	static bool isWebCLException(ExceptionCode ec) { return (ec>=DEVICE_NOT_FOUND && ec <= INVALID_GL_SHAREGROUP_REFERENCE_KHR)?true:false; }

	static String getErrorName(int);
	static String getErrorMessage(int);

	static void throwException(int& code, ExceptionState& es);

	DEFINE_INLINE_TRACE() { }

private:
	WebCLException(unsigned short code, const String& name, const String& sanitizedMessage, const String& unsanitizedMessage);

	unsigned short m_code;
	String m_name;
	String m_sanitizedMessage;
	String m_unsanitizedMessage;
};

} // namespace blink

#endif // WebCLException_h
