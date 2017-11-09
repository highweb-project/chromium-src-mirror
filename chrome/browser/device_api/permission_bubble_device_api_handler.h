/*
 * permission_bubble_device_api_handler.h
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */

#ifndef CHROME_BROWSER_DEVICE_API_PERMISSION_BUBBLE_DEVICE_API_HANDLER_H_
#define CHROME_BROWSER_DEVICE_API_PERMISSION_BUBBLE_DEVICE_API_HANDLER_H_

#include "base/memory/singleton.h"

#include "content/public/common/device_api_permission_request.h"

namespace content {
class WebContents;
}

class PermissionBubbleDeviceApiHandler {
public:
	static PermissionBubbleDeviceApiHandler* GetInstance();

	void CheckPermission(content::WebContents* web_contents, const content::DeviceApiPermissionRequest& request);

	void OnPermissionResult(content::DeviceApiPermissionRequestResult result);

private:
	friend struct base::DefaultSingletonTraits<PermissionBubbleDeviceApiHandler>;

	PermissionBubbleDeviceApiHandler();
	~PermissionBubbleDeviceApiHandler();
};



#endif /* CHROME_BROWSER_DEVICE_API_PERMISSION_BUBBLE_DEVICE_API_HANDLER_H_ */
