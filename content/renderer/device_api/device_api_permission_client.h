/*
 * DeviceApiPermissionChecker.h
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#ifndef CONTENT_RENDERER_DEVICE_API_PERMISSION_DEVICE_API_PERMISSION_CLIENT_H_
#define CONTENT_RENDERER_DEVICE_API_PERMISSION_DEVICE_API_PERMISSION_CLIENT_H_

#include <map>
#include <deque>
#include <utility>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_thread.h"

#include "third_party/WebKit/public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

namespace content {

class CONTENT_EXPORT DeviceApiPermissionClient :
	public RenderFrameObserver,
	NON_EXPORTED_BASE(public blink::WebDeviceApiPermissionCheckClient) {
public:
	explicit DeviceApiPermissionClient(RenderFrame* render_frame);
	~DeviceApiPermissionClient() override;

	// WebDeviceApiPermissionCheckClient implements
	void CheckPermission(blink::WebDeviceApiPermissionCheckRequest* request) override;

	void OnPermissionChecked(int result);

  // RenderFrameObserver implementation
  void OnDestruct() override;
private:
	// RenderFrameObserver implementation.
	bool OnMessageReceived(const IPC::Message& message) override;
	void DidFinishLoad() override;

	void HandlePermissionRequest(blink::WebDeviceApiPermissionCheckRequest* request);
	void HandlePermissionResponse(blink::PermissionResult result, bool fromUI);

	typedef std::deque<blink::WebDeviceApiPermissionCheckRequest*> RequestsQueue;
	typedef std::map<std::string, bool> SavedResultMap;

	RequestsQueue request_queue_;
	SavedResultMap result_map_;
};

} // namespace content


#endif  // CONTENT_RENDERER_DEVICE_API_PERMISSION_DEVICE_API_PERMISSION_CLIENT_H_ */
