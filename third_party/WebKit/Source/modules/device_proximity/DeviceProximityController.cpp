/*
 * DeviceProximityController.cpp
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */

#include "wtf/build_config.h"
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
#if !ENABLE(OILPAN)
    stopUpdating();
#endif
}

const char* DeviceProximityController::supplementName()
{
    return "DeviceProximityController";
}

DeviceProximityController& DeviceProximityController::from(Document& document)
{
	DeviceProximityController* controller = static_cast<DeviceProximityController*>(Supplement<Document>::from(document, supplementName()));
    if (!controller) {
        controller = new DeviceProximityController(document);
        Supplement<Document>::provideTo(document, supplementName(), controller);
    }
    return *controller;
}

bool DeviceProximityController::hasLastData()
{
    return 0;
}

void DeviceProximityController::registerWithDispatcher()
{
    DeviceProximityDispatcher::instance().addController(this);
}

void DeviceProximityController::unregisterWithDispatcher()
{
    DeviceProximityDispatcher::instance().removeController(this);
}

Event* DeviceProximityController::lastEvent() const
{
    return DeviceProximityEvent::create(EventTypeNames::deviceproximity, DeviceProximityDispatcher::instance().latestDeviceProximityData());
}

bool DeviceProximityController::isNullEvent(Event* event) const
{
    //DeviceProximityEvent* proximityEvent = toDeviceProximityEvent(event);
    return 0;
}

const AtomicString& DeviceProximityController::eventTypeName() const
{
    return EventTypeNames::deviceproximity;
}

DEFINE_TRACE(DeviceProximityController)
{
    DeviceSingleWindowEventController::trace(visitor);
    Supplement<Document>::trace(visitor);
}


} // namespace blink


