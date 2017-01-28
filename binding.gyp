{
  'targets': [
    {
      'target_name': 'memwatch',
      'include_dirs': [
        "<!(node -e \"require('nan')\")"
      ],
      'sources': [
        'src/compat-inl.hh',
        'src/compat.hh',
        'src/snapshot-posix.hh',
        'src/snapshot-win32.hh',
        'src/fileoutputstream.cc',
        'src/heapdiff.cc',
        'src/init.cc',
        'src/memwatch.cc',
        'src/util.cc'
      ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'OTHER_CFLAGS' : ['-fpermissive']
          }
        }]
      ]
    }
  ]
}
