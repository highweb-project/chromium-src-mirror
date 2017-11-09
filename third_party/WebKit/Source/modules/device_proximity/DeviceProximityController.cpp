/*
 * DeviceProximityController.cpp
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */

#include "platform/wtf/build_config.h"
#include "modules/device_proximity/DeviceProximityController.h"

#include "core/dom/Document.h"
#include "modules/EventModules.h"
#include "platform/RuntimeEnabledFeatures.h"

#include "modules/device_proximity/DeviceProximityEvent.h"
#include "modules/device_proximity/DeviceProximityDispatcher.h"

namespace blink {

DeviceProximityController::DeviceProximityController(Document& document)
    : DeviceSingleWindowEventController(document)
{
}

DeviceProximityController::~DeviceProximityController()
{
    StopUpdating();
}

const char* DeviceProximityController::SupplementName()
{
    return "DeviceProximityController";
}

DeviceProximityController& DeviceProximityController::From(Document& document)
{
	DeviceProximityController* controller = static_cast<DeviceProximityController*>(Supplement<Document>::From(document, SupplementName()));
    if (!controller) {
        controller = new DeviceProximityController(document);
        Supplement<Document>::ProvideTo(document, SupplementName(), controller);
    }
    return *controller;
}

bool DeviceProximityController::HasLastData()
{
    return 0;
}

void DeviceProximityController::RegisterWithDispatcher()
{
    DeviceProximityDispatcher::instance().AddController(this);
}

void DeviceProximityController::UnregisterWithDispatcher()
{
    DeviceProximityDispatcher::instance().RemoveController(this);
}

Event* DeviceProximityController::LastEvent() const
{
    return DeviceProximityEvent::Create(EventTypeNames::deviceproximity, DeviceProximityDispatcher::instance().latestDeviceProximityData());
}

bool DeviceProximityController::IsNullEvent(Event* event) const
{
    //DeviceProximityEvent* proximityEvent = toDeviceProximityEvent(event);
    return 0;
}

const AtomicString& DeviceProximityController::EventTypeName() const
{
    return EventTypeNames::deviceproximity;
}

DEFINE_TRACE(DeviceProximityController)
{
    DeviceSingleWindowEventController::Trace(visitor);
    Supplement<Document>::Trace(visitor);
}


} // namespace blink


