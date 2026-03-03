#include "memory.h"
#include "driver.h"

NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	SIZE_T Bytes = 0;
	return MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(), TargetAddress, Size, KernelMode, &Bytes);
}

NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	SIZE_T Bytes = 0;
	return MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process, TargetAddress, Size, KernelMode, &Bytes);
}

NTSTATUS GetModuleBaseAddressInfo(PEPROCESS Process, PUNICODE_STRING ModuleName, PMODULE_INFO ModuleInfo)
{
	PPEB Peb = PsGetProcessPeb(Process);

	if (!Peb || !Peb->Ldr)
		return STATUS_UNSUCCESSFUL;

	PLIST_ENTRY ListEntry = Peb->Ldr->InMemoryOrderModuleList.Flink;

	while (ListEntry != &Peb->Ldr->InMemoryOrderModuleList)
	{
		PLDR_DATA_TABLE_ENTRY Entry = CONTAINING_RECORD(ListEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		if (Entry->BaseDllName.Buffer && RtlCompareUnicodeString(&Entry->BaseDllName, ModuleName, TRUE) == 0)
		{
			ModuleInfo->BaseAddress = Entry->DllBase;
			ModuleInfo->EntryPoint = Entry->EntryPoint;
			ModuleInfo->SizeOfImage = Entry->SizeOfImage;
			ModuleInfo->FullDllName = Entry->FullDllName;
			ModuleInfo->BaseDllName = Entry->BaseDllName;
			return STATUS_SUCCESS;
		}

		ListEntry = ListEntry->Flink;
	}

	return STATUS_NOT_FOUND;
}

NTSTATUS DumpProcessMemory(PEPROCESS Process, PUNICODE_STRING ModuleName, PVOID OutBuffer)
{
	MODULE_INFO ModuleInfo;
	KAPC_STATE ApcState;
	NTSTATUS Status;

	KeStackAttachProcess((PRKPROCESS)Process, &ApcState);

	Status = GetModuleBaseAddressInfo(Process, ModuleName, &ModuleInfo);

	if (!NT_SUCCESS(Status))
	{
		KeUnstackDetachProcess(&ApcState);
		return Status;
	}

	Status = KeReadVirtualMemory(Process, ModuleInfo.BaseAddress, OutBuffer, ModuleInfo.SizeOfImage);

	KeUnstackDetachProcess(&ApcState);
	return Status;
}
