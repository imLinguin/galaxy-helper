#include "galaxy.h"
#include "cjson/cJSON.h"

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "galaxy.protocols.communication_service.pb-c.h"
#include "gog.protocols.pb.pb-c.h"
#include "protobuf-c/protobuf-c.h"
#include "protocols.h"

void free_game_details(GameDetails* game_details) {
    if (game_details->exe_names) {
        int cursor = 0;
        while(game_details->exe_names[cursor] != 0) {
            free(game_details->exe_names[cursor]);
            cursor++;
        }
        free(game_details->exe_names);
    }
    free(game_details->game_id);
    free(game_details->title);
    game_details->exe_names = NULL;
    game_details->game_id = NULL;
    game_details->title = NULL;
}

int find_game_details(GameDetails* game_details) {
    char search[1024];
    char* install_location = NULL;
    int total_names = 2, cursor = 0;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    install_location = getenv("STEAM_COMPAT_INSTALL_PATH");
    if (!install_location) return 1;
    
    strcpy(search, install_location);
    strcat(search, "\\goggame-*.info");
    // Find goggame files to determine .exe name we want to look for in child processes
    hFind = FindFirstFileA(search, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 1;
    }
    do {
        char file_path[1024];
        char* file_data = NULL;
        char* current_game_id = NULL;
        FILE *file = NULL;
        cJSON *json = NULL;
        const cJSON *currGID = NULL;
        const cJSON *tasks = NULL;
        const cJSON *task = NULL;

        strcpy(file_path, install_location);
        strcat(file_path, "\\");
        strcat(file_path, FindFileData.cFileName);

        // If file is bigger than 1MiB, skip it
        if (FindFileData.nFileSizeHigh != 0 || FindFileData.nFileSizeLow >= 1024 * 1024) {
            goto cleanup_file;
        }

        file = fopen(file_path, "r");
        if (!file) {
            fprintf(stderr, "Failed to open %s file\n", FindFileData.cFileName);
            goto cleanup_file;
        }
        // Read and parse json file
        file_data = malloc(FindFileData.nFileSizeLow * sizeof(char));
        if (!file_data) {
            goto cleanup_file;
        }
        memset(file_data, 0, FindFileData.nFileSizeLow * sizeof(char));

        fread(file_data, sizeof(char), FindFileData.nFileSizeLow, file);
        json = cJSON_Parse(file_data);
        if (!json) {
            goto cleanup_file;
        }

        currGID = cJSON_GetObjectItemCaseSensitive(json, "gameId");
        current_game_id = cJSON_GetStringValue(currGID);

        if (!game_details->game_id) {
            cJSON *rootGameId = cJSON_GetObjectItemCaseSensitive(json, "rootGameId");
            game_details->game_id = cJSON_GetStringValue(rootGameId);
            if (game_details->game_id) 
                game_details->game_id = strdup(game_details->game_id);
        }

        if (!game_details->title && game_details->game_id && current_game_id && strcmp(game_details->game_id, current_game_id) == 0) {
            cJSON *gameName = cJSON_GetObjectItemCaseSensitive(json, "name");
            game_details->title = cJSON_GetStringValue(gameName);
            if (game_details->title) 
                game_details->title = strdup(game_details->title);
        }

        // Find game playTasks and get exe names from them
        tasks = cJSON_GetObjectItemCaseSensitive(json, "playTasks");
        cJSON_ArrayForEach(task, tasks) {
            cJSON* category = cJSON_GetObjectItemCaseSensitive(task, "category");
            cJSON* path = cJSON_GetObjectItemCaseSensitive(task, "path");
            WCHAR* new_exe_name = NULL;

            if (category && path && (strcmp(cJSON_GetStringValue(category), "game") == 0)) {
                int is_exe = 0;
                static const char exeStr[] = ".exe";
                char *pathStr = cJSON_GetStringValue(path);
                if (!pathStr) {
                    goto cleanup_file;
                }

                unsigned long long pathLen = strlen(pathStr), new_length = 0;
                int index = 0;

                for (int i = 0; i < 4; i++) {
                    if (pathStr[i + pathLen - 4] != exeStr[i]) {
                        is_exe = 0;
                        break;
                    }
                    is_exe = 1;
                }

                if (!is_exe) goto cleanup_file;

                // Get last part of the path - exe name
                for (unsigned long long i = pathLen - 1; i > 0; i--) {
                    if (pathStr[i] == '\\' || pathStr[i] == '/') {
                        index = i + 1;
                        break;
                    }
                }

                new_length = (pathLen - index);
                new_exe_name = malloc(sizeof(WCHAR) * (1 + new_length));
                if (!new_exe_name) {
                    goto cleanup_file;
                }
                mbstowcs(new_exe_name, pathStr + index, new_length);
                new_exe_name[new_length] = 0;
                WCHAR** tmp = realloc(game_details->exe_names, (total_names++) * sizeof(WCHAR*));
                if (!tmp) {
                    fclose(file);
                    free(file_data);
                    cJSON_Delete(json);
                    free_game_details(game_details);
                    FindClose(hFind);
                    return 1;
                }
                game_details->exe_names = tmp;
                game_details->exe_names[cursor++] = new_exe_name;
            }
        }

cleanup_file:
        fclose(file);
        free(file_data);
        cJSON_Delete(json);

    } while(FindNextFileA(hFind, &FindFileData) != 0);
    FindClose(hFind);
    if (game_details->exe_names) {
        (game_details->exe_names)[total_names-2] = NULL;
    }

    return 0;
}

