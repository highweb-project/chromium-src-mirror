#include "platform_calendarstatus.h"
namespace blink {

  PlatformCalendarStatus::PlatformCalendarStatus() {
  }

  PlatformCalendarStatus::~PlatformCalendarStatus() {
  }

  void PlatformCalendarStatus::setResultCode(int code) {
		mResultCode = code;
	}

  DEFINE_TRACE(PlatformCalendarStatus) {
  }

}
