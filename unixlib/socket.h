// Heavily based on https://github.com/openglfreak/winestreamproxy/blob/master/src/proxy_unixlib/socket.h

#pragma once
#ifndef SOCKET_H
#define SOCKET_H
#include <stdlib.h>

typedef enum recv_status {
    RECV_STATUS_SUCCESS,
    RECV_STATUS_INSUFFICIENT_BUFFER,
    RECV_STATUS_DISCARDED_DATA
} recv_status;

typedef enum poll_status {
    POLL_STATUS_SUCCESS,
    POLL_STATUS_TIMEOUT,
    POLL_STATUS_CLOSED_CONNECTION,
    POLL_STATUS_INVALID_MESSAGE_FLAGS
} poll_status;

typedef struct UnixlibFunctions {
    int __stdcall (*create)(int* const out_socket);
    int __stdcall (*connect)(int const socket, void const* const address);
    void __stdcall (*close)(int socket);
    int __stdcall (*send)(int const fd, unsigned char* const buf, size_t const size, size_t* const written);
    int __stdcall (*poll)(int const socket, poll_status* const out_status);
    int __stdcall (*recv)(int const socket, unsigned char* const buffer, size_t const buffer_size, recv_status* const out_status, size_t* const out_message_length);
    size_t __stdcall (*size_of_sockaddr)(void);
    size_t __stdcall (*size_of_sockpath)(void);
    void __stdcall (*init_address)(void* const address, char* const path, size_t const path_len);
    char* __stdcall (*comet_redist)(void);
} UnixlibFunctions;


#ifndef _GALAXY_LIB_NO_FUN_DEFS
void __stdcall unix_socket_init(UnixlibFunctions* const out_functions);
int __stdcall socket_create(int* const socket);
int __stdcall socket_connect(int const socket, void const* const address);
void __stdcall socket_close(int socket);
size_t __stdcall size_of_sockaddr(void);
size_t __stdcall size_of_sockpath(void);
void __stdcall socket_init_address(void* const address, char* const path, size_t const path_len);
int __stdcall socket_send(int const fd, unsigned char* const buf, size_t const size, size_t* const written);
int __stdcall socket_poll(int const socket, poll_status* const out_status);
int __stdcall socket_recv(int const socket, unsigned char* const buffer, size_t const buffer_size, recv_status* const out_status, size_t* const out_message_length);
#endif

typedef void __stdcall (*unix_socket_init_t)(UnixlibFunctions* out_functions);

#endif
