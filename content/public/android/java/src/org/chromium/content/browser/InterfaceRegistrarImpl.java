// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.content.Context;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.blink.mojom.FaceDetection;
import org.chromium.content.browser.shapedetection.FaceDetectionFactory;
import org.chromium.content_public.browser.InterfaceRegistrar;
import org.chromium.content_public.browser.WebContents;
import org.chromium.device.BatteryMonitor;
import org.chromium.device.VibrationManager;
import org.chromium.device.battery.BatteryMonitorFactory;
import org.chromium.device.nfc.mojom.Nfc;
import org.chromium.device.vibration.VibrationManagerImpl;
import org.chromium.mojo.system.impl.CoreImpl;
import org.chromium.services.service_manager.InterfaceRegistry;

import org.chromium.device.AppLauncherManager;
import org.chromium.device.AppLauncher.AppLauncherManagerImpl;
import org.chromium.device.CalendarManager;
import org.chromium.device.Calendar.CalendarManagerImpl;
import org.chromium.device.DeviceCpuManager;
import org.chromium.device.cpu.DeviceCpuManagerImpl;
import org.chromium.device.DeviceGalleryManager;
import org.chromium.device.gallery.DeviceGalleryManagerImpl;
import org.chromium.device.DeviceSoundManager;
import org.chromium.device.sound.DeviceSoundManagerImpl;
import org.chromium.device.DeviceStorageManager;
import org.chromium.device.storage.DeviceStorageManagerImpl;
import org.chromium.device.ContactManager;
import org.chromium.device.contact.ContactManagerImpl;
import org.chromium.device.MessagingManager;
import org.chromium.device.messaging.MessagingManagerImpl;
import org.chromium.device.DeviceThirdpartyManager;
import org.chromium.device.thirdparty.DeviceThirdpartyManagerImpl;
import org.chromium.base.BuildConfig;

@JNINamespace("content")
class InterfaceRegistrarImpl {
    @CalledByNative
    static void createInterfaceRegistryForContext(int nativeHandle, Context applicationContext) {
        ensureContentRegistrarsAreRegistered();

        InterfaceRegistry registry = InterfaceRegistry.create(
                CoreImpl.getInstance().acquireNativeHandle(nativeHandle).toMessagePipeHandle());
        InterfaceRegistrar.Registry.applyContextRegistrars(registry, applicationContext);
    }

    @CalledByNative
    static void createInterfaceRegistryForWebContents(int nativeHandle, WebContents webContents) {
        ensureContentRegistrarsAreRegistered();

        InterfaceRegistry registry = InterfaceRegistry.create(
                CoreImpl.getInstance().acquireNativeHandle(nativeHandle).toMessagePipeHandle());
        InterfaceRegistrar.Registry.applyWebContentsRegistrars(registry, webContents);
    }

    private static void ensureContentRegistrarsAreRegistered() {
        if (sHasRegisteredRegistrars) return;
        sHasRegisteredRegistrars = true;
        InterfaceRegistrar.Registry.addContextRegistrar(new ContentContextInterfaceRegistrar());
        InterfaceRegistrar.Registry.addWebContentsRegistrar(
                new ContentWebContentsInterfaceRegistrar());
    }

    private static boolean sHasRegisteredRegistrars;
}

class ContentContextInterfaceRegistrar implements InterfaceRegistrar<Context> {
    @Override
    public void registerInterfaces(InterfaceRegistry registry, final Context applicationContext) {
        registry.addInterface(
                VibrationManager.MANAGER, new VibrationManagerImpl.Factory(applicationContext));
        registry.addInterface(
                BatteryMonitor.MANAGER, new BatteryMonitorFactory(applicationContext));
        registry.addInterface(
                FaceDetection.MANAGER, new FaceDetectionFactory(applicationContext));
        if (BuildConfig.ENABLE_HIGHWEB_DEVICEAPI) {
          registry.addInterface(
                  AppLauncherManager.MANAGER, new AppLauncherManagerImpl.Factory(applicationContext));
          registry.addInterface(
                  CalendarManager.MANAGER, new CalendarManagerImpl.Factory(applicationContext));
          registry.addInterface(
                  ContactManager.MANAGER, new ContactManagerImpl.Factory(applicationContext));
          registry.addInterface(
                  DeviceSoundManager.MANAGER, new DeviceSoundManagerImpl.Factory(applicationContext));
          registry.addInterface(
                  DeviceStorageManager.MANAGER, new DeviceStorageManagerImpl.Factory(applicationContext));
          registry.addInterface(
                  MessagingManager.MANAGER, new MessagingManagerImpl.Factory(applicationContext));
          registry.addInterface(
                  DeviceCpuManager.MANAGER, new DeviceCpuManagerImpl.Factory(applicationContext));
          registry.addInterface(
                  DeviceGalleryManager.MANAGER, new DeviceGalleryManagerImpl.Factory(applicationContext));
          registry.addInterface(
                  DeviceThirdpartyManager.MANAGER, new DeviceThirdpartyManagerImpl.Factory(applicationContext));
        }
        // TODO(avayvod): Register the PresentationService implementation here.
    }
}

class ContentWebContentsInterfaceRegistrar implements InterfaceRegistrar<WebContents> {
    @Override
    public void registerInterfaces(InterfaceRegistry registry, final WebContents webContents) {
        registry.addInterface(Nfc.MANAGER, new NfcFactory(webContents));
    }
}