WCHAR* get_comet_redist(void) {
    return _wgetenv(L"COMET_REDIST_PATH");
}

WINBOOL notify_comet(DWORD pid) {
    int iResult;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    unsigned char* buffer = NULL;
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;

    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        return FALSE;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo("127.0.0.1", "9977", &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        return FALSE;
    }

    for (ptr = result; ptr != NULL; ptr=ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            WSACleanup();
            freeaddrinfo(result);
            return FALSE;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        WSACleanup();
        return FALSE;
    }

    Gog__Protocols__Pb__Header header;
    Galaxy__Protocols__CommunicationService__StartGameSessionRequest start_request;
    galaxy__protocols__communication_service__start_game_session_request__init(&start_request);
    gog__protocols__pb__header__init(&header);

    start_request.game_pid = (uint32_t)pid;
    start_request.has_game_pid = 1;
    start_request.overlay_support = GALAXY__PROTOCOLS__COMMUNICATION_SERVICE__START_GAME_SESSION_REQUEST__OVERLAY_SUPPORT__OVERLAY_SUPPORT_ENABLED;
    start_request.has_overlay_support = 1;

    header.size = galaxy__protocols__communication_service__start_game_session_request__get_packed_size(&start_request);
    header.has_size = 1;
    header.oseq = 1;
    header.has_oseq = 1;
    header.type = GALAXY__PROTOCOLS__COMMUNICATION_SERVICE__MESSAGE_TYPE__START_GAME_SESSION_REQUEST;
    header.sort = GALAXY__PROTOCOLS__COMMUNICATION_SERVICE__MESSAGE_SORT__MESSAGE_SORT;

    unsigned short header_size = gog__protocols__pb__header__get_packed_size(&header);
    unsigned short payload_size = header_size + header.size;
    int total_size = sizeof(short) + (sizeof(char) * payload_size);
    buffer = malloc(total_size);
    if (!buffer) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        WSACleanup();
    }

    short r_header_size = _byteswap_ushort(header_size);
    memcpy(buffer, &r_header_size, sizeof(header_size));
    gog__protocols__pb__header__pack(&header, buffer+sizeof(header_size));
    galaxy__protocols__communication_service__start_game_session_request__pack(&start_request, buffer + sizeof(header_size) + header_size);

    iResult = send(ConnectSocket, (char*)buffer, total_size, 0);
    if (iResult == SOCKET_ERROR) {
        printf("Failed to send data to socket");
        free(buffer);
        closesocket(ConnectSocket);
        WSACleanup();
        return FALSE;
    }

    closesocket(ConnectSocket);
    free(buffer);
    WSACleanup();
    return TRUE;
}

