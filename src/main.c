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

int wmain(int argc, WCHAR** argv) {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    int retval = 0;
    if (argc == 1) {
        eprintf("Need an exe to run!\n");
        return -1;
    }
    int len = 1;
    WCHAR* args = NULL;
    WCHAR* win_game_exe = NULL;
    HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
    HANDLE process = INVALID_HANDLE_VALUE;
    HANDLE launcher_process = INVALID_HANDLE_VALUE;
    HANDLE win_pipe = INVALID_HANDLE_VALUE;
    int unix_pipe = 0;
    GameDetails details = { 0 };
    OverlayInfo overlay = { 0 };
    STARTUPINFOW si;
    PROCESS_INFORMATION launch_process;
    SHFILEINFOW sfi;
    DWORD flags = CREATE_UNICODE_ENVIRONMENT;
    DWORD console;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&launch_process, sizeof(launch_process));
    launch_process.hThread = INVALID_HANDLE_VALUE;
    launch_process.hProcess = INVALID_HANDLE_VALUE;

    // Make sure logs directory is created for overlay to succeed
    CreateDirectory("C:\\ProgramData\\GOG.com", NULL);
    CreateDirectory("C:\\ProgramData\\GOG.com\\Galaxy", NULL);
    CreateDirectory("C:\\ProgramData\\GOG.com\\Galaxy\\logs", NULL);

    if (!load_functions_once()) {
        eprintf("[galaxy_helper] Failed to load unix functions for sockets\n");
        return -1;
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
    WCHAR exe_pattern[] = L".exe"; 
    BOOL is_exe = TRUE;
    unsigned long long scan_cursor = wcslen(argv[1]) - 1;
    for (int i = 3; i >= 0; i--) {
        if (argv[1][scan_cursor] == '"') scan_cursor--;
        if (argv[1][scan_cursor] != exe_pattern[i]) {
            is_exe = FALSE;
            break;
        }
        scan_cursor--;
    }

    win_game_exe = convert_to_win32(argv[1]);
    len += wcslen(win_game_exe) + 2;
    for (int i = 2; i < argc; i++) {
        len += wcslen(argv[i]) + 3;
    }
    
    args = calloc(len, sizeof(*args));
    
    if (!args) {
        retval = -1;
        goto end;
    }
    wcscat(args, L"\"");
    if (!wcsstr(argv[1], L"://"))
        wcscat(args, win_game_exe);
    else 
        wcscat(args, argv[1]);
    wcscat(args, L"\"");
    WCHAR* parameters_start = args + wcslen(args);

    for (int i = 2; i < argc; i++) {
        wcscat(args, L" ");
        wcscat(args, L"\"");
        wcscat(args, argv[i]);
        wcscat(args, L"\"");
    }

    CoInitialize(NULL);

    if (is_exe) {
        console = SHGetFileInfoW(win_game_exe, 0, &sfi, sizeof(sfi), SHGFI_EXETYPE);
        if (console && !HIWORD(console))
            flags |= CREATE_NEW_CONSOLE;

        eprintf("[galaxy_helper] Spawning %ls\n", args);
        if (!CreateProcessW(NULL, args, NULL, NULL, FALSE, flags, NULL, NULL, &si, &launch_process)) {
            eprintf("Failed to start game process %lu\n", GetLastError());
            retval = -1;
            goto end;
        }
        CloseHandle(launch_process.hThread);
        launcher_process = launch_process.hProcess;
        SetProcessInformation(GetCurrentProcess(), (PROCESS_INFORMATION_CLASS)1000, &launch_process.hProcess, sizeof(HANDLE*));
    } else {
        SHELLEXECUTEINFOW exeInfo;
        ZeroMemory(&exeInfo, sizeof(exeInfo));
        exeInfo.cbSize = sizeof(exeInfo);
        exeInfo.lpVerb = L"open";
        exeInfo.nShow = SW_HIDE;
        exeInfo.lpFile = args;
        exeInfo.lpParameters = parameters_start + 1;
        (*parameters_start) = '\0';

        eprintf("[galaxy_helper] Spawning %ls %ls\n", args, parameters_start + 1);
        ShellExecuteExW(&exeInfo);
        launcher_process = exeInfo.hProcess;
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    while (details.game_id && process == INVALID_HANDLE_VALUE) {
        Sleep(1000);
        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        WCHAR* exe;
        DWORD launcher_running = STILL_ACTIVE;

        if (hProcessSnap == INVALID_HANDLE_VALUE) {
            retval = -1;
            goto end;
        } 

        if (Process32FirstW(hProcessSnap, &pe32)) {
            do {
                int index = 0; 
                while((exe = details.exe_names[index++])) {
                    if (wcsstr(exe, pe32.szExeFile)) {
                        eprintf("[galaxy_helper] Found target executable %ls\n", exe);
                        process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, pe32.th32ProcessID);
                        break;
                    }
                }
                if (process != INVALID_HANDLE_VALUE) break;
            } while (Process32NextW(hProcessSnap, &pe32));
        }
        CloseHandle(hProcessSnap);
        if (launcher_process != INVALID_HANDLE_VALUE) {
            GetExitCodeProcess(launcher_process, &launcher_running);
            eprintf("The launcher is %lu\n", launcher_running);
            if (process == INVALID_HANDLE_VALUE && launcher_running != STILL_ACTIVE) {
                goto end;
            }
        }
    }

    if (process == INVALID_HANDLE_VALUE) {
        retval = -1;
        goto end;
    }

    overlay = overlay_get_info(pe32.th32ProcessID, &details);
    WINBOOL pipes = FALSE;
    if (!details.game_id || !overlay.parameters || !overlay.executable || !overlay.cwd) {
        eprintf("[galaxy_helper] failed to get overlay info\n");
    }
    else {
        if (!notify_comet(pe32.th32ProcessID)) {
            eprintf("Failed to notify comet about game session\n");
        } else {
            ShellExecuteW(NULL, NULL, overlay.executable, overlay.parameters, overlay.cwd, SW_NORMAL);
            pipes = init_pipes(pe32.th32ProcessID, &win_pipe, &unix_pipe);
        }
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
    free(win_game_exe);
    free_overlay_details(&overlay);
    return retval;
}

