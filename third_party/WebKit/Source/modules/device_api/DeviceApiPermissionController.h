/*
 * DeviceApiPermissionController.h
 *
 *  Created on: 2015. 12. 1.
 *      Author: azureskybox
 */

#ifndef DeviceApiPermissionController_h
#define DeviceApiPermissionController_h

#include "modules/ModulesExport.h"
#include "core/frame/LocalFrame.h"
#include "platform/Supplementable.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

namespace blink {

class MODULES_EXPORT DeviceApiPermissionController final : public GarbageCollectedFinalized<DeviceApiPermissionController>, public Supplement<LocalFrame> {
	USING_GARBAGE_COLLECTED_MIXIN(DeviceApiPermissionController);
public:
	~DeviceApiPermissionController();// override;

    static void provideTo(LocalFrame&, WebDeviceApiPermissionCheckClient*);
    static DeviceApiPermissionController* from(LocalFrame&);
    static const char* supplementName();

    WebDeviceApiPermissionCheckClient* client() { return m_client; };

	DECLARE_VIRTUAL_TRACE();

private:
	DeviceApiPermissionController(LocalFrame&, WebDeviceApiPermissionCheckClient*);

	WebDeviceApiPermissionCheckClient* m_client;
};

}

#endif  // DeviceApiPermissionController_h
