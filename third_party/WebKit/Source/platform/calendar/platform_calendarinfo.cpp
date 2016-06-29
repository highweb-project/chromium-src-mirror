#include "platform_calendarinfo.h"

namespace blink {

  PlatformCalendarInfo::PlatformCalendarInfo() {
  }

  PlatformCalendarInfo::~PlatformCalendarInfo() {
  }

  String PlatformCalendarInfo::id() {
    return mId;
  }
	String PlatformCalendarInfo::description() {
    return mDescription;
  }
	String PlatformCalendarInfo::location() {
    return mLocation;
  }
	String PlatformCalendarInfo::summary() {
    return mSummary;
  }
	String PlatformCalendarInfo::start() {
    return mStart;
  }
	String PlatformCalendarInfo::end() {
    return mEnd;
  }
	String PlatformCalendarInfo::status() {
    return mStatus;
  }
	String PlatformCalendarInfo::transparency() {
    return mTransparency;
  }
	String PlatformCalendarInfo::reminder() {
    return mReminder;
  }

  void PlatformCalendarInfo::setId(String value) {
    mId = value;
  }
  void PlatformCalendarInfo::setDescription(String value) {
    mDescription = value;
  }
  void PlatformCalendarInfo::setLocation(String value) {
    mLocation = value;
  }
  void PlatformCalendarInfo::setSummary(String value) {
    mSummary = value;
  }
  void PlatformCalendarInfo::setStart(String value) {
    mStart = value;
  }
  void PlatformCalendarInfo::setEnd(String value) {
    mEnd = value;
  }
  void PlatformCalendarInfo::setStatus(String value) {
    mStatus = value;
  }
  void PlatformCalendarInfo::setTransparency(String value) {
    mTransparency = value;
  }
  void PlatformCalendarInfo::setReminder(String value) {
    mReminder = value;
  }

  DEFINE_TRACE(PlatformCalendarInfo) {
  }

}
