// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef WebVKCException_h
#define WebVKCException_h

#include "wtf/build_config.h"
#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "core/dom/ExceptionCode.h"
#include "core/dom/DOMException.h"
#include "wtf/text/WTFString.h"

#include "WebVKCInclude.h"


namespace blink {

class WebVKCException final : public GarbageCollectedFinalized<WebVKCException>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static const int WebVKCExceptionOffset = WebVulkanError;
	static const int WebVKCExceptionMax = 26;

	enum WebVKCExceptionCode {
		VK_NOT_READY_ECODE = WebVKCExceptionOffset,
		VK_TIMEOUT_ECODE,
		VK_ERROR_OUT_OF_HOST_MEMORY_ECODE,
		VK_ERROR_OUT_OF_DEVICE_MEMORY_ECODE,
		VK_ERROR_INITIALIZATION_FAILED_ECODE,
		VK_ERROR_DEVICE_LOST_ECODE,
		VK_ERROR_MEMORY_MAP_FAILED_ECODE,
		VK_ERROR_LAYER_NOT_PRESENT_ECODE,
		VK_ERROR_EXTENSION_NOT_PRESENT_ECODE,
		VK_ERROR_FEATURE_NOT_PRESENT_ECODE,
		VK_ERROR_INCOMPATIBLE_DRIVER_ECODE,
		VK_ERROR_TOO_MANY_OBJECTS_ECODE,
		VK_ERROR_FORMAT_NOT_SUPPORTED_ECODE,
		VK_ERROR_SURFACE_LOST_KHR_ECODE,
		VK_ERROR_NATIVE_WINDOW_IN_USE_KHR_ECODE,
		VK_SUBOPTIMAL_KHR_ECODE,
		VK_ERROR_OUT_OF_DATE_KHR_ECODE,
		VK_ERROR_INCOMPATIBLE_DISPLAY_KHR_ECODE,
		VK_ERROR_VALIDATION_FAILED_EXT_ECODE,
		VKC_SEND_IPC_MESSAGE_FAILURE_ECODE,
		VKC_FAILURE_ECODE,
		VKC_ARGUMENT_NOT_VALID_ECODE,
		VKC_GET_SHADER_CODE_FAIL_ECODE,
		VKC_ALREADY_BEGIN_COMMAND_QUEUE_ECODE,
		VKC_COMMAND_QUEUE_NOT_BEGINING_ECODE,
		VKC_ERROR_NOT_SETARG_BUFFER_INDEX_ECODE,
	};

	static WebVKCException* create(ExceptionCode, const String& sanitizedMessage = String(), const String& unsanitizedMessage = String());

	unsigned short code() const { return m_code; }
	String name() const { return m_name; }
	String message() const { return m_sanitizedMessage; }

	static bool isWebVKCException(ExceptionCode ec) { 
		return ec>=VK_NOT_READY_ECODE && ec <= WebVKCExceptionOffset + WebVKCExceptionMax?true:false; }

	static String getErrorName(int);
	static String getErrorMessage(int);

	static void throwVKCException(int result, ExceptionState& es);

	DECLARE_VIRTUAL_TRACE();

private:
	WebVKCException(unsigned short code, const String& name, const String& sanitizedMessage, const String& unsanitizedMessage);

	static void throwException(int code, ExceptionState& es);

	unsigned short m_code;
	String m_name;
	String m_sanitizedMessage;
	String m_unsanitizedMessage;
};

} // namespace blink

#endif // WebCLException_h
