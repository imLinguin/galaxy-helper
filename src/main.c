#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "galaxy.h"
#include "service.h"
#include "cjson/cJSON.h"
#include "protocols.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        fprintf(stderr, "Need an exe to run!\n");
        return 0;
    }
    char** game_exe_names = NULL;
    char* gog_game_id = NULL;

    gog_game_id = getenv("GALAXY_GAME_ID");
    if (!gog_game_id) gog_game_id = getenv("HEROIC_APP_NAME");

    game_exe_names = find_game_exe_names();

    // START AND TRACK THE PROCESS
    

    exe_names_free(game_exe_names);
    return 0;
}

