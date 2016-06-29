/*
 * DeviceProximityDispatcher.cpp
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */

#include "wtf/build_config.h"
#include "modules/device_proximity/DeviceProximityDispatcher.h"

#include "modules/device_proximity/DeviceProximityController.h"
#include "public/platform/Platform.h"

namespace blink {

DeviceProximityDispatcher& DeviceProximityDispatcher::instance()
{
    DEFINE_STATIC_LOCAL(Persistent<DeviceProximityDispatcher>, deviceProximityDispatcher, (new DeviceProximityDispatcher()));
    return *deviceProximityDispatcher;
}

DeviceProximityDispatcher::DeviceProximityDispatcher()
    : m_lastDeviceProximityData(-1)
{
}

DeviceProximityDispatcher::~DeviceProximityDispatcher()
{
}

DEFINE_TRACE(DeviceProximityDispatcher)
{
    PlatformEventDispatcher::trace(visitor);
}

void DeviceProximityDispatcher::startListening()
{
    Platform::current()->startListening(WebPlatformEventTypeDeviceProximity, this);
}

void DeviceProximityDispatcher::stopListening()
{
    Platform::current()->stopListening(WebPlatformEventTypeDeviceProximity);
    m_lastDeviceProximityData = -1;
}

void DeviceProximityDispatcher::didChangeDeviceProximity(double value)
{
	m_lastDeviceProximityData = value;
    notifyControllers();
}

double DeviceProximityDispatcher::latestDeviceProximityData() const
{
    return m_lastDeviceProximityData;
}

} // namespace blink


