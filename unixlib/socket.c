#include "socket.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int socket_create(int* const out_socket) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        return errno ? errno : -1;
    }
    *out_socket = sock;
    return 0;
}

void socket_close(int socket) {
    close(socket);
}


void unix_socket_init(SocketFunctions *const out_functions) {
    out_functions->create = socket_create;
    out_functions->close = socket_close;
}
