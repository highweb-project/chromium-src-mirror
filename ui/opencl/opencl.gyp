{
  'variables': {
    'chromium_code': 1,
  },

  'targets': [
    {
      'target_name': 'openlc_impl',
      'type': 'executable',
      'include_dirs': [
        '<(DEPTH)/third_party/opencl'
      ],
      'sources': [
        'opencl_test.h',
        'opencl_test.cc'
      ]
    },
  ]
}