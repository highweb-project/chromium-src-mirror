/*
 * device_api_permission_request.cc
 *
 *  Created on: 2015. 12. 2.
 *      Author: azureskybox
 */
//#include "content/public/common/device_api_permission_request.h"
//
//namespace content {
//
//DeviceApiPermissionRequest::DeviceApiPermissionRequest(const DeviceApiPermissionCallback& callback)
//	: callback_(callback)
//{
//}
//
//DeviceApiPermissionRequest::~DeviceApiPermissionRequest()
//{
//}
//
//}



#include "device_api_permission_request.h"

namespace content {
	DeviceApiPermissionRequest::DeviceApiPermissionRequest(const DeviceApiPermissionRequest& other) = default;
	DeviceApiPermissionRequest::DeviceApiPermissionRequest(const DeviceApiPermissionCallback& callback) {
		callback_ = callback;
	}
	DeviceApiPermissionRequest::~DeviceApiPermissionRequest() {
	}
}