// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.services.device;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.device.battery.BatteryMonitorFactory;
import org.chromium.device.mojom.BatteryMonitor;
import org.chromium.device.mojom.NfcProvider;
import org.chromium.device.mojom.VibrationManager;
import org.chromium.device.nfc.NfcDelegate;
import org.chromium.device.nfc.NfcProviderImpl;
import org.chromium.device.vibration.VibrationManagerImpl;
import org.chromium.mojo.system.impl.CoreImpl;
import org.chromium.services.service_manager.InterfaceRegistry;

import org.chromium.base.BuildConfig;
import org.chromium.device.calendar.CalendarManagerImpl;
import org.chromium.device.mojom.CalendarManager;
import org.chromium.device.contact.ContactManagerImpl;
import org.chromium.device.mojom.ContactManager;
import org.chromium.device.cpu.DeviceCpuManagerImpl;
import org.chromium.device.mojom.DeviceCpuManager;
import org.chromium.device.gallery.DeviceGalleryManagerImpl;
import org.chromium.device.mojom.DeviceGalleryManager;
import org.chromium.device.messaging.MessagingManagerImpl;
import org.chromium.device.mojom.MessagingManager;
import org.chromium.device.sound.DeviceSoundManagerImpl;
import org.chromium.device.mojom.DeviceSoundManager;
import org.chromium.device.storage.DeviceStorageManagerImpl;
import org.chromium.device.mojom.DeviceStorageManager;
import org.chromium.device.thirdparty.DeviceThirdpartyManagerImpl;
import org.chromium.device.mojom.DeviceThirdpartyManager;

@JNINamespace("device")
class InterfaceRegistrar {
    @CalledByNative
    static void createInterfaceRegistryForContext(
            int nativeHandle, NfcDelegate nfcDelegate) {
        // Note: The bindings code manages the lifetime of this object, so it
        // is not necessary to hold on to a reference to it explicitly.
        InterfaceRegistry registry = InterfaceRegistry.create(
                CoreImpl.getInstance().acquireNativeHandle(nativeHandle).toMessagePipeHandle());
        registry.addInterface(BatteryMonitor.MANAGER, new BatteryMonitorFactory());
        registry.addInterface(NfcProvider.MANAGER, new NfcProviderImpl.Factory(nfcDelegate));
        registry.addInterface(VibrationManager.MANAGER, new VibrationManagerImpl.Factory());
        if (BuildConfig.ENABLE_HIGHWEB_DEVICEAPI) {
            registry.addInterface(CalendarManager.MANAGER, new CalendarManagerImpl.Factory());
            registry.addInterface(ContactManager.MANAGER, new ContactManagerImpl.Factory());
            registry.addInterface(DeviceCpuManager.MANAGER, new DeviceCpuManagerImpl.Factory());
            registry.addInterface(DeviceGalleryManager.MANAGER, new DeviceGalleryManagerImpl.Factory());
            registry.addInterface(MessagingManager.MANAGER, new MessagingManagerImpl.Factory());
            registry.addInterface(DeviceSoundManager.MANAGER, new DeviceSoundManagerImpl.Factory());
            registry.addInterface(DeviceStorageManager.MANAGER, new DeviceStorageManagerImpl.Factory());
            registry.addInterface(DeviceThirdpartyManager.MANAGER, new DeviceThirdpartyManagerImpl.Factory());
        }
    }
}
