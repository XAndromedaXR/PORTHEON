/* version.dll's API, forwarded to the system's copy (the pointers are set in DllMain). */
.text
.macro F n
.globl p_\n
p_\n:
    jmp *v_\n(%rip)
.endm
F GetFileVersionInfoA
F GetFileVersionInfoByHandle
F GetFileVersionInfoExA
F GetFileVersionInfoExW
F GetFileVersionInfoSizeA
F GetFileVersionInfoSizeExA
F GetFileVersionInfoSizeExW
F GetFileVersionInfoSizeW
F GetFileVersionInfoW
F VerFindFileA
F VerFindFileW
F VerInstallFileA
F VerInstallFileW
F VerLanguageNameA
F VerLanguageNameW
F VerQueryValueA
F VerQueryValueW
