#include "platform/wtf/build_config.h"
#include "modules/applauncher/DeviceApiApplauncherController.h"
#include "core/dom/Document.h"

namespace blink {
DeviceApiApplauncherController::~DeviceApiApplauncherController()
{
}

void DeviceApiApplauncherController::ProvideTo(LocalFrame& frame, WebDeviceApiApplauncherClient* client)
{
	DeviceApiApplauncherController* controller = new DeviceApiApplauncherController(frame, client);
    Supplement<LocalFrame>::ProvideTo(frame, SupplementName(), controller);
}

DeviceApiApplauncherController* DeviceApiApplauncherController::From(LocalFrame& frame)
{
    return static_cast<DeviceApiApplauncherController*>(Supplement<LocalFrame>::From(frame, SupplementName()));
}

DeviceApiApplauncherController::DeviceApiApplauncherController(LocalFrame& frame, WebDeviceApiApplauncherClient* client)
    : m_client(client)
{
}

const char* DeviceApiApplauncherController::SupplementName()
{
    return "DeviceApiApplauncherController";
}

DEFINE_TRACE(DeviceApiApplauncherController)
{
    Supplement<LocalFrame>::Trace(visitor);
}

}



