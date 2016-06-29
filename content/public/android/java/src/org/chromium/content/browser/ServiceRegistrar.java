// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.content.Context;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.content.browser.ServiceRegistry.ImplementationFactory;
import org.chromium.device.battery.BatteryMonitorFactory;
import org.chromium.device.vibration.VibrationManagerImpl;
import org.chromium.mojom.device.BatteryMonitor;
import org.chromium.mojom.device.VibrationManager;
import org.chromium.device.AppLauncher.AppLauncherManagerImpl;
import org.chromium.device.Calendar.CalendarManagerImpl;
import org.chromium.device.cpu.DeviceCpuManagerImpl;
import org.chromium.device.gallery.DeviceGalleryManagerImpl;
import org.chromium.device.sound.DeviceSoundManagerImpl;
import org.chromium.device.storage.DeviceStorageManagerImpl;
import org.chromium.device.contact.ContactManagerImpl;
import org.chromium.device.messaging.MessagingManagerImpl;
import org.chromium.mojom.device.CalendarManager;
import org.chromium.mojom.device.DeviceSoundManager;
import org.chromium.mojom.device.BatteryMonitor;
import org.chromium.mojom.device.VibrationManager;
import org.chromium.mojom.device.AppLauncherManager;
import org.chromium.mojom.device.MessagingManager;
import org.chromium.mojom.device.BatteryMonitor;
import org.chromium.mojom.device.VibrationManager;
import org.chromium.mojom.device.ContactManager;
import org.chromium.mojom.device.DeviceStorageManager;
import org.chromium.mojom.device.DeviceCpuManager;
import org.chromium.mojom.device.DeviceGalleryManager;

/**
 * Registers mojo services exposed by the browser in the given registry.
 */
@JNINamespace("content")
class ServiceRegistrar {
    // BatteryMonitorFactory can't implement ImplementationFactory itself, as we don't depend on
    // /content in /device. Hence we use BatteryMonitorImplementationFactory as a wrapper.
    private static class BatteryMonitorImplementationFactory
            implements ImplementationFactory<BatteryMonitor> {
        private final BatteryMonitorFactory mFactory;

        BatteryMonitorImplementationFactory(Context applicationContext) {
            mFactory = new BatteryMonitorFactory(applicationContext);
        }

        @Override
        public BatteryMonitor createImpl() {
            return mFactory.createMonitor();
        }
    }

    private static class VibrationManagerImplementationFactory
            implements ImplementationFactory<VibrationManager> {
        private final Context mApplicationContext;

        VibrationManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public VibrationManager createImpl() {
            return new VibrationManagerImpl(mApplicationContext);
        }
    }

    private static class ContactManagerImplementationFactory
            implements ImplementationFactory<ContactManager> {
        private final Context mApplicationContext;

        ContactManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public ContactManager createImpl() {
            return new ContactManagerImpl(mApplicationContext);
        }
    }

    private static class AppLauncherManagerImplementationFactory
            implements ImplementationFactory<AppLauncherManager> {
        private final Context mApplicationContext;

        AppLauncherManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public AppLauncherManager createImpl() {
            return new AppLauncherManagerImpl(mApplicationContext);
        }
    }
            
    private static class MessagingManagerImplementationFactory
            implements ImplementationFactory<MessagingManager> {
        private final Context mApplicationContext;

        MessagingManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public MessagingManager createImpl() {
            return new MessagingManagerImpl(mApplicationContext);
        }
    }

    private static class CalendarManagerImplementationFactory
            implements ImplementationFactory<CalendarManager> {
        private final Context mApplicationContext;

        CalendarManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public CalendarManager createImpl() {
            return new CalendarManagerImpl(mApplicationContext);
        }
    }

    private static class DeviceSoundManagerImplementationFactory
            implements ImplementationFactory<DeviceSoundManager> {
        private final Context mApplicationContext;

        DeviceSoundManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public DeviceSoundManager createImpl() {
            return new DeviceSoundManagerImpl(mApplicationContext);
        }
    }

    private static class DeviceStorageManagerImplementationFactory
            implements ImplementationFactory<DeviceStorageManager> {
        private final Context mApplicationContext;

        DeviceStorageManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public DeviceStorageManager createImpl() {
            return new DeviceStorageManagerImpl(mApplicationContext);
        }
    }

    private static class DeviceCpuManagerImplementationFactory
            implements ImplementationFactory<DeviceCpuManager> {
        private final Context mApplicationContext;

        DeviceCpuManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public DeviceCpuManager createImpl() {
            return new DeviceCpuManagerImpl(mApplicationContext);
        }
    }

    private static class DeviceGalleryManagerImplementationFactory
            implements ImplementationFactory<DeviceGalleryManager> {
        private final Context mApplicationContext;

        DeviceGalleryManagerImplementationFactory(Context applicationContext) {
            mApplicationContext = applicationContext;
        }

        @Override
        public DeviceGalleryManager createImpl() {
            return new DeviceGalleryManagerImpl(mApplicationContext);
        }
    }

    @CalledByNative
    static void registerProcessHostServices(ServiceRegistry registry, Context applicationContext) {
        assert applicationContext != null;
        registry.addService(BatteryMonitor.MANAGER,
                new BatteryMonitorImplementationFactory(applicationContext));
        registry.addService(AppLauncherManager.MANAGER,
                new AppLauncherManagerImplementationFactory(applicationContext));
        registry.addService(CalendarManager.MANAGER,
                new CalendarManagerImplementationFactory(applicationContext));
        registry.addService(ContactManager.MANAGER,
                new ContactManagerImplementationFactory(applicationContext));
        registry.addService(DeviceSoundManager.MANAGER,
                new DeviceSoundManagerImplementationFactory(applicationContext));
        registry.addService(DeviceStorageManager.MANAGER,
                new DeviceStorageManagerImplementationFactory(applicationContext));
        registry.addService(MessagingManager.MANAGER,
                new MessagingManagerImplementationFactory(applicationContext));
        registry.addService(DeviceCpuManager.MANAGER,
                new DeviceCpuManagerImplementationFactory(applicationContext));
        registry.addService(DeviceGalleryManager.MANAGER,
                new DeviceGalleryManagerImplementationFactory(applicationContext));
    }

    @CalledByNative
    static void registerFrameHostServices(ServiceRegistry registry, Context applicationContext) {
        assert applicationContext != null;
        registry.addService(VibrationManager.MANAGER,
                new VibrationManagerImplementationFactory(applicationContext));
        // TODO(avayvod): Register the PresentationService implementation here.
    }
}
