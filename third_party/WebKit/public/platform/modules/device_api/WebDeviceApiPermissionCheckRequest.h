/*
 * WebDeviceApiPermissionCheckRequest.h
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#ifndef WebDeviceApiPermissionCheckRequest_h
#define WebDeviceApiPermissionCheckRequest_h

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

namespace blink {

struct WebDeviceApiPermissionCheckRequest {
	WebDeviceApiPermissionCheckClient::ApiType api;
	WebDeviceApiPermissionCheckClient::OperationType operation;
	DeviceApiPermissionCallback callback;
	std::string request_id;

	WebDeviceApiPermissionCheckRequest(
			WebDeviceApiPermissionCheckClient::ApiType api,
			WebDeviceApiPermissionCheckClient::OperationType operation,
			const DeviceApiPermissionCallback& callback)
		: api(api),
		  operation(operation),
		  callback(callback)
	{
	}
};

} // namespace blink

#endif // WebDeviceApiPermissionCheckRequest_h
