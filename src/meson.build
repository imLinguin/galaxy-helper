sources = ['main.c', 'service.c', 'galaxy.c', 'cjson/cJSON.c']

exe = executable('galaxy', sources,
  dependencies: [protobuf_c_dep, protobuf_compiled],
  native: false,
  install : true)

test('basic', exe)
