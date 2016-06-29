/*
 * device_api_contact_manager.h
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#ifndef CONTENT_RENDERER_DEVICE_API_DEVICE_API_CONTACT_MANAGER_H_
#define CONTENT_RENDERER_DEVICE_API_DEVICE_API_CONTACT_MANAGER_H_

#include <map>
#include <deque>
#include <vector>
#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "device/contact/contact_manager.mojom.h"
#include "mojo/public/cpp/bindings/array.h"
#include <utility>
 
namespace blink {
struct WebDeviceContactFindOptions;
struct WebDeviceContactParameter;
class WebString;
struct WebDeviceContactObject;
}

namespace content {

class CONTENT_EXPORT DeviceApiContactManager {
 public:
  explicit DeviceApiContactManager();
  ~DeviceApiContactManager();

  void checkPermission();
  void didCheckPermission(bool result);

  void findContact(blink::WebDeviceContactParameter* parameter);
  void didFindContact(int requestID, unsigned error, mojo::Array<device::ContactObjectPtr> results);

  void addContact(blink::WebDeviceContactParameter* parameter);
  void didAddContact(int requestID, unsigned error, mojo::Array<device::ContactObjectPtr> results);

  void deleteContact(blink::WebDeviceContactParameter* parameter);
  void didDeleteContact(int requestID, unsigned error, mojo::Array<device::ContactObjectPtr> results);

  void updateContact(blink::WebDeviceContactParameter* parameter);
  void didUpdateContact(int requestID, unsigned error, mojo::Array<device::ContactObjectPtr> results);
 private:

  blink::WebDeviceContactObject* convertToBlinkContact(device::ContactObject* mojoContact);
  void convertToMojoContact(device::ContactObject* contactPtr, blink::WebDeviceContactObject* blinkContact);

  device::ContactManagerPtr monitor_;

  typedef std::deque<blink::WebDeviceContactFindOptions*> FindRequestsQueue;
  FindRequestsQueue find_request_queue_;

  typedef std::map<int, blink::WebDeviceContactParameter*> FindRequestsMap;
  FindRequestsMap find_request_map_;

  typedef std::map<int, blink::WebDeviceContactParameter*> AddRequestsMap;
  AddRequestsMap add_request_map_;

  typedef std::map<int, blink::WebDeviceContactParameter*> DeleteRequestsMap;
  DeleteRequestsMap delete_request_map_;

  typedef std::map<int, blink::WebDeviceContactParameter*> UpdateRequestsMap;
  DeleteRequestsMap update_request_map_;

  DISALLOW_COPY_AND_ASSIGN(DeviceApiContactManager);
};

}  // namespace content

#endif  // CONTENT_RENDERER_DEVICE_API_DEVICE_API_CONTACT_MANAGER_H_ */
