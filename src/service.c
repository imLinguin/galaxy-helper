#include "service.h"
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

WINBOOL init_pipes(DWORD pid, HANDLE* win_pipe, int* unix_pipe) {
    char pipeName[MAX_PATH];
    sprintf(pipeName, "\\\\.\\pipe\\Galaxy-%ld-CommunicationService-Overlay", pid);
    *win_pipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES, 1024, 1024, 0, NULL);

    return TRUE;
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
