/*
 * device_api_infobar_delegate.cc
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#include "chrome/browser/device_api/device_api_infobar_delegate.h"

#include "base/callback_helpers.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/infobars/infobar_service.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/generated_resources.h"
#include "components/google/core/browser/google_util.h"
#include "components/infobars/core/infobar.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/origin_util.h"
#include "components/strings/grit/components_strings.h"
#include "chrome/grit/theme_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"

#if defined(OS_ANDROID)
#include "chrome/browser/android/android_theme_resources.h"
#endif

bool DeviceApiInfoBarDelegate::Create(content::WebContents* web_contents, const content::DeviceApiPermissionRequest& request, const content::DeviceApiPermissionCallback& callback)
{
	InfoBarService* infobar_service = InfoBarService::FromWebContents(web_contents);

	std::unique_ptr<infobars::InfoBar> infobar(
	  infobar_service->CreateConfirmInfoBar(std::unique_ptr<ConfirmInfoBarDelegate>(
		  new DeviceApiInfoBarDelegate(request))));

	infobar_service->AddInfoBar(std::move(infobar));

	return true;
}

infobars::InfoBarDelegate::Type
DeviceApiInfoBarDelegate::GetInfoBarType() const {
  return PAGE_ACTION_TYPE;
}

int DeviceApiInfoBarDelegate::GetIconId() const {
#if defined(OS_ANDROID)
	return IDR_ANDROID_INFOBAR_WARNING;
#else
  return IDR_WARNING;
#endif
}

void DeviceApiInfoBarDelegate::InfoBarDismissed() {
	request_.callback_.Run(content::DeviceApiPermissionRequestResult::RESULT_DENIED);
}

base::string16 DeviceApiInfoBarDelegate::GetMessageText() const {
	int message_id = IDS_DEVICE_API_PERMISSION_REQUEST;
	int api_name__id = IDS_DEVICE_API_TITLE_CONTACT;

	if(request_.operation_type_ == content::OperationType::VIEW) {
		message_id = IDS_DEVICE_API_PERMISSION_REQUEST_VIEW;
	}
	else if(request_.operation_type_ == content::OperationType::MODIFY) {
		message_id = IDS_DEVICE_API_PERMISSION_REQUEST_MODIFY;
	}

	switch(request_.api_type_) {
	case content::ApiType::CONTACT:
		api_name__id = IDS_DEVICE_API_TITLE_CONTACT;
		break;
	case content::ApiType::CALENDAR:
		api_name__id = IDS_DEVICE_API_TITLE_CALENDAR;
		break;
	case content::ApiType::MESSAGING:
		api_name__id = IDS_DEVICE_API_TITLE_MESSAGING;
		break;
	case content::ApiType::GALLERY:
		api_name__id = IDS_DEVICE_API_TITLE_GALLERY;
		break;
	case content::ApiType::APPLICATION_LAUNCHER:
		api_name__id = IDS_DEVICE_API_TITLE_APPLICATION_LAUNCHER;
		break;
	case content::ApiType::SYSTEM_INFORMATION:
		api_name__id = IDS_DEVICE_API_TITLE_SYSTEM_INFORMATION;
		break;
	}

  return l10n_util::GetStringFUTF16(message_id, l10n_util::GetStringUTF16(api_name__id));
}

base::string16 DeviceApiInfoBarDelegate::GetButtonLabel(InfoBarButton button) const {
  return l10n_util::GetStringUTF16((button == BUTTON_OK) ?
      IDS_MEDIA_CAPTURE_ALLOW : IDS_MEDIA_CAPTURE_BLOCK);
}

bool DeviceApiInfoBarDelegate::Accept() {
	request_.callback_.Run(content::DeviceApiPermissionRequestResult::RESULT_OK);

	return true;
}

bool DeviceApiInfoBarDelegate::Cancel() {
	request_.callback_.Run(content::DeviceApiPermissionRequestResult::RESULT_DENIED);

	return true;
}

base::string16 DeviceApiInfoBarDelegate::GetLinkText() const {
  return base::string16();
}

GURL DeviceApiInfoBarDelegate::GetLinkURL() const {
  return GURL(chrome::kMediaAccessLearnMoreUrl);
}

DeviceApiInfoBarDelegate::DeviceApiInfoBarDelegate(const content::DeviceApiPermissionRequest& request)
	: request_(request)
{

}

DeviceApiInfoBarDelegate::~DeviceApiInfoBarDelegate()
{

}

infobars::InfoBarDelegate::InfoBarIdentifier DeviceApiInfoBarDelegate::GetIdentifier() const
{
	return DEVICE_API_INFOBAR_DELEGATE;
}
