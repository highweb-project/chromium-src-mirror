/*
 * DeviceProximityEvent.h
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */

#ifndef DeviceProximityEvent_h
#define DeviceProximityEvent_h

#include "modules/EventModules.h"
#include "platform/heap/Handle.h"

namespace blink {

class DeviceProximityEvent final : public Event {
	DEFINE_WRAPPERTYPEINFO();
public:
	~DeviceProximityEvent() override;

	static DeviceProximityEvent* Create() {
		return new DeviceProximityEvent();
	}
	static DeviceProximityEvent* Create(const AtomicString& eventType, double value)
	{
		return new DeviceProximityEvent(eventType, value);
	}

	double value() { return m_value; }

	const AtomicString& InterfaceName() const override;

private:
	DeviceProximityEvent();
	DeviceProximityEvent(const AtomicString& eventType, double value);

	double m_value;
};

DEFINE_TYPE_CASTS(DeviceProximityEvent, Event, event, event->InterfaceName() == EventNames::DeviceProximityEvent, event.InterfaceName() == EventNames::DeviceProximityEvent);

}

#endif /* DeviceProximityEvent_h */
