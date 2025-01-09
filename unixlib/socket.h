#pragma once

#ifndef SOCKET_H
#define SOCKET_H

typedef struct SocketFunctions {
    int (*create)(int* const out_socket);
    void (*close)(int socket);
} SocketFunctions;


void unix_socket_init(SocketFunctions* const out_functions);
int socket_create(int* const socket);
void socket_close(int socket);

typedef void (*unix_socket_init_t)(SocketFunctions* out_functions);

#endif
