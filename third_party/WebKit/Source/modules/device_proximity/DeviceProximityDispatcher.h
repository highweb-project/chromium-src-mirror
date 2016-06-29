/*
 * DeviceProximityDispatcher.h
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */

#ifndef DeviceProximityDispatcher_h
#define DeviceProximityDispatcher_h

#include "core/frame/PlatformEventDispatcher.h"
#include "platform/heap/Handle.h"
#include "public/platform/WebDeviceProximityListener.h"

namespace blink {

class DeviceProximityDispatcher final : public GarbageCollectedFinalized<DeviceProximityDispatcher>, public PlatformEventDispatcher, public WebDeviceProximityListener {
    USING_GARBAGE_COLLECTED_MIXIN(DeviceProximityDispatcher);
public:
    static DeviceProximityDispatcher& instance();
    ~DeviceProximityDispatcher() override;

    double latestDeviceProximityData() const;

    // Inherited from WebDeviceProximityListener.
    void didChangeDeviceProximity(double) override;

    DECLARE_VIRTUAL_TRACE();

private:
    DeviceProximityDispatcher();

    // Inherited from PlatformEventDispatcher.
    void startListening() override;
    void stopListening() override;

    double m_lastDeviceProximityData;
};

} // namespace blink

#endif // DeviceProximityDispatcher_h
