#include "devicecpu_manager_linux.h"
#include "base/message_loop/message_loop.h"

namespace device {

DeviceCpuManagerLinux::DeviceCpuManagerLinux(mojom::DeviceCpuManagerRequest request)
  : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {
    isAlive = true;
    main_thread_task_runner_ = base::MessageLoop::current()->task_runner();
}
DeviceCpuManagerLinux::~DeviceCpuManagerLinux() {
  isAlive = false;
  if (callbackThread) {
    delete callbackThread;
  }
  if (monitorThread) {
    delete monitorThread;
  }
  deviceCpuLoadCallbackList.clear();
}

void DeviceCpuManagerLinux::getDeviceCpuLoad(getDeviceCpuLoadCallback callback) {
  if (!callbackThread) {
    callbackThread = new DeviceCpuCallbackThread();
  }
  deviceCpuLoadCallbackList.push_back(std::move(callback));
  callbackThread->startCallbackThread(base::Bind(&DeviceCpuManagerLinux::callbackCpuLoad, base::Unretained(this)));
}

void DeviceCpuManagerLinux::callbackCpuLoad() {
  if (isAlive) {
    main_thread_task_runner_->PostTask(FROM_HERE, base::Bind(&DeviceCpuManagerLinux::callbackMojo, base::Unretained(this)));
  }
}

void DeviceCpuManagerLinux::callbackMojo() {
  mojom::DeviceCpu_ResultCodePtr result(mojom::DeviceCpu_ResultCode::New());
  result->functionCode = int32_t(mojom::device_cpu_function::FUNC_GET_CPU_LOAD);
  result->load = load_;
  if (load_ == -1.0f) {
    result->resultCode = int32_t(mojom::device_cpu_ErrorCodeList::FAILURE);
  } else {
    result->resultCode = int32_t(mojom::device_cpu_ErrorCodeList::SUCCESS);
  }
  while(!deviceCpuLoadCallbackList.empty()) {
    getDeviceCpuLoadCallback callback = std::move(deviceCpuLoadCallbackList.front());
    std::move(callback).Run(result.Clone());
    deviceCpuLoadCallbackList.pop_front();
  }
}

void DeviceCpuManagerLinux::startCpuLoad() {
  if (!monitorThread) {
    monitorThread = new DeviceCpuMonitorThread();
  }
  isAlive = true;
  DeviceCpuMonitorThread::DeviceCpuMonitorCallback callback = base::Bind(&DeviceCpuManagerLinux::callbackLoad, base::Unretained(this));
  monitorThread->startMonitor(callback);
}

void DeviceCpuManagerLinux::stopCpuLoad() {
  isAlive = false;
  monitorThread->stopMonitor();
}

void DeviceCpuManagerLinux::callbackLoad(const float& load) {
  load_ = load;
  if (isAlive) {
    startCpuLoad();
  }
}

}  // namespace device
