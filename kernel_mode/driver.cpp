#include <ntdef.h>
#include <ntddk.h>
#include "driver.h"
#include "memory.h"

#define IOCTL_DMPCNT_DUMP      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMPCNT_IMAGESIZE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMPCNT_PRINT     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

NTSTATUS DrvCreate(PDEVICE_OBJECT DriverObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(DriverObject);

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DrvClose(PDEVICE_OBJECT DriverObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(DriverObject);

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DrvDeviceControl(PDEVICE_OBJECT DriverObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(DriverObject);

	PIO_STACK_LOCATION pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	PVOID pInBuffer = pIrp->AssociatedIrp.SystemBuffer;
	PVOID pOutBuffer = pIrp->AssociatedIrp.SystemBuffer;
	ULONG inLen = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG outLen = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG ioctl = pIoStack->Parameters.DeviceIoControl.IoControlCode;

	NTSTATUS status = STATUS_SUCCESS;

	switch (ioctl)
	{
	case IOCTL_DMPCNT_DUMP:
	{
		DbgPrint("[Hook] Write: memory dump");
		if (inLen != sizeof(INPUT_DUMP_INFO))
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		PINPUT_DUMP_INFO pInput = (PINPUT_DUMP_INFO)pInBuffer;
		PEPROCESS pProcess;

		status = PsLookupProcessByProcessId((HANDLE)pInput->ProcessId, &pProcess);
		if (!NT_SUCCESS(status))
			break;

		ANSI_STRING nameAnsi;
		UNICODE_STRING nameUni;
		RtlInitAnsiString(&nameAnsi, pInput->ModuleName);

		status = RtlAnsiStringToUnicodeString(&nameUni, &nameAnsi, TRUE);

		if (!NT_SUCCESS(status))
			break;

		status = DumpProcessMemory(pProcess, &nameUni, pOutBuffer);
		RtlFreeUnicodeString(&nameUni);

		if (NT_SUCCESS(status))
			pIrp->IoStatus.Information = outLen;

		break;
	}

	case IOCTL_DMPCNT_IMAGESIZE:
	{
		DbgPrint("[Hook] Read: image size");
		if (inLen != sizeof(INPUT_DUMP_INFO))
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		PINPUT_DUMP_INFO pInput = (PINPUT_DUMP_INFO)pInBuffer;
		PEPROCESS pProcess;

		status = PsLookupProcessByProcessId((HANDLE)pInput->ProcessId, &pProcess);

		if (!NT_SUCCESS(status))
			break;

		ANSI_STRING nameAnsi;
		UNICODE_STRING nameUni;
		RtlInitAnsiString(&nameAnsi, pInput->ModuleName);

		status = RtlAnsiStringToUnicodeString(&nameUni, &nameAnsi, TRUE);

		if (!NT_SUCCESS(status))
			break;

		MODULE_INFO ModuleInfo = { 0 };
		KAPC_STATE ApcState = { 0 };

		KeStackAttachProcess((PRKPROCESS)pProcess, &ApcState);

		status = GetModuleBaseAddressInfo(pProcess, &nameUni, &ModuleInfo);

		KeUnstackDetachProcess(&ApcState);

		if (!NT_SUCCESS(status) || ModuleInfo.BaseAddress == NULL)
		{
			status = STATUS_NOT_FOUND;
		}
		else
		{
			*(PULONG)pOutBuffer = ModuleInfo.SizeOfImage;
			pIrp->IoStatus.Information = sizeof(ULONG);
		}
		RtlFreeUnicodeString(&nameUni);
		break;
	}

	case IOCTL_DMPCNT_PRINT:
	default:
		break;
	}

	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DriverInitialize(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->MajorFunction[IRP_MJ_CREATE] = DrvCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DrvClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DrvDeviceControl;
	DriverObject->DriverUnload = UnloadDriver;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\DmpMemoryCnt");

	PDEVICE_OBJECT pDevice;

	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, TRUE, &pDevice);

	if (!NT_SUCCESS(status))
		return status;

	UNICODE_STRING linkName;
	RtlInitUnicodeString(&linkName, L"\\DosDevices\\DmpMemoryCnt");

	status = IoCreateSymbolicLink(&linkName, &devName);

	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevice);
		return status;
	}

	pDevice->Flags |= DO_BUFFERED_IO;
	pDevice->Flags &= ~DO_DEVICE_INITIALIZING;

	DbgPrint("[Hook] Driver initialized");
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	if (!DriverObject)
	{
		UNICODE_STRING driverName;
		RtlInitUnicodeString(&driverName, L"\\Driver\\DmpMemoryCnt");
		return IoCreateDriver(&driverName, &DriverInitialize);
	}

	return DriverInitialize(DriverObject, RegistryPath);
}

VOID UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
	UNREFERENCED_PARAMETER(pDriverObject);

	UNICODE_STRING linkName;
	RtlInitUnicodeString(&linkName, L"\\DosDevices\\DmpMemoryCnt");
	IoDeleteSymbolicLink(&linkName);

	if (pDriverObject->DeviceObject)
	{
		IoDeleteDevice(pDriverObject->DeviceObject);
	}	
}
