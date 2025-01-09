#pragma once

#ifndef SOCKET_H
#define SOCKET_H

typedef struct UnixlibFunctions {
    int __stdcall (*create)(int* const out_socket);
    void __stdcall (*close)(int socket);
    char* __stdcall (*comet_redist)(void);
} UnixlibFunctions;


void __stdcall unix_socket_init(UnixlibFunctions* const out_functions);
int __stdcall socket_create(int* const socket);
void __stdcall socket_close(int socket);

typedef void __stdcall (*unix_socket_init_t)(UnixlibFunctions* out_functions);

#endif
