#include "galaxy.h"
#include "cjson/cJSON.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exe_names_free(char **names) {
    if (!names) return;
    int cursor = 0;
    while(names[cursor] != 0) {
        free(names[cursor]);
        cursor++;
    }
    free(names);
}

char ** find_game_exe_names(void) {
    char search[1024];
    char* install_location = NULL;
    char** game_exe_names = NULL; // Null terminated buffer for exe names
    int total_names = 2, cursor = 0;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    install_location = getenv("STEAM_COMPAT_INSTALL_PATH");
    if (install_location) {
        strcpy(search, install_location);
        strcat(search, "\\goggame-*.info");
        // Find goggame files to determine .exe name we want to look for in child processes
        hFind = FindFirstFileA(search, &FindFileData);
        if (!hFind) {
            return NULL;
        }
        do {
            char file_path[1024];
            char* file_data = NULL;
            FILE *file = NULL;
            cJSON *json = NULL;
            const cJSON *tasks = NULL;
            const cJSON *task = NULL;

            strcpy(file_path, install_location);
            strcat(file_path, "\\");
            strcat(file_path, FindFileData.cFileName);

            // If file is bigger than 1MiB, skip it
            if (FindFileData.nFileSizeLow >= 1024 * 1024) {
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

            // Find game playTasks and get exe names from them
            tasks = cJSON_GetObjectItemCaseSensitive(json, "playTasks");
            cJSON_ArrayForEach(task, tasks) {
                cJSON *category = cJSON_GetObjectItemCaseSensitive(task, "category");
                cJSON *path = cJSON_GetObjectItemCaseSensitive(task, "path");
                char *new_exe_name = NULL;

                if (category && path && (strcmp(cJSON_GetStringValue(category), "game") == 0)) {
                    char *pathStr = cJSON_GetStringValue(path);
                    if (!pathStr) {
                        goto cleanup_file;
                    }
                    unsigned long long pathLen = strlen(pathStr), new_length = 0;
                    int index = 0;

                    // Get last part of the path - exe name
                    for (unsigned long long i = pathLen - 1; i > 0; i--) {
                        if (pathStr[i] == '\\' || pathStr[i] == '/') {
                            index = i + 1;
                            break;
                        }
                    }

                    new_length = (pathLen - index);
                    new_exe_name = malloc(sizeof(char) * (1 + new_length));
                    if (!new_exe_name) {
                        goto cleanup_file;
                    }
                    memcpy(new_exe_name, pathStr + index, new_length);
                    new_exe_name[new_length] = 0;
                    char** tmp = realloc(game_exe_names, (total_names++) * sizeof(char*));
                    if (!tmp) {
                        fclose(file);
                        free(file_data);
                        cJSON_Delete(json);
                        exe_names_free(game_exe_names);
                        game_exe_names = NULL;
                        break;
                    }
                    game_exe_names = tmp;
                    game_exe_names[cursor++] = new_exe_name;
                }
            }

cleanup_file:
            fclose(file);
            free(file_data);
            cJSON_Delete(json);

        } while(FindNextFileA(hFind, &FindFileData) != 0);
        FindClose(hFind);
        if (game_exe_names) {
            game_exe_names[total_names-1] = 0;
        }
    }

    return game_exe_names;
}

