#include "socket.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <poll.h>
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

size_t __stdcall size_of_sockaddr(void) {
    return sizeof(struct sockaddr_un);
}

size_t __stdcall size_of_sockpath(void) {
    return sizeof(((struct sockaddr_un*)0)->sun_path) - 1;
}

void __stdcall socket_init_address(void* const address, char* const path, size_t const path_len) {
    struct sockaddr_un* const addr = (struct sockaddr_un*)address;
    memset(addr, 0, sizeof(*addr));
    addr->sun_family = AF_UNIX;
    memcpy(addr->sun_path, path, path_len);
}

int __stdcall socket_send(int const fd, unsigned char* const buf, size_t const size, size_t* const written) {
    ssize_t res = write(fd, buf, size);
    if (res == -1) {
        return errno ? errno : -1;
    }
    *written = (size_t)res;
    return 0;
}

int __stdcall socket_poll(int const socket, poll_status* const out_status) {
    struct pollfd fd;
    int nfds;

    fd.fd = socket;
    fd.events = POLLIN | POLLPRI | POLLHUP;
    fd.revents = 0;
    while (((nfds = poll(&fd, 1, 500)) == -1 && (errno == EAGAIN || errno == EINTR)));
    
    if (nfds == 0) {
        *out_status = POLL_STATUS_TIMEOUT;
        return 0;
    }

    if (nfds == -1)
        return errno ? errno : -1;

    if (fd.revents)
    {
        if (fd.revents & POLLHUP)
            *out_status = POLL_STATUS_CLOSED_CONNECTION;
        if (!(fd.revents & (POLLIN | POLLPRI)))
            *out_status = POLL_STATUS_INVALID_MESSAGE_FLAGS;
    }
    else
        *out_status = POLL_STATUS_SUCCESS;

    return 0;
}

int __stdcall socket_recv(int const socket, unsigned char* const buffer, size_t const buffer_size,
                            recv_status* const out_status, size_t* const out_message_length) {
    ssize_t recv_ret = recv(socket, buffer, buffer_size, MSG_PEEK);
    if (recv_ret == -1)
        return errno ? errno : -1;

    if ((size_t)recv_ret >= buffer_size)
    {
        *out_status = RECV_STATUS_INSUFFICIENT_BUFFER;
        *out_message_length = (size_t)recv_ret;
        return 0;
    }

    recv_ret = recv(socket, buffer, buffer_size, 0);
    if (recv_ret == -1)
        return errno ? errno : -1;

    *out_status = (size_t)recv_ret >= buffer_size ? RECV_STATUS_DISCARDED_DATA : RECV_STATUS_SUCCESS;
    *out_message_length = (size_t)recv_ret;
    return 0;
}

int __stdcall socket_connect(int const socket, void const* const address) {
    if (connect(socket, (struct sockaddr const*)address, sizeof(struct sockaddr_un)) != 0)
        return errno ? errno : -1;
    return 0;
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
    out_functions->connect = socket_connect;
    out_functions->recv = socket_recv;
    out_functions->send = socket_send;
    out_functions->poll = socket_poll;
    out_functions->size_of_sockaddr = size_of_sockaddr;
    out_functions->size_of_sockpath = size_of_sockpath;
    out_functions->init_address = socket_init_address;
    out_functions->comet_redist = comet_redist;
}
