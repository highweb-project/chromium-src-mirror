/*
 * DeviceProximityEvent.cpp
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
#include "modules/device_proximity/DeviceProximityEvent.h"

namespace blink {

DeviceProximityEvent::~DeviceProximityEvent()
{
}

DeviceProximityEvent::DeviceProximityEvent()
{
	m_value = 0;
}

DeviceProximityEvent::DeviceProximityEvent(const AtomicString& eventType, double value)
    : Event(eventType, true, false) // The DeviceLightEvent bubbles but is not cancelable.
    , m_value(value)
{
}

const AtomicString& DeviceProximityEvent::InterfaceName() const
{
	return EventNames::DeviceProximityEvent;
}

}
