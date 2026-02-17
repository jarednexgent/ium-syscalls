#pragma once


#ifndef HELLSHALL_H
#define HELLSHALL_H

#include <Windows.h>


typedef struct _NT_SYSCALL
{
    DWORD dwSSn;                    // Syscall number
    PVOID pSyscallInstAddress;      // Address of a random 'syscall' instruction in iumdll.dll 

}NT_SYSCALL, * PNT_SYSCALL;

BOOL			FetchNtSyscall(IN DWORD dwSysHash, OUT PNT_SYSCALL pNtSys);
extern VOID		SetSSn(IN DWORD dwSSn, IN PVOID pSyscallInstAddress);
extern			RunSyscall();
UINT32          CRC32BA(IN LPCSTR cString);

#define SET_SYSCALL(NtSys)(SetSSn((DWORD)NtSys.dwSSn,(PVOID)NtSys.pSyscallInstAddress))
#define HASH(String)(CRC32BA((LPCSTR)String))


#endif // !HELLSHALL_H
