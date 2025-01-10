#pragma once
#ifndef BRIDGE_GALAXY_H
#define BRIDGE_GALAXY_H

#include <windows.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__);

typedef struct GameDetails {
    char* game_id;
    char* title;
    WCHAR** exe_names;
} GameDetails; 

int find_game_details(GameDetails *game_details);
void free_game_details(GameDetails *game_details);
WINBOOL notify_comet(DWORD pid);


#endif
