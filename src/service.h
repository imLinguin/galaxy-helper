#pragma once
#ifndef SERVICE_H
#define SERVICE_H

#include <windows.h>

WINBOOL load_functions_once(void);
WINBOOL init_pipes(DWORD pid, HANDLE* win_pipe, int* unix_pipe);

#endif
