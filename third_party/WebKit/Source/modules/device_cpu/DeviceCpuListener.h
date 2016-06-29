/*
 * DeviceProximityListener.h
 *
 *  Created on: 2016. 01. 04.
 *      Author: Jeseon Park.
 */



#ifndef DeviceCpuListener_h
#define DeviceCpuListener_h


#include "modules/EventModules.h"
#include "platform/heap/Handle.h"
#include "wtf/text/WTFString.h"
#include "public/platform/modules/device_systeminformation/WebDeviceCpuListener.h"

namespace blink {

class DeviceCpu;
class DeviceCpuStatus;

class DeviceCpuListener final : public GarbageCollectedFinalized<DeviceCpuListener>, public WebDeviceCpuListener{
public:
    static DeviceCpuListener* instance(DeviceCpu*);
    ~DeviceCpuListener();

    void resultDeviceCpu(const WebDeviceCpuStatus&) override;

    DeviceCpuStatus* lastData();

    DECLARE_VIRTUAL_TRACE();

private:
    static DeviceCpuListener* mListener;
    DeviceCpuListener(DeviceCpu*);
    Member<DeviceCpu> mDeviceCpu;
    Member<DeviceCpuStatus> mCpuStatus;
};

} // namespace blink

#endif // DeviceCpuListener_h
