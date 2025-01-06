#pragma once
#ifndef BRIDGE_GALAXY_H
#define BRIDGE_GALAXY_H

typedef struct {
    char* game_id;
    char* title;
    char** exe_names;
} GameDetails; 

int find_game_details(GameDetails *game_details);
void free_game_details(GameDetails *game_details);


#endif
