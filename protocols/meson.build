protoc = find_program('protoc')

proto_files = files(
  'gog_protocols/galaxy.common.protocols.connection.proto',
  'gog_protocols/galaxy.common.protocols.peer_common.proto',
  'gog_protocols/galaxy.common.protocols.peer_to_peer.proto',
  'gog_protocols/galaxy.common.protocols.peer_to_server.proto',
  'gog_protocols/galaxy.protocols.communication_service.proto',
  'gog_protocols/galaxy.protocols.overlay_for_peer.proto',
  'gog_protocols/galaxy.protocols.webbroker_service.proto',
  'gog_protocols/gog.protocols.pb.proto'
)

proto_include = '-I../protocols/gog_protocols'

generated = custom_target(
  'protoc',
  output: [
    'galaxy.common.protocols.connection.pb-c.c',
    'galaxy.common.protocols.connection.pb-c.h',
    'galaxy.common.protocols.peer_common.pb-c.c',
    'galaxy.common.protocols.peer_common.pb-c.h',
    'galaxy.common.protocols.peer_to_peer.pb-c.c',
    'galaxy.common.protocols.peer_to_peer.pb-c.h',
    'galaxy.common.protocols.peer_to_server.pb-c.c',
    'galaxy.common.protocols.peer_to_server.pb-c.h',
    'galaxy.protocols.communication_service.pb-c.c',
    'galaxy.protocols.communication_service.pb-c.h',
    'galaxy.protocols.overlay_for_peer.pb-c.c',
    'galaxy.protocols.overlay_for_peer.pb-c.h',
    'galaxy.protocols.webbroker_service.pb-c.c',
    'galaxy.protocols.webbroker_service.pb-c.h',
    'gog.protocols.pb.pb-c.c',
    'gog.protocols.pb.pb-c.h'
    ],
  input: proto_files,
  command: [
    protoc,
    proto_include,
    '--c_out=@OUTDIR@',
    '@INPUT@'
  ]
  )

protobuf_compiled = declare_dependency(
  sources: [generated],
  include_directories: include_directories('.'))
