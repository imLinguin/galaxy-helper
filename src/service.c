#include "service.h"
#include <stdio.h>
#include <windows.h>
#include "unixlib/socket.h"

static HMODULE unixlib = NULL;
static SocketFunctions socket_functions;

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

WINBOOL init_pipes(DWORD pid, HANDLE* win_pipe, int* unix_pipe) {
    char pipeName[256];
    sprintf(pipeName, "\\\\.\\pipe\\Galaxy-%ld-CommunicationService-Overlay", pid);
    *win_pipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES, 1024, 1024, 0, NULL);

    return TRUE;
}
