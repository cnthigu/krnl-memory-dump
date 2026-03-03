#pragma once
#include <ntdef.h>
#include <ntddk.h>


typedef UCHAR BYTE;
typedef USHORT WORD;
typedef ULONG DWORD;


typedef struct _MODULE_INFO {
	PVOID BaseAddress;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
} MODULE_INFO, *PMODULE_INFO;


typedef struct _KAPC_STATE {
	LIST_ENTRY ApcListHead[2];
	struct _KPROCESS* Process;
	UCHAR InProgressFlags;
	UCHAR KernelApcPending;
	UCHAR UserApcPendingAll;
} KAPC_STATE, *PKAPC_STATE, *PRKAPC_STATE;

typedef struct _KPROCESS* PRKPROCESS;

#ifdef __cplusplus
extern "C" {
#endif
NTKERNELAPI VOID KeStackAttachProcess(PRKPROCESS Process, PRKAPC_STATE ApcState);
NTKERNELAPI VOID KeUnstackDetachProcess(PRKAPC_STATE ApcState);
#ifdef __cplusplus
}
#endif


typedef struct _PEB_LDR_DATA {
	BYTE Reserved1[8];
	PVOID Reserved2[3];
	LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY {
	PVOID Reserved1[2];
	LIST_ENTRY InMemoryOrderLinks;
	PVOID Reserved2[2];
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	PVOID Reserved5[3];
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[5];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
} PEB, *PPEB;


NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size);

NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size);

NTSTATUS GetModuleBaseAddressInfo(PEPROCESS Process, PUNICODE_STRING ModuleName, PMODULE_INFO ModuleInfo);

NTSTATUS DumpProcessMemory(PEPROCESS Process, PUNICODE_STRING ModuleName, PVOID OutBuffer);