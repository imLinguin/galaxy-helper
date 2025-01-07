sources = ['main.c', 'service.c', 'galaxy.c', 'wine.c', 'cjson/cJSON.c']
headers = ['galaxy.h', 'protocols.h', 'service.h', 'wine.h', 'cjson/cJSON.h']

exe = executable('galaxy',
  [sources, headers],
  dependencies: [protobuf_c_dep, protobuf_compiled],
  link_args: ['-municode'],
  native: false,
  install : true)

test('basic', exe)
