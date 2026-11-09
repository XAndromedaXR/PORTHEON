/* Player 2 for PORTHEON's UFC 6 engine (KytyPS5 3309ab2), which has a single pad and a single user.
 * A proxy version.dll, next to "UFC 6.exe". At load it redirects the engine's registration of the
 * libScePad and libSceUserService functions (the `lea r8, fn` before each AddSymbol call) to the
 * hooks below; the engine's code itself stays as it is and Player 1 goes straight through to it.
 *   Player 1: everything the engine reads.
 *   Player 2: a second user id and pad handle, fed by the first XInput pad. That pad is hidden from
 *   the engine (SDL_GAMECONTROLLER_IGNORE_DEVICES), which would otherwise treat it as Player 1's.
 * Any other engine build fails the site check: nothing is patched and the game runs as before.
 * The log is _Logs\p2-addon.log. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SV __attribute__((sysv_abi))
typedef SV int (*Fn)(intptr_t, intptr_t, intptr_t, intptr_t);

#define USER1 1000
#define USER2 1256              /* the engine's user + 0x100, as KytyPS5's own players */
#define HANDLE2 2
#define ERR_INVALID_ARG ((int)0x80920001)
#define ERR_ALREADY_OPENED ((int)0x80920004)
#define ERR_NO_HANDLE ((int)0x80920008)
#define IGNORE_PADS "0x045E/0x028E"
#define EXE_SIZE 28305920u
#define STUB_RVA 0x1488800u     /* zeroes after .text, inside its last page */
#define STUB_SIZE 16u

enum {
#define H(n, site, fn) I_##n,
#include "hooks.inc"
#undef H
    I_COUNT
};
static const char *const hook_names[I_COUNT] = {
#define H(n, site, fn) #n,
#include "hooks.inc"
#undef H
};
static uint8_t *image;
static volatile LONG calls[I_COUNT];
#define ORIG(n, a, b, c, d) ((Fn)(image + RVA_##n))((intptr_t)(a), (intptr_t)(b), (intptr_t)(c), (intptr_t)(d))
#define H(n, site, fn) enum { RVA_##n = fn };
#include "hooks.inc"
#undef H
#define COUNT(n) InterlockedIncrement(&calls[I_##n])

/* ---- the XInput pad, polled on a host thread: guest threads only copy the snapshot ---- */
typedef struct { WORD buttons; BYTE lt, rt; SHORT lx, ly, rx, ry; } XPad;
typedef struct { DWORD packet; XPad pad; } XState;
typedef struct { WORD left, right; } XVib;
static SRWLOCK pad_lock = SRWLOCK_INIT;
static XPad pad_now;
static volatile LONG pad_connected, pad_ever, login_sent, port_open;
static volatile LONG vib_large, vib_small;

static uint8_t stick(int v) {
    if (v > -4000 && v < 4000) return 128;
    return (uint8_t)((v + 32768) >> 8);
}
static void fill(uint8_t *d) {
    static LARGE_INTEGER freq;
    XPad p; LARGE_INTEGER t;
    AcquireSRWLockShared(&pad_lock); p = pad_now; ReleaseSRWLockShared(&pad_lock);
    if (!freq.QuadPart) QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t);
    static const struct { WORD x; uint32_t ps; } map[] = {
        {0x1000, 0x4000}, {0x2000, 0x2000}, {0x4000, 0x8000}, {0x8000, 0x1000},  /* A B X Y */
        {0x0100, 0x0400}, {0x0200, 0x0800}, {0x0040, 0x0002}, {0x0080, 0x0004},  /* LB RB L3 R3 */
        {0x0010, 0x0008}, {0x0020, 0x100000},                                    /* Start Back */
        {0x0001, 0x0010}, {0x0002, 0x0040}, {0x0004, 0x0080}, {0x0008, 0x0020},  /* d-pad */
    };
    uint32_t buttons = 0;
    for (size_t i = 0; i < sizeof map / sizeof *map; i++) if (p.buttons & map[i].x) buttons |= map[i].ps;
    if (p.lt > 30) buttons |= 0x100;
    if (p.rt > 30) buttons |= 0x200;
    memset(d, 0, 120);
    memcpy(d, &buttons, 4);
    d[4] = stick(p.lx); d[5] = (uint8_t)(255 - stick(p.ly));
    d[6] = stick(p.rx); d[7] = (uint8_t)(255 - stick(p.ry));
    if (d[5] == 127) d[5] = 128;
    if (d[7] == 127) d[7] = 128;
    d[8] = p.lt; d[9] = p.rt;
    const float w = 1.0f; memcpy(d + 24, &w, 4);
    d[76] = pad_connected != 0;
    const uint64_t us = (uint64_t)(t.QuadPart / freq.QuadPart * 1000000 + t.QuadPart % freq.QuadPart * 1000000 / freq.QuadPart);
    memcpy(d + 80, &us, 8);
    d[104] = 1;
}

