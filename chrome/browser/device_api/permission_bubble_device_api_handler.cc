/*
 * permission_bubble_device_api_handler.cc
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */

#include "chrome/browser/device_api/permission_bubble_device_api_handler.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "chrome/browser/device_api/device_api_infobar_delegate.h"
#include "content/public/browser/web_contents.h"
#include "base/callback.h"
#include <utility>

PermissionBubbleDeviceApiHandler* PermissionBubbleDeviceApiHandler::GetInstance()
{
	return base::Singleton<PermissionBubbleDeviceApiHandler>::get();
}

void PermissionBubbleDeviceApiHandler::CheckPermission(content::WebContents* web_contents, const content::DeviceApiPermissionRequest& request)
{
	DLOG(INFO) << "PermissionBubbleDeviceApiHandler::checkPermission";

	DeviceApiInfoBarDelegate::Create(web_contents, request, base::Bind(&PermissionBubbleDeviceApiHandler::OnPermissionResult, base::Unretained(this)));
}

void PermissionBubbleDeviceApiHandler::OnPermissionResult(content::DeviceApiPermissionRequestResult result)
{
	DLOG(INFO) << "PermissionBubbleDeviceApiHandler::OnPermissionResult";
}

PermissionBubbleDeviceApiHandler::PermissionBubbleDeviceApiHandler()
{

}

PermissionBubbleDeviceApiHandler::~PermissionBubbleDeviceApiHandler()
{

}
