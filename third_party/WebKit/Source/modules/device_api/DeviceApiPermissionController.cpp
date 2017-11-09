/*
 * DeviceApiPermissionController.cpp
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_api/DeviceApiPermissionController.h"
#include "core/dom/Document.h"

namespace blink {
DeviceApiPermissionController::~DeviceApiPermissionController()
{
}

void DeviceApiPermissionController::ProvideTo(LocalFrame& frame, WebDeviceApiPermissionCheckClient* client)
{
	DeviceApiPermissionController* controller = new DeviceApiPermissionController(frame, client);
    Supplement<LocalFrame>::ProvideTo(frame, SupplementName(), controller);
}

DeviceApiPermissionController* DeviceApiPermissionController::From(LocalFrame& frame)
{
    return static_cast<DeviceApiPermissionController*>(Supplement<LocalFrame>::From(frame, SupplementName()));
}

DeviceApiPermissionController::DeviceApiPermissionController(LocalFrame& frame, WebDeviceApiPermissionCheckClient* client)
    : m_client(client)
{
}

const char* DeviceApiPermissionController::SupplementName()
{
    return "DeviceApiPermissionController";
}

DEFINE_TRACE(DeviceApiPermissionController)
{
    Supplement<LocalFrame>::Trace(visitor);
}

}



