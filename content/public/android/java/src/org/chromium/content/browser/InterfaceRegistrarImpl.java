// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.content.Context;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.content.browser.androidoverlay.AndroidOverlayProviderImpl;
import org.chromium.content_public.browser.InterfaceRegistrar;
import org.chromium.content_public.browser.RenderFrameHost;
import org.chromium.content_public.browser.WebContents;
import org.chromium.media.mojom.AndroidOverlayProvider;
import org.chromium.mojo.system.impl.CoreImpl;
import org.chromium.services.service_manager.InterfaceRegistry;

import android.util.Log;
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

@JNINamespace("content")
class InterfaceRegistrarImpl {

    private static boolean sHasRegisteredRegistrars;

    @CalledByNative
    static void createInterfaceRegistryForContext(int nativeHandle) {
        ensureContentRegistrarsAreRegistered();

        InterfaceRegistry registry = InterfaceRegistry.create(
                CoreImpl.getInstance().acquireNativeHandle(nativeHandle).toMessagePipeHandle());
        InterfaceRegistrar.Registry.applyContextRegistrars(registry);
    }

    @CalledByNative
    static void createInterfaceRegistryForWebContents(int nativeHandle, WebContents webContents) {
        ensureContentRegistrarsAreRegistered();

        InterfaceRegistry registry = InterfaceRegistry.create(
                CoreImpl.getInstance().acquireNativeHandle(nativeHandle).toMessagePipeHandle());
        InterfaceRegistrar.Registry.applyWebContentsRegistrars(registry, webContents);
    }

    @CalledByNative
    static void createInterfaceRegistryForRenderFrameHost(
            int nativeHandle, RenderFrameHost renderFrameHost) {
        ensureContentRegistrarsAreRegistered();

        InterfaceRegistry registry = InterfaceRegistry.create(
                CoreImpl.getInstance().acquireNativeHandle(nativeHandle).toMessagePipeHandle());
        InterfaceRegistrar.Registry.applyRenderFrameHostRegistrars(registry, renderFrameHost);
    }

    private static void ensureContentRegistrarsAreRegistered() {
        if (sHasRegisteredRegistrars) return;
        sHasRegisteredRegistrars = true;
        InterfaceRegistrar.Registry.addContextRegistrar(new ContentContextInterfaceRegistrar());
    }

    private static class ContentContextInterfaceRegistrar implements InterfaceRegistrar<Context> {
        @Override
        public void registerInterfaces(
                InterfaceRegistry registry, final Context applicationContext) {
            registry.addInterface(AndroidOverlayProvider.MANAGER,
                    new AndroidOverlayProviderImpl.Factory(applicationContext));
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
            // TODO(avayvod): Register the PresentationService implementation here.
        }
    }
}
