/*
 * DeviceProximityController.h
 *
 *  Created on: 2015. 10. 20.
 *      Author: azureskybox
 */

#ifndef DeviceProximityController_h
#define DeviceProximityController_h

#include "core/dom/Document.h"
#include "core/frame/DeviceSingleWindowEventController.h"
#include "modules/ModulesExport.h"

namespace blink {

class Event;

class MODULES_EXPORT DeviceProximityController final : public DeviceSingleWindowEventController, public Supplement<Document> {
	USING_GARBAGE_COLLECTED_MIXIN(DeviceProximityController);
public:
	~DeviceProximityController() override;

	static const char* supplementName();
	static DeviceProximityController& from(Document&);

	DECLARE_VIRTUAL_TRACE();

private:
    explicit DeviceProximityController(Document&);

    // Inherited from DeviceEventControllerBase.
    void registerWithDispatcher() override;
    void unregisterWithDispatcher() override;
    bool hasLastData() override;

    // Inherited from DeviceSingleWindowEventController.
    Event* lastEvent() const override;
    const AtomicString& eventTypeName() const override;
    bool isNullEvent(Event*) const override;
};

} // namespace blink

#endif // DeviceProximityController_h