/* ---- libSceUserService ---- */
static SV int h_UEvent(int *event) {
    COUNT(UEvent);
    int r = ORIG(UEvent, event, 0, 0, 0);
    if (r && pad_ever && !InterlockedExchange(&login_sent, 1)) {
        event[0] = 0; event[1] = USER2;   /* Login */
        return 0;
    }
    return r;
}
static SV int h_UList(int *list) {
    COUNT(UList);
    int r = ORIG(UList, list, 0, 0, 0);
    if (!r && pad_ever) list[1] = USER2;
    return r;
}
static SV int h_UName(int user, char *name, size_t size) {
    COUNT(UName);
    int r = ORIG(UName, user == USER2 ? USER1 : user, name, size, 0);
    if (!r && user == USER2 && size >= 9) memcpy(name, "Player 2", 9);
    return r;
}
static SV int num_common(int i, int rva, int user, int *number) {
    InterlockedIncrement(&calls[i]);
    int r = ((Fn)(image + rva))(user == USER2 ? USER1 : user, (intptr_t)number, 0, 0);
    if (!r && user == USER2) *number = 2;
    return r;
}
static SV int h_UNum1(int user, int *number) { return num_common(I_UNum1, RVA_UNum1, user, number); }
static SV int h_UNum2(int user, int *number) { return num_common(I_UNum2, RVA_UNum2, user, number); }
/* The rest answer the same for both users. */
#define AS_USER1(n) static SV int h_##n(int user, intptr_t a, intptr_t b) { COUNT(n); return ORIG(n, user == USER2 ? USER1 : user, a, b, 0); }
AS_USER1(UPresets) AS_USER1(UAge) AS_USER1(UChat) AS_USER1(UHold) AS_USER1(UVib) AS_USER1(UTrig) AS_USER1(UZoom) AS_USER1(UZoomF)

/* ---- libScePad ---- */
static int open2(int type, int index) {
    if (type || index) return ERR_INVALID_ARG;
    if (InterlockedExchange(&port_open, 1)) return ERR_ALREADY_OPENED;
    return HANDLE2;
}
static SV int h_POpen(int user, int type, int index, intptr_t param) {
    COUNT(POpen);
    return user == USER2 ? open2(type, index) : ORIG(POpen, user, type, index, param);
}
static SV int h_POpenExt(int user, int type, int index, intptr_t param) {
    COUNT(POpenExt);
    return user == USER2 ? open2(type, index) : ORIG(POpenExt, user, type, index, param);
}
static SV int h_PGetHandle(int user, int type, int index) {
    COUNT(PGetHandle);
    if (user != USER2) return ORIG(PGetHandle, user, type, index, 0);
    return port_open && !type && !index ? HANDLE2 : ERR_NO_HANDLE;
}
static SV int h_PClose(int handle) {
    COUNT(PClose);
    if (handle != HANDLE2) return ORIG(PClose, handle, 0, 0, 0);
    port_open = 0;
    return 0;
}
static SV int h_PReadState(int handle, uint8_t *data) {
    COUNT(PReadState);
    if (handle != HANDLE2) return ORIG(PReadState, handle, data, 0, 0);
    if (!data) return ERR_INVALID_ARG;
    fill(data);
    return 0;
}
static SV int h_PRead(int handle, uint8_t *data, int num) {
    COUNT(PRead);
    if (handle != HANDLE2) return ORIG(PRead, handle, data, num, 0);
    if (!data || num < 1) return ERR_INVALID_ARG;
    fill(data);
    return 1;
}
static SV int h_PInfo(int handle, uint8_t *info) {
    COUNT(PInfo);
    int r = ORIG(PInfo, handle == HANDLE2 ? 1 : handle, info, 0, 0);
    if (!r && handle == HANDLE2) { info[11] = 1; info[12] = pad_connected != 0; }
    return r;
}
static SV int h_PVib(int handle, const uint8_t *param) {
    COUNT(PVib);
    if (handle != HANDLE2) return ORIG(PVib, handle, param, 0, 0);
    if (!param) return ERR_INVALID_ARG;
    vib_large = param[0]; vib_small = param[1];
    return 0;
}
/* Answered from Player 1's pad: the same kind of controller as far as the game is concerned. */
#define ON_PAD1(n) static SV int h_##n(int handle, intptr_t a, intptr_t b, intptr_t c) { COUNT(n); return ORIG(n, handle == HANDLE2 ? 1 : handle, a, b, c); }
ON_PAD1(PDevExt) ON_PAD1(PDevParse) ON_PAD1(PRemote) ON_PAD1(PExtInfo) ON_PAD1(PTrigGet)
/* Light bar, motion, adaptive triggers: an XInput pad has none, and they must not reach Player 1's. */
#define NOT_ON_PAD2(n) static SV int h_##n(int handle, intptr_t a, intptr_t b, intptr_t c) { COUNT(n); return handle == HANDLE2 ? 0 : ORIG(n, handle, a, b, c); }
NOT_ON_PAD2(PMotion) NOT_ON_PAD2(PDeadband) NOT_ON_PAD2(PResetOri) NOT_ON_PAD2(PTilt) NOT_ON_PAD2(PTrigSet) NOT_ON_PAD2(PVibMode) NOT_ON_PAD2(PLightReset) NOT_ON_PAD2(PLightSet)

