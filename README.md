# galaxy-helper

A utility allowing for comet interaction with windows version of overlay on Linux

## How does this work

1. This tool runs under Wine and is meant to be ran as a wrapper for game executable. 
2. `galaxy-helper` relies on `STEAM_COMPAT_INSTALL_PATH` env variable to get install location of the game, thanks to that it will be possible to detect game executable and inject overlay if its supported.
3. Running on "Windows" side is crucial in order to refer to PID exposed by WINE.
`galaxy-helper` will reach out to running comet instance and tell it that the game with particular PID has been found and if overlay has been injected.  
4. Appropriate pipe will be created in `/tmp/` and will be bridged to appropriate windows named pipe. This will allow overlay connection to comet.


In order to use the tool wrap your game executable with it.  
Make sure comet instance is running as well and that overlay files are downloaded there

### Download/update the overlay

```
comet --from-heroic --username <username> overlay --force
```

### Start comet


> [!IMPORTANT]
> Make sure your tokens are up-to date. You can ensure that by refreshing library for example 

```
comet --from-heroic --username <username>
```

### Run the game

> [!IMPORTANT]
> Galaxy overlay doesn't work when user is offline

```
GAMEID=0 STEAM_COMPAT_INSTALL_PATH=/game/install/location umu-run galaxy.exe game.exe
```

## What about Mac?

I wanted to give Mac a try with a slightly different approach - Injecting a native build of the overlay.  
The feature would require a separate bridge flow to provide Game <-> Overlay IPC. 

I do not own any Mac device so I can't do much about that.  
Contributions are welcome.


## Building

This is meant for Wine/Proton environments, thus unix-like system is required.

You will need
- mingw32
- winegcc (usually provided by wine package)
- protoc and protobuf-c installed - for protobuf to C generation

```
meson setup --cross-file meson/x86_64-w64-mingw32.txt --native-file meson/winegcc.txt build
```

```
meson compile -C build
```

## Atributions

The project is heavily inspired by https://github.com/openglfreak/winestreamproxy
