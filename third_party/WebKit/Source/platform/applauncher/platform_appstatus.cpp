#include "platform_appstatus.h"

namespace blink {

  PlatformAppStatus::PlatformAppStatus() {
  }

  PlatformAppStatus::~PlatformAppStatus() {
  }

  void PlatformAppStatus::setResultCode(int code) {
		mResultCode = code;
	}

  DEFINE_TRACE(PlatformAppStatus) {
  }

}