/* ---- install ---- */
static const struct { uint32_t site, fn; void *hook; } hooks[I_COUNT] = {
#define H(n, s, f) {s, f, (void *)h_##n},
#include "hooks.inc"
#undef H
};
static int patched;
static char why[160], log_path[MAX_PATH];

static int sites_match(void) {
    const IMAGE_NT_HEADERS *nt = (const IMAGE_NT_HEADERS *)(image + ((const IMAGE_DOS_HEADER *)image)->e_lfanew);
    const IMAGE_SECTION_HEADER *text = IMAGE_FIRST_SECTION(nt);
    if (memcmp(text->Name, ".text", 6) || text->VirtualAddress + text->Misc.VirtualSize > STUB_RVA)
        { snprintf(why, sizeof why, "another engine: .text"); return 0; }
    for (int i = 0; i < I_COUNT; i++) {
        const uint8_t *p = image + hooks[i].site; int32_t disp;
        memcpy(&disp, p + 3, 4);
        if (p[0] != 0x4c || p[1] != 0x8d || p[2] != 0x05 || hooks[i].site + 7 + disp != (int64_t)hooks[i].fn)
            { snprintf(why, sizeof why, "another engine: %s", hook_names[i]); return 0; }
    }
    for (uint32_t i = 0; i < I_COUNT * STUB_SIZE; i++)
        if (image[STUB_RVA + i]) { snprintf(why, sizeof why, "another engine: no room for the stubs"); return 0; }
    return 1;
}
static int install(void) {
    if (!sites_match()) return 0;
    DWORD old;
    uint8_t *stubs = image + STUB_RVA;
    if (!VirtualProtect(stubs, I_COUNT * STUB_SIZE, PAGE_EXECUTE_READWRITE, &old)) return 0;
    for (int i = 0; i < I_COUNT; i++) {
        uint8_t *s = stubs + i * STUB_SIZE;
        s[0] = 0xff; s[1] = 0x25; memset(s + 2, 0, 4);     /* jmp [rip] */
        memcpy(s + 6, &hooks[i].hook, 8);
    }
    VirtualProtect(stubs, I_COUNT * STUB_SIZE, old, &old);
    for (int i = 0; i < I_COUNT; i++) {
        uint8_t *p = image + hooks[i].site;
        const int32_t disp = (int32_t)(STUB_RVA + i * STUB_SIZE - (hooks[i].site + 7));
        if (!VirtualProtect(p, 7, PAGE_EXECUTE_READWRITE, &old)) return 0;
        memcpy(p + 3, &disp, 4);
        VirtualProtect(p, 7, old, &old);
    }
    FlushInstructionCache(GetCurrentProcess(), image, 0);
    return 1;
}

