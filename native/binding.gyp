{
  "targets": [
    {
      "target_name": "statvfs",
      "sources": [
        "statvfs.cc",
        "errno-error.cc"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [
        "-fno-exceptions"
      ],
      "cflags_cc!": [
        "-fno-exceptions"
      ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7"
      }
    },
    {
        "target_name": "copy_binary",
        "type":"none",
        "dependencies" : [ "statvfs" ],
        "copies": [
        {
          "destination": "<(module_root_dir)/<!(node -p '`${os.type()}-${os.arch()}`.toLowerCase()')",
          "files": [ "<(module_root_dir)/build/Release/statvfs.node" ]
        }
      ]
    }
  ]
}
