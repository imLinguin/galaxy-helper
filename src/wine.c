#include <windows.h>
#include <wine.h>

#define CP_UNIXCP     65010 /* Wine extension */

WCHAR* convert_to_win32(const WCHAR *unixW) {
    typedef WCHAR* (CDECL *wine_get_dos_file_name_PFN)(const char* path);

    HMODULE k32 = LoadLibraryW(L"kernel32");
    wine_get_dos_file_name_PFN pwine_get_dos_file_name =
        (wine_get_dos_file_name_PFN)GetProcAddress(k32, "wine_get_dos_file_name");

    if (!pwine_get_dos_file_name) return NULL;

    int r = WideCharToMultiByte(CP_UNIXCP, 0, unixW, -1, NULL, 0, NULL, NULL);

    if (!r) return NULL;

    char *unixA = malloc(r);

    if (!unixA) return NULL;

    r = WideCharToMultiByte(CP_UNIXCP, 0, unixW, -1, unixA, r, NULL, NULL);

    if (!r) return NULL;

    WCHAR *ret = pwine_get_dos_file_name(unixA);
    free(unixA);

    return ret;
}