static void log_line(const char *text) {
    HANDLE f = CreateFileA(log_path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    SYSTEMTIME t; char line[300]; DWORD n;
    GetLocalTime(&t);
    n = (DWORD)snprintf(line, sizeof line, "%02d:%02d:%02d %s\r\n", t.wHour, t.wMinute, t.wSecond, text);
    WriteFile(f, line, n, &n, NULL);
    CloseHandle(f);
}
static DWORD WINAPI poll_thread(void *unused) {
    (void)unused;
    char line[200];
    HMODULE x = LoadLibraryA("xinput1_4.dll");
    typedef DWORD (WINAPI *GetState)(DWORD, XState *);
    typedef DWORD (WINAPI *SetState)(DWORD, XVib *);
    GetState get = x ? (GetState)GetProcAddress(x, "XInputGetState") : NULL;
    SetState set = x ? (SetState)GetProcAddress(x, "XInputSetState") : NULL;
    if (!get || !set) { log_line("xinput1_4.dll not available: no Player 2"); return 0; }
    log_line("hooks in, waiting for an XInput pad");
    int slot = -1, sent_large = -1, sent_small = -1, was_login = 0, was_open = 0, pressed = 0;
    LONG seen[I_COUNT] = {0};
    for (unsigned tick = 0;; tick++) {
        if (slot < 0 && tick % 250 == 0) {
            for (int i = 0; i < 4 && slot < 0; i++) {
                XState s;
                if (get(i, &s) == 0) slot = i;
            }
            if (slot >= 0) {
                pad_connected = pad_ever = 1;
                sent_large = sent_small = -1;
                snprintf(line, sizeof line, "Player 2's pad: XInput slot %d", slot); log_line(line);
            }
        }
        if (slot >= 0) {
            XState s;
            if (get(slot, &s) == 0) {
                AcquireSRWLockExclusive(&pad_lock); pad_now = s.pad; ReleaseSRWLockExclusive(&pad_lock);
                if (!pressed && s.pad.buttons) { pressed = 1; log_line("Player 2's pad: first button seen"); }
                if (vib_large != sent_large || vib_small != sent_small) {
                    XVib v = {(WORD)(vib_large * 257), (WORD)(vib_small * 257)};
                    set(slot, &v); sent_large = vib_large; sent_small = vib_small;
                }
            } else {
                AcquireSRWLockExclusive(&pad_lock); memset(&pad_now, 0, sizeof pad_now); ReleaseSRWLockExclusive(&pad_lock);
                pad_connected = 0; slot = -1; log_line("Player 2's pad: disconnected");
            }
        }
        if (tick % 500 == 0) {
            if (login_sent && !was_login) { was_login = 1; log_line("Player 2 logged in to the game"); }
            if (port_open != was_open) { was_open = port_open; log_line(was_open ? "the game opened Player 2's pad" : "the game closed Player 2's pad"); }
            for (int i = 0; i < I_COUNT; i++) {
                if (calls[i] && !seen[i]) { snprintf(line, sizeof line, "first call: %s", hook_names[i]); log_line(line); }
                seen[i] = calls[i];
            }
        }
        Sleep(2);
    }
}

#define X(n) void *v_##n;
#include "fwd_table.inc"
#undef X
static int forward_init(void) {
    wchar_t path[MAX_PATH];
    GetSystemDirectoryW(path, MAX_PATH);
    wcscat(path, L"\\version.dll");
    HMODULE m = LoadLibraryW(path);
    if (!m) return 0;
#define X(n) v_##n = (void *)GetProcAddress(m, #n);
#include "fwd_table.inc"
#undef X
    return 1;
}
BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, void *reserved) {
    (void)reserved;
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    DisableThreadLibraryCalls(dll);
    if (!forward_init()) return FALSE;
    image = (uint8_t *)GetModuleHandleW(NULL);
    char exe[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, exe, MAX_PATH);
    while (n && exe[n - 1] != '\\') n--;
    if (strcasecmp(exe + n, "UFC 6.exe")) return TRUE;
    exe[n] = 0;
    snprintf(log_path, sizeof log_path, "%s_Logs\\p2-addon.log", exe);
    WIN32_FILE_ATTRIBUTE_DATA a;
    GetModuleFileNameA(NULL, exe, MAX_PATH);
    if (!GetFileAttributesExA(exe, GetFileExInfoStandard, &a) || a.nFileSizeLow != EXE_SIZE) {
        snprintf(why, sizeof why, "another engine: size");
    } else patched = install();
    if (!patched) { log_line(why[0] ? why : "could not patch: Player 2 off"); return TRUE; }
    SetEnvironmentVariableA("SDL_GAMECONTROLLER_IGNORE_DEVICES", IGNORE_PADS);
    HANDLE t = CreateThread(NULL, 0, poll_thread, NULL, 0, NULL);
    if (t) CloseHandle(t);
    return TRUE;
}
