{
  "targets": [
    {
      "target_name": "neurose",
      "sources": [ "neurose.cpp" , "jsmn.cpp" , "request.cpp" ],
      'link_settings': {
                'libraries': [
                    '-lhiredis'
                ],
                'ldflags': [
                    '-L..'
                ]
            },
      "conditions": [
        [ 'OS=="mac"', {
          "xcode_settings": {
            "OTHER_CPLUSPLUSFLAGS" : [
                "-O3",
                "-I.",
                "-I/usr/include",
                "-I../include"
            ],
            "OTHER_LDFLAGS": [ "-L.","-lhiredis",'-luuid' ],
            "MACOSX_DEPLOYMENT_TARGET": "10.7"
          },
        }],
        [ 'OS=="linux"', {
            'cflags+' : [ '-O3', '-I.','-I../include'  ],
            'cflags_c+' : [ '-O3', '-I.','-I../include'],
            'cflags_cc+' : [ '-O3', '-I.','-I../include'],
            'link_settings' : {
                'libraries' : [
                    '-lhiredis', '-luuid'
                ],
                'ldflags': [
                    '-L..'
                ]
            }
        }
        ]
      ]
     }
  ]
}
