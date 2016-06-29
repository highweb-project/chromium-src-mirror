/*
 * DeviceApiPermissionController.cpp
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */
#include "wtf/build_config.h"
#include "modules/device_api/DeviceApiPermissionController.h"
#include "core/dom/Document.h"

namespace blink {
DeviceApiPermissionController::~DeviceApiPermissionController()
{
}

void DeviceApiPermissionController::provideTo(LocalFrame& frame, WebDeviceApiPermissionCheckClient* client)
{
	DeviceApiPermissionController* controller = new DeviceApiPermissionController(frame, client);
    Supplement<LocalFrame>::provideTo(frame, supplementName(), controller);
}

DeviceApiPermissionController* DeviceApiPermissionController::from(LocalFrame& frame)
{
    return static_cast<DeviceApiPermissionController*>(Supplement<LocalFrame>::from(frame, supplementName()));
}

DeviceApiPermissionController::DeviceApiPermissionController(LocalFrame& frame, WebDeviceApiPermissionCheckClient* client)
    : m_client(client)
{
}

const char* DeviceApiPermissionController::supplementName()
{
    return "DeviceApiPermissionController";
}

DEFINE_TRACE(DeviceApiPermissionController)
{
    Supplement<LocalFrame>::trace(visitor);
}

}



