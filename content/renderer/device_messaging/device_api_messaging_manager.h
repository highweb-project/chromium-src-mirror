/*
 * device_api_contact_manager.h
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#ifndef CONTENT_RENDERER_DEVICE_API_DEVICE_API_MESSAGING_MANAGER_H_
#define CONTENT_RENDERER_DEVICE_API_DEVICE_API_MESSAGING_MANAGER_H_

#include <map>
#include <deque>
#include <vector>
#include <string>
#include <utility>
 
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "device/messaging/messaging_manager.mojom.h"
#include "mojo/public/cpp/bindings/array.h"

namespace blink {
class WebString;
class WebDeviceMessagingResultListener;
struct WebDeviceMessageFindOptions;
struct WebDeviceMessageObject;
struct WebDeviceMessagingParameter;
}

namespace content {

class CONTENT_EXPORT DeviceApiMessagingManager {
 public:
  explicit DeviceApiMessagingManager();
  ~DeviceApiMessagingManager();

  void FindMessage(blink::WebDeviceMessagingParameter* parameter);
  void DidFindMessage(int requestID, unsigned error, mojo::Array<device::MessageObjectPtr> results);

  void SendMessage(std::unique_ptr<blink::WebDeviceMessageObject> message);

  void AddListener(blink::WebDeviceMessagingParameter* parameter);
  void DidMessageReceived(int observerID, device::MessageObjectPtr results);

  void RemoveListener(blink::WebDeviceMessagingParameter* parameter);
 private:

  blink::WebDeviceMessageObject* ConvertToBlinkMessage(device::MessageObject* mojo_message);
  void ConvertToMojoMessage(device::MessageObject* message_ptr, blink::WebDeviceMessageObject* blink_message);

  device::MessagingManagerPtr monitor_;

  typedef std::map<int, blink::WebDeviceMessagingParameter*> FindRequestMap;
  FindRequestMap find_request_map_;

  typedef std::map<int, blink::WebDeviceMessagingParameter*> ObserverMap;
  ObserverMap observer_map_;

  DISALLOW_COPY_AND_ASSIGN(DeviceApiMessagingManager);
};

}  // namespace content

#endif  // CONTENT_RENDERER_DEVICE_API_DEVICE_API_MESSAGING_MANAGER_H_
