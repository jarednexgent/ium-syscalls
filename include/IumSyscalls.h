#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------------------------------------------------------
// Function prototype to satisfy the linker and trigger the loading of 'iumbase.dll'


__declspec(dllimport) BOOLEAN WINAPI IsSecureProcess(void);


//------------------------------------------------------------------------------------------------------------
// Function that loads 'iumbase.dll' (IAT) that proxy loads 'iumdll.dll'.

VOID AddIumdllToIAT(void);

//------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
