#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <overlay.h>
#include <galaxy.h>
#include <wine.h>


void free_overlay_details(OverlayInfo *overlay) {
    free(overlay->executable);
    free(overlay->parameters);
    free(overlay->cwd);
}

OverlayInfo overlay_get_info(DWORD pid, GameDetails* game_details) {
    OverlayInfo overlay_details = { 0 };
    WCHAR* comet_redist = get_comet_redist();
    static WCHAR comet_overlay[] = L"/overlay/GalaxyOverlay.exe";
    static WCHAR comet_web[] = L"/web/overlay.html";

    WCHAR* comet_cwd_win = NULL;
    WCHAR* win_comet_webpath = NULL;
    WCHAR* parameters = NULL;

    WCHAR* comet_webpath = NULL;
    WCHAR* executable = NULL;
    WCHAR comet_cwd[256];
    WCHAR game_id[32];

    if (!comet_redist) {
        return overlay_details;
    }

    printf("Getting overlay info\n");

    executable = calloc(wcslen(comet_redist) + sizeof(comet_overlay), sizeof(*executable)); 
    comet_webpath = calloc(wcslen(comet_redist) + sizeof(comet_web), sizeof(*comet_webpath)); 
    parameters = calloc(10 * 1024, sizeof(*parameters));
    if (!executable || !comet_webpath || !parameters) {
        free(comet_cwd_win);
        free(parameters);
        goto cleanup;
    }
    memcpy(executable, comet_redist, sizeof(*comet_redist) * wcslen(comet_redist));
    wcscat(executable, comet_overlay); 

    memcpy(comet_webpath, comet_redist, sizeof(*comet_redist) * wcslen(comet_redist));
    wcscat(comet_webpath, comet_web); 

    memcpy(comet_cwd, comet_redist, sizeof(*comet_redist) * wcslen(comet_redist));
    memcpy(comet_cwd+wcslen(comet_redist), comet_overlay, sizeof(*comet_redist) * 8);
    comet_cwd[wcslen(comet_redist) + 8] = 0;

    overlay_details.executable = convert_to_win32(executable);
    win_comet_webpath = convert_to_win32(comet_webpath);
    comet_cwd_win = convert_to_win32(comet_cwd);

    for (unsigned long long i = 0; i < wcslen(win_comet_webpath); i++) {
        if (win_comet_webpath[i] == L'\\') {
            win_comet_webpath[i] = L'/';
        }
    }
    mbstowcs(game_id, game_details->game_id, 31); 

    swprintf(parameters, (10 * 1024) - 1, L"--advanced-features --startoverlay --client-language-code=en-US --startup-url=file://%ls --attached-pid=%ld --product-id=%ls -cache-path=\"C:\\ProgramData\\GOG.com\\Galaxy\\webcache\\%ls-overlay\"", win_comet_webpath, pid, game_id, game_id);
    overlay_details.parameters = parameters;
    overlay_details.cwd = comet_cwd_win;

    wprintf(L"[OVERLAY] \"%ls\" %ls\n", overlay_details.executable, parameters);
    wprintf(L"[OVERLAY] cwd: %ls\n", comet_cwd_win);

cleanup:
    free(executable);
    free(comet_webpath);
    return overlay_details;
}
