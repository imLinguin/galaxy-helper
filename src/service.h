#pragma once
#ifndef SERVICE_H
#define SERVICE_H

#include <windows.h>

WCHAR* get_comet_redist(void);
WINBOOL load_functions_once(void);
WINBOOL init_pipes(DWORD pid, HANDLE* win_pipe, int* unix_pipe);
void forward_messages(HANDLE *win_pipe, int* unix_pipe);
void cleanup_pipe(int unix_pipe);

#endif
