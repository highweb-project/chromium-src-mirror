/*
 * WebDeviceProximityListener.h
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */

#ifndef WebDeviceProximityListener_h
#define WebDeviceProximityListener_h

#include "public/platform/WebPlatformEventListener.h"

namespace blink {

class WebDeviceProximityListener : public WebPlatformEventListener {
public:
    // This method is called every time new device light data is available.
    virtual void didChangeDeviceProximity(double) = 0;

    virtual ~WebDeviceProximityListener() { }
};

} // namespace blink

#endif // WebDeviceOrientationListener
