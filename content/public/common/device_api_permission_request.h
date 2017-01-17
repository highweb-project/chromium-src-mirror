/*
 * device_api_permission_request.h
 *
 *  Created on: 2015. 12. 2.
 *      Author: azureskybox
 */

#ifndef CONTENT_PUBLIC_COMMON_DEVICE_API_PERMISSION_REQUEST_H_
#define CONTENT_PUBLIC_COMMON_DEVICE_API_PERMISSION_REQUEST_H_

// #include "base/basictypes.h"
#include "base/callback.h"
#include "content/common/content_export.h"
#include <utility>

namespace content {

enum ApiType {
	CALENDAR = 1,
	MESSAGING = 2,
	CONTACT = 3,
	GALLERY = 4,
	APPLICATION_LAUNCHER = 5,
	SYSTEM_INFORMATION = 6
};

enum OperationType {
	NONE = 0,
	VIEW = 1,
	MODIFY = 2
};

enum DeviceApiPermissionRequestResult {
	RESULT_OK = 0,
	RESULT_DENIED = 1,
	RESULT_NOT_IMPLEMENTED = 2,
	RESULT_NOT_SUPPORTED = 3
};

typedef base::Callback<void(content::DeviceApiPermissionRequestResult result)> DeviceApiPermissionCallback;

struct CONTENT_EXPORT DeviceApiPermissionRequest {
	DeviceApiPermissionRequest(const DeviceApiPermissionRequest& other);
	DeviceApiPermissionRequest(const DeviceApiPermissionCallback& callback);
	~DeviceApiPermissionRequest();
	int api_type_;
	int operation_type_;
	DeviceApiPermissionCallback callback_;
};

}



#endif  // CONTENT_PUBLIC_COMMON_DEVICE_API_PERMISSION_REQUEST_H_ */
