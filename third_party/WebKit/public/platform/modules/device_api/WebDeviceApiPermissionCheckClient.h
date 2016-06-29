/*
 * WebDeviceApiPermissionCheckListener.h
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#ifndef WebDeviceApiPermissionCheckClient_h
#define WebDeviceApiPermissionCheckClient_h

#include <string>

// #include "base/basictypes.h"
#include "base/callback.h"
#include "base/callback_forward.h"

namespace blink {

struct WebDeviceApiPermissionCheckRequest;

class WebDeviceApiPermissionCheckClient {
public:
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
		MODIFY = 2,
		FIND = 3,
		ADD = 4,
		UPDATE = 5,
		DELETE = 6
	};

	enum DeviceApiPermissionRequestResult {
		RESULT_OK = 0,
		RESULT_DENIED = 1,
		RESULT_NOT_IMPLEMENTED = 2,
		RESULT_NOT_SUPPORTED = 3
	};

	void SetOrigin(std::string origin) { origin_ = origin; };

	virtual void CheckPermission(WebDeviceApiPermissionCheckRequest* request) = 0;

    virtual ~WebDeviceApiPermissionCheckClient() { }
protected:
    std::string origin_;
};

typedef WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult PermissionResult;
typedef WebDeviceApiPermissionCheckClient::ApiType PermissionAPIType;
typedef WebDeviceApiPermissionCheckClient::OperationType PermissionOptType;

typedef base::Callback<void(
		PermissionResult result)> DeviceApiPermissionCallback;

} // namespace blink

#endif // WebDeviceApiPermissionCheckClient_h
