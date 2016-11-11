# Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      # GN version: //device/calendar:mojo_bindings
      'target_name': 'device_calendar_mojo_bindings',
      'type': 'static_library',
      'includes': [
        '../../mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        'calendar_manager.mojom',
        'calendar_ResultCode.mojom',
      ],
    },
    {
      # GN version: //device/contact:mojo_bindings_blink
      'target_name': 'device_calendar_for_blink',
      'type': 'static_library',
      'variables': {
        'for_blink': 'true',
      },
      'includes': [
        '../../mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        'calendar_manager.mojom',
        'calendar_ResultCode.mojom',
      ],
    },
    {
      # GN version: //device/calendar
      'target_name': 'device_calendar',
      'type': '<(component)',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        #'../../mojo/mojo_base.gyp:mojo_environment_chromium',
        '../../mojo/mojo_edk.gyp:mojo_system_impl',
        '../../mojo/mojo_public.gyp:mojo_cpp_bindings',
        'device_calendar_mojo_bindings',
      ],
      'defines': [
        'DEVICE_CALENDAR_IMPLEMENTATION',
      ],
      'sources': [
        'calendar_manager_impl.h',
        'calendar_manager_impl_default.cc',
      ],
      'conditions': [
        ['OS == "android"', {
          'dependencies': [
            'device_calendar_java',
          ],
          'sources!': [
            'calendar_manager_impl_default.cc',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['OS == "android"', {
      'targets': [
        {
          'target_name': 'device_calendar_java',
          'type': 'none',
          'dependencies': [
            '../../base/base.gyp:base',
            '../../mojo/mojo_public.gyp:mojo_bindings_java',
            'device_calendar_mojo_bindings',
          ],
          'variables': {
            'java_in_dir': '../../device/calendar/android/java',
          },
          'includes': [ '../../build/java.gypi' ],
        },
      ],
    }],
  ],
}
