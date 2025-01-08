#pragma once
#ifndef OVERLAY_H
#define OVERLAY_H

#include "galaxy.h"
#include <windows.h>

typedef struct {
    WCHAR* executable;
    WCHAR* parameters;
    WCHAR* cwd;
} OverlayInfo;

OverlayInfo overlay_get_info(DWORD pid, GameDetails* game_details);
void free_overlay_details(OverlayInfo* overlay);

#endif
