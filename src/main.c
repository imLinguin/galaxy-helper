#define _GALAXY_LIB_NO_FUN_DEFS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <tlhelp32.h>

#include "overlay.h"
#include "wine.h"
#include "galaxy.h"
#include "service.h"
#include "cjson/cJSON.h"

int wmain(int argc, WCHAR** argv) {
    int retval = 0;
    if (argc == 1) {
        eprintf("Need an exe to run!\n");
        return -1;
    }
    int len = 1;
    WCHAR* args;
    HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
    HANDLE process = INVALID_HANDLE_VALUE;
    HANDLE win_pipe = INVALID_HANDLE_VALUE;
    int unix_pipe = 0;
    GameDetails details = { 0 };
    OverlayInfo overlay = { 0 };

    if (!load_functions_once()) {
        eprintf("[galaxy_helper] Failed to load unix functions for sockets\n");
    }

    // Obtain information about the game
    if (!find_game_details(&details)) {
        int index = 0;
        eprintf("[galaxy_helper] Detected game \"%s\" (%s)\n", details.title, details.game_id);
        if (details.exe_names) {
            WCHAR* exe;
            eprintf("[galaxy_helper] Handler will look for: ");
            while((exe = details.exe_names[index++])) {
                eprintf("\"%ls\" ", exe);
            }
            eprintf("\n");
        }
    } else {
        eprintf("[galaxy_helper] Failed to determine game information. The overlay will not get injected\n");
    }

    // START AND TRACK THE PROCESS

    for (int i = 2; i < argc; i++) {
        len += wcslen(argv[i]) + 1;
    }
    
    args = calloc(len, sizeof(*args));
    
    if (!args) {
        retval = -1;
        goto end;
    }

    for (int i = 2; i < argc; i++) {
        wcscat(args, L" ");
        wcscat(args, argv[i]);
    }

    eprintf("[galaxy_helper] Spawning %ls %ls\n", argv[1], args);
    ShellExecuteW(NULL, NULL, convert_to_win32(argv[1]), args, NULL, SW_SHOWNORMAL);
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        retval = -1;
        goto end;
    } 

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    WCHAR* exe;

    if (Process32FirstW(hProcessSnap, &pe32)) {
        do {
            Sleep(100);
            int index = 0; 
            while((exe = details.exe_names[index++])) {
                if (wcsstr(exe, pe32.szExeFile)) {
                    eprintf("[galaxy_helper] Found target executable %ls\n", exe);
                    process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, pe32.th32ProcessID);
                    break;
                }
            }
            if (process != INVALID_HANDLE_VALUE) break;
        } while (Process32NextW(hProcessSnap, &pe32) || process == INVALID_HANDLE_VALUE);
    }

    if (process == INVALID_HANDLE_VALUE) {
        retval = -1;
        goto end;
    }

    overlay = overlay_get_info(pe32.th32ProcessID, &details);
    WINBOOL pipes = FALSE;
    if (!overlay.parameters || !overlay.executable || !overlay.cwd) {
        eprintf("[galaxy_helper] failed to get overlay info");
    }
    else {
        if (!notify_comet(pe32.th32ProcessID)) {
            eprintf("Failed to notify comet about game session\n");
        }
        ShellExecuteW(NULL, NULL, overlay.executable, overlay.parameters, overlay.cwd, SW_NORMAL);
        pipes = init_pipes(pe32.th32ProcessID, &win_pipe, &unix_pipe);
    }
    if (!pipes) eprintf("An error with pipes\n");
    eprintf("[galaxy_helper] Waiting for app exit\n");
    while (WaitForSingleObject(process, 100) == WAIT_TIMEOUT) {
        if (pipes) forward_messages(&win_pipe, &unix_pipe);
        else Sleep(1000);
    }
    eprintf("[galaxy_helper] Finished waiting for game\n");
end:
    CloseHandle(process);
    CloseHandle(hProcessSnap);
    CloseHandle(win_pipe);
    cleanup_pipe(unix_pipe);
    free_game_details(&details);
    free(args);
    free_overlay_details(&overlay);
    return retval;
}

