/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: galaxy.common.protocols.peer_common.proto */

#ifndef PROTOBUF_C_galaxy_2ecommon_2eprotocols_2epeer_5fcommon_2eproto__INCLUDED
#define PROTOBUF_C_galaxy_2ecommon_2eprotocols_2epeer_5fcommon_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1005000 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct Galaxy__Common__Protocols__PeerCommon__Property Galaxy__Common__Protocols__PeerCommon__Property;


/* --- enums --- */


/* --- messages --- */

struct  Galaxy__Common__Protocols__PeerCommon__Property
{
  ProtobufCMessage base;
  char *name;
  char *value;
};
#define GALAXY__COMMON__PROTOCOLS__PEER_COMMON__PROPERTY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&galaxy__common__protocols__peer_common__property__descriptor) \
, NULL, NULL }


/* Galaxy__Common__Protocols__PeerCommon__Property methods */
void   galaxy__common__protocols__peer_common__property__init
                     (Galaxy__Common__Protocols__PeerCommon__Property         *message);
size_t galaxy__common__protocols__peer_common__property__get_packed_size
                     (const Galaxy__Common__Protocols__PeerCommon__Property   *message);
size_t galaxy__common__protocols__peer_common__property__pack
                     (const Galaxy__Common__Protocols__PeerCommon__Property   *message,
                      uint8_t             *out);
size_t galaxy__common__protocols__peer_common__property__pack_to_buffer
                     (const Galaxy__Common__Protocols__PeerCommon__Property   *message,
                      ProtobufCBuffer     *buffer);
Galaxy__Common__Protocols__PeerCommon__Property *
       galaxy__common__protocols__peer_common__property__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   galaxy__common__protocols__peer_common__property__free_unpacked
                     (Galaxy__Common__Protocols__PeerCommon__Property *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Galaxy__Common__Protocols__PeerCommon__Property_Closure)
                 (const Galaxy__Common__Protocols__PeerCommon__Property *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor galaxy__common__protocols__peer_common__property__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_galaxy_2ecommon_2eprotocols_2epeer_5fcommon_2eproto__INCLUDED */
