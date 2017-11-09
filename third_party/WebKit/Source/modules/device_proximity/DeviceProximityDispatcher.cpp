/*
 * DeviceProximityDispatcher.cpp
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */

#include "platform/wtf/build_config.h"
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
    PlatformEventDispatcher::Trace(visitor);
}

void DeviceProximityDispatcher::StartListening()
{
#if defined(OS_ANDROID)
    Platform::Current()->StartListening(kWebPlatformEventTypeDeviceProximity, this);
#endif
}

void DeviceProximityDispatcher::StopListening()
{
#if defined(OS_ANDROID)
    Platform::Current()->StopListening(kWebPlatformEventTypeDeviceProximity);
#endif
    m_lastDeviceProximityData = -1;
}

void DeviceProximityDispatcher::didChangeDeviceProximity(double value)
{
	m_lastDeviceProximityData = value;
    NotifyControllers();
}

double DeviceProximityDispatcher::latestDeviceProximityData() const
{
    return m_lastDeviceProximityData;
}

} // namespace blink
