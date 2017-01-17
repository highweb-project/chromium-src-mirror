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

	static DeviceProximityEvent* create() {
		return new DeviceProximityEvent();
	}
	static DeviceProximityEvent* create(const AtomicString& eventType, double value)
	{
		return new DeviceProximityEvent(eventType, value);
	}

	double value() { return m_value; }

	const AtomicString& interfaceName() const override;

private:
	DeviceProximityEvent();
	DeviceProximityEvent(const AtomicString& eventType, double value);

	double m_value;
};

DEFINE_TYPE_CASTS(DeviceProximityEvent, Event, event, event->interfaceName() == EventNames::DeviceProximityEvent, event.interfaceName() == EventNames::DeviceProximityEvent);

}

#endif /* DeviceProximityEvent_h */
