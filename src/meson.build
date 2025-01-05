sources = ['main.c', 'service.c', 'galaxy.c', 'cjson/cJSON.c']
headers = ['galaxy.h', 'protocols.h', 'service.h', 'cjson/cJSON.h']

exe = executable('galaxy',
  [sources, headers],
  dependencies: [protobuf_c_dep, protobuf_compiled],
  native: false,
  install : true)

test('basic', exe)
