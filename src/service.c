#include "galaxy.h"
#include "service.h"
#include <errhandlingapi.h>
#include <namedpipeapi.h>
#include <stdio.h>
#include <windows.h>
#include "unixlib/socket.h"

static HMODULE unixlib = NULL;
static UnixlibFunctions socket_functions;

WINBOOL load_functions_once(void) {
    if (unixlib) return FALSE;
    unixlib = LoadLibrary("libgalaxyunixlib.dll.so");
    if (!unixlib) {
        return FALSE;
    }

    unix_socket_init_t p_socket_init;
    p_socket_init = (unix_socket_init_t)(ULONG_PTR)GetProcAddress(unixlib, "unix_socket_init");
    if (!p_socket_init) {
        printf("Failed to init %ld \n", GetLastError());
        FreeLibrary(unixlib);
        return FALSE;
    }
    p_socket_init(&socket_functions);

    return TRUE;
}

void cleanup_pipe(int unix_pipe) {
    socket_functions.close(unix_pipe);
    return;
}

WINBOOL init_pipes(DWORD pid, HANDLE* win_pipe, int* unix_pipe) {
    char pipeName[MAX_PATH];
    char* unixPipe = malloc(socket_functions.size_of_sockpath());
    void* address = malloc(socket_functions.size_of_sockaddr());

    if (!unixPipe || !address) {
        free(unixPipe);
        free(address);
        return FALSE;
    }
    sprintf(pipeName, "\\\\.\\pipe\\Galaxy-%ld-CommunicationService-Overlay", pid);
    sprintf(unixPipe, "/tmp/Galaxy-%ld-CommunicationService-Overlay", pid);
    *win_pipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES, 1024, 1024, 0, NULL);
    if (*win_pipe == INVALID_HANDLE_VALUE) {
        free(unixPipe);
        free(address);
        return FALSE;
    }

    socket_functions.init_address(address, unixPipe, strlen(unixPipe));
    socket_functions.create(unix_pipe);
    int ret = 0;
    int res = 0;
    for (ret = 0; ret < 10 && (res = socket_functions.connect(*unix_pipe, address)) != 0; ret++) {
        eprintf("Connection failed err - %d\n", res);
        Sleep(1000);
    }
    if (ret == 10) {
        eprintf("Failed to connect - out of retries\n");
        CloseHandle(*win_pipe);
        socket_functions.close(*unix_pipe);
        free(unixPipe);
        free(address);
        *win_pipe = INVALID_HANDLE_VALUE;
        *unix_pipe = 0;
        return FALSE;
    }
    return TRUE;
}

void forward_messages(HANDLE *win_pipe, int* unix_pipe) {
    static unsigned char buffer[64*1024];
    size_t bytesWritten = 0;
    size_t bytesRead_u = 0;
    DWORD bytesRead = 0;
    DWORD bytesAvail = 0;
    poll_status status = 0;
    recv_status r_status = 0;

    if (PeekNamedPipe(*win_pipe, NULL, 0, 0, &bytesAvail, 0) && bytesAvail > 0) {
        if (ReadFile(*win_pipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            eprintf("[galaxy_helper] win->unix read %ld from pipe\n", bytesRead);
            socket_functions.send(*unix_pipe, buffer, (size_t)bytesRead, &bytesWritten);
            eprintf("[galaxy_helper] win->unix wrote %llu\n", bytesWritten);
        }
        else if (GetLastError() == ERROR_PIPE_LISTENING) {
            eprintf("[galaxy_helper] Waiting for pipe to open on client side\n");
        }
        else {
            eprintf("[galaxy_helper] failed to read from win pipe %ld\n", GetLastError());
        }
    }

    if (socket_functions.poll(*unix_pipe, &status) == 0) {
        if (status == POLL_STATUS_SUCCESS && socket_functions.recv(*unix_pipe, buffer, sizeof(buffer), &r_status, &bytesRead_u) == 0) {
            eprintf("[galaxy_helper] unix->win read %llu\n", bytesRead_u);
            if (bytesRead_u && !WriteFile(*win_pipe, buffer, (DWORD)bytesRead_u, 0, NULL)) {
                eprintf("[galaxy_helper] unix->win pipe write failure %ld\n", GetLastError());
            }
        }
    }
}

WCHAR* get_comet_redist(void) {
    if(socket_functions.comet_redist) {
        char* redist = socket_functions.comet_redist();
        if (!redist) return NULL;
        WCHAR* new_str = calloc(strlen(redist)+1, sizeof(WCHAR));
        if (!new_str) return NULL;
        mbstowcs(new_str, redist, strlen(redist));

        return new_str;
    }
    return NULL;
}
