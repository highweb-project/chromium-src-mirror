/*
 * device_api_infobar_delegate.h
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#include <string>

#ifndef CONTENT_BROWSER_DEVICE_API_DEVICE_API_INFOBAR_DELEGATE_H_
#define CONTENT_BROWSER_DEVICE_API_DEVICE_API_INFOBAR_DELEGATE_H_

#include "base/compiler_specific.h"
#include "components/infobars/core/confirm_infobar_delegate.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/common/device_api_permission_request.h"

class DeviceApiInfoBarDelegate : public ConfirmInfoBarDelegate {
public:
	~DeviceApiInfoBarDelegate() override;

	static bool Create(content::WebContents* web_contents, const content::DeviceApiPermissionRequest& request, const content::DeviceApiPermissionCallback& callback);

	// The identifier is unique across all Chrome processes.
  	infobars::InfoBarDelegate::InfoBarIdentifier GetIdentifier() const override;

private:
	DeviceApiInfoBarDelegate(const content::DeviceApiPermissionRequest& request);

	Type GetInfoBarType() const override;
	int GetIconId() const override;
	void InfoBarDismissed() override;
	base::string16 GetMessageText() const override;
	base::string16 GetButtonLabel(InfoBarButton button) const override;
	bool Accept() override;
	bool Cancel() override;
	base::string16 GetLinkText() const override;
	GURL GetLinkURL() const override;

	content::DeviceApiPermissionRequest request_;
};


#endif /* CONTENT_BROWSER_DEVICE_API_DEVICE_API_INFOBAR_DELEGATE_H_ */
