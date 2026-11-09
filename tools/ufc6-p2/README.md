# UFC 6: a second local player

`bin\UFC 6.exe` (KytyPS5 `3309ab2`) predates KytyPS5's multi-controller support (#1583, `415387f`):
it has one pad and one user, so every controller drives Player 1. Its source is not available, so
this adds Player 2 from outside. The executable on disk is not changed.

`sh build.sh` needs `x86_64-w64-mingw32-gcc` and gives `version.dll`, which goes next to
`UFC 6.exe`. Deleting the DLL removes the feature.

## How it works

- The engine imports `VERSION.dll`, so a copy in its folder loads first. It forwards the real API
  to the system's DLL.
- At load it redirects 34 libScePad and libSceUserService registrations (the `lea r8, fn` before
  each symbol is added) to stubs in the slack after `.text`. Player 1's calls go on to the engine's
  functions unchanged.
- Player 2 is user 1256 with pad handle 2, fed by the first XInput pad on a host thread.
- That pad is hidden from the engine with `SDL_GAMECONTROLLER_IGNORE_DEVICES`.
- The DLL checks the executable's size and every site. On any other build it patches nothing and
  the game runs as before.

`_Logs\p2-addon.log` records the pad, the login and the first call of each hooked function.

## Tested

One PC, offline 1v1: PS4 pad as Player 1, wired Xbox 360-compatible pad as Player 2.

## Limits

- Only pads with the Xbox 360 ID `045E:028E` are hidden from the engine (`IGNORE_PADS` in `p2.c`).
  Other XInput pads are not, so they also reach Player 1.
- Player 2 needs an XInput pad, and Player 1 a pad the engine still sees. Two PS pads do not work.
- Offline only. Nothing here turns it off when the game goes online.
- The addresses in `hooks.inc` are for this one build, and a new engine needs them found again.
- No save or trophy handling for user 1256. Merging #1583 into the engine would replace all of this.
