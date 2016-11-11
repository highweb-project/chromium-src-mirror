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
      # GN version: //device/applauncher:mojo_bindings
      'target_name': 'device_applauncher_mojo_bindings',
      'type': 'static_library',
      'includes': [
        '../../mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        'applauncher_manager.mojom',
        'applauncher_ResultCode.mojom',
      ],
    },
    {
      # GN version: //device/applauncher:mojo_bindings_blink
      'target_name': 'device_applauncher_for_blink',
      'type': 'static_library',
      'variables': {
        'for_blink': 'true',
      },
      'includes': [
        '../../mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        'applauncher_manager.mojom',
        'applauncher_ResultCode.mojom',
      ],
    },
    {
      # GN version: //device/applauncher
      'target_name': 'device_applauncher',
      'type': '<(component)',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        #'../../mojo/mojo_base.gyp:mojo_environment_chromium',
        '../../mojo/mojo_edk.gyp:mojo_system_impl',
        '../../mojo/mojo_public.gyp:mojo_cpp_bindings',
        'device_applauncher_mojo_bindings',
      ],
      'defines': [
        'DEVICE_APPLAUNCHER_IMPLEMENTATION',
      ],
      'sources': [
        'applauncher_manager_impl.h',
        'applauncher_manager_impl_default.cc',
      ],
      'conditions': [
        ['OS == "android"', {
          'dependencies': [
            'device_applauncher_java',
          ],
          'sources!': [
            'applauncher_manager_impl_default.cc',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['OS == "android"', {
      'targets': [
        {
          'target_name': 'device_applauncher_java',
          'type': 'none',
          'dependencies': [
            '../../base/base.gyp:base',
            '../../mojo/mojo_public.gyp:mojo_bindings_java',
            'device_applauncher_mojo_bindings',
          ],
          'variables': {
            'java_in_dir': '../../device/applauncher/android/java',
          },
          'includes': [ '../../build/java.gypi' ],
        },
      ],
    }],
  ],
}
