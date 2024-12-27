# galaxy-helper

A utility allowing for comet interaction with windows version of overlay on Linux


## Building

This is meant for Wine/Proton environments, thus unix-like system is required.

You will need
- mingw32
- winegcc (usually provided by wine package)

```
meson setup --cross-file meson/x86_64-w64-mingw32.txt --native-file winegcc.txt build
```

```
meson compile -C build
```
