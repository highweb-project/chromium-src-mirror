#ifndef DevToolsCallback_h
#define DevToolsCallback_h

#include "platform/heap/Handle.h"

namespace blink {

class DevToolsCallback : public GarbageCollectedFinalized<DevToolsCallback> {
public:
    virtual ~DevToolsCallback() { }
    DEFINE_INLINE_VIRTUAL_TRACE() { }
    virtual void handleEvent(const WTF::String& result) = 0;
};

} // namespace blink

#endif // DevToolsCallback_h
