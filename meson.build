project('galaxy_helper', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

protobuf_c_lib = static_library('protobuf-c', 'protobuf-c/protobuf-c/protobuf-c.c', native: false)
protobuf_c_dep = declare_dependency(link_with: protobuf_c_lib, include_directories: 'protobuf-c')

subdir('protocols')
subdir('src')
subdir('unixlib')
