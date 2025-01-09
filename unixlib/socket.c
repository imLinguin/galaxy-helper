#include "socket.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

int __stdcall socket_create(int* const out_socket) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        return errno ? errno : -1;
    }
    *out_socket = sock;
    return 0;
}

void __stdcall socket_close(int socket) {
    close(socket);
}

char* __stdcall comet_redist(void) {
    char* start = getenv("XDG_DATA_HOME");
    char* new_str = calloc(260, sizeof(char));
    if (start) {
        memcpy(new_str, start, strlen(start));
        strcat(new_str, "/comet/redist");
    } else {
        start = getenv("HOME");
        if (!start) {
            free(new_str);
            return NULL;
        }
        memcpy(new_str, start, strlen(start));
        strcat(new_str, "/.local/share/comet/redist");
    }

    return new_str;
}

void __stdcall unix_socket_init(UnixlibFunctions *const out_functions) {
    out_functions->create = socket_create;
    out_functions->close = socket_close;
    out_functions->comet_redist = comet_redist;
}
