#include "platform_applicationinfo.h"

namespace blink {

  PlatformApplicationInfo::PlatformApplicationInfo() {
  }

  PlatformApplicationInfo::~PlatformApplicationInfo() {
  }

  void PlatformApplicationInfo::setName(String name) {
    mName = name;
  }

  void PlatformApplicationInfo::setClassName(String className) {
    mClassName = className;
  }

  void PlatformApplicationInfo::setDataDir(String dataDir) {
    mDataDir = dataDir;
  }

  void PlatformApplicationInfo::setEnabled(bool enabled) {
    mEnabled = enabled;
  }

  void PlatformApplicationInfo::setFlags(long flags) {
    mFlags = flags;
  }

  void PlatformApplicationInfo::setPermission(String permission) {
    mPermission = permission;
  }

  void PlatformApplicationInfo::setProcessName(String processName) {
    mProcessName = processName;
  }

  void PlatformApplicationInfo::setTargetSdkVersion(long targetSdkVersion) {
    mTargetSdkVersion = targetSdkVersion;
  }

  void PlatformApplicationInfo::setTheme(long theme) {
    mTheme = theme;
  }

  void PlatformApplicationInfo::setUid(long uid) {
    mUid = uid;
  }

  void PlatformApplicationInfo::setPackageName(String packageName) {
    mPackageName = packageName;
  }

  String PlatformApplicationInfo::name() {
    return mName;
  }

  String PlatformApplicationInfo::className() {
    return mClassName;
  }

  String PlatformApplicationInfo::dataDir() {
    return mDataDir;
  }

  bool PlatformApplicationInfo::enabled() {
    return mEnabled;
  }

  long PlatformApplicationInfo::flags() {
    return mFlags;
  }

  String PlatformApplicationInfo::permission() {
    return mPermission;
  }

  String PlatformApplicationInfo::processName() {
    return mProcessName;
  }

  long PlatformApplicationInfo::targetSdkVersion() {
    return mTargetSdkVersion;
  }

  long PlatformApplicationInfo::theme() {
    return mTheme;
  }

  long PlatformApplicationInfo::uid() {
    return mUid;
  }

  String PlatformApplicationInfo::packageName() {
    return mPackageName;
  }

  DEFINE_TRACE(PlatformApplicationInfo) {
  }

}
