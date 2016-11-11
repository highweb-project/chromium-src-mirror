# Copyright 2014 The Chromium Authors. All rights reserved.
# Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      # GN version: //device/cpu:mojo_bindings
      'target_name': 'device_cpu_mojo_bindings',
      'type': 'static_library',
      'includes': [
        '../../mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        'devicecpu_manager.mojom',
        'devicecpu_ResultCode.mojom',
      ],
    },
    {
      # GN version: //device/cpu:mojo_bindings_blink
      'target_name': 'device_cpu_for_blink',
      'type': 'static_library',
      'variables': {
        'for_blink': 'true',
      },
      'includes': [
        '../../mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        'devicecpu_manager.mojom',
        'devicecpu_ResultCode.mojom',
      ],
    },
    {
      # GN version: //device/cpu
      'target_name': 'device_cpu',
      'type': '<(component)',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        #'../../mojo/mojo_base.gyp:mojo_environment_chromium',
        '../../mojo/mojo_edk.gyp:mojo_system_impl',
        '../../mojo/mojo_public.gyp:mojo_cpp_bindings',
        'device_cpu_mojo_bindings',
      ],
      'defines': [
        'DEVICE_CPU_IMPLEMENTATION',
      ],
      'sources': [
        'devicecpu_manager_impl.h',
        'devicecpu_manager_impl_default.cc',
      ],
      'conditions': [
        ['OS == "android"', {
          'dependencies': [
            'device_cpu_java',
          ],
          'sources!': [
            'devicecpu_manager_impl_default.cc',
          ],
        }],
        ['OS == "linux"', {
          'sources!': [
            'devicecpu_manager_impl_default.cc',
          ],
          'sources': [
            'devicecpu_manager_linux.cc',
            'devicecpu_manager_linux.h',
            'devicecpu_service_linux.cc',
            'devicecpu_service_linux.h',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['OS == "android"', {
      'targets': [
        {
          'target_name': 'device_cpu_java',
          'type': 'none',
          'dependencies': [
            '../../base/base.gyp:base',
            '../../mojo/mojo_public.gyp:mojo_bindings_java',
            'device_cpu_mojo_bindings',
          ],
          'variables': {
            'java_in_dir': '../../device/cpu/android/java',
          },
          'includes': [ '../../build/java.gypi' ],
        },
      ],
    }],
  ],
}
