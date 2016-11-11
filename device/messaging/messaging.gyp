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
      'target_name': 'device_messaging_mojo_bindings',
      'type': 'static_library',
      'includes': [
        '../../mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        'messaging_manager.mojom',
      ],
    },
    {
      # GN version: //device/messaging:mojo_bindings_blink
      'target_name': 'device_messaging_for_blink',
      'type': 'static_library',
      'variables': {
        'for_blink': 'true',
      },
      'includes': [
        '../../mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        'messaging_manager.mojom',
      ],
    },
    {
      'target_name': 'device_messaging',
      'type': '<(component)',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        #'../../mojo/mojo_base.gyp:mojo_environment_chromium',
        '../../mojo/mojo_edk.gyp:mojo_system_impl',
        '../../mojo/mojo_public.gyp:mojo_cpp_bindings',
        'device_messaging_mojo_bindings',
      ],
      'defines': [
        'DEVICE_MESSAGING_IMPLEMENTATION',
      ],
      'sources': [
        'messaging_manager_impl.h',
        'messaging_manager_impl_default.cc',
      ],
      'conditions': [
         ['OS == "android"', {
           'dependencies': [
             'device_messaging_java',
           ],
           'sources!': [
             'messaging_manager_impl_default.cc',
           ],
         }],
       ],
    },
  ],
  'conditions': [
	['OS == "android"', {
   'targets': [
     {
       'target_name': 'device_messaging_java',
       'type': 'none',
       'dependencies': [
         '../../base/base.gyp:base',
         '../../mojo/mojo_public.gyp:mojo_bindings_java',
         'device_messaging_mojo_bindings',
       ],
       'variables': {
         'java_in_dir': '../../device/messaging/android/java',
       },
       'includes': [ '../../build/java.gypi' ],
         },
       ],
     }],
   ],
}
