#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "galaxy.h"
#include "service.h"
#include "cjson/cJSON.h"
#include "protocols.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__);

int main(int argc, char** argv) {
    if (argc == 1) {
        eprintf("Need an exe to run!\n");
        return -1;
    }
    GameDetails details = { 0 };

    // Obtain information about the game
    if (!find_game_details(&details)) {
        int index = 0;
        eprintf("[galaxy_helper] Detected game \"%s\" (%s)\n", details.title, details.game_id);
        if (details.exe_names) {
            char *exe;
            eprintf("[galaxy_helper] Handler will look for: ");
            while((exe = details.exe_names[index++])) {
                eprintf("\"%s\" ", exe);
            }
            eprintf("\n");
        }
    } else {
        eprintf("[galaxy_helper] Failed to determine game information. The overlay will not get injected\n");
    }

    // START AND TRACK THE PROCESS
    

    free_game_details(&details);
    return 0;
}

