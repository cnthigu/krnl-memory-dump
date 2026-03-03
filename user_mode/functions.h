#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "logs.h"

#define IOCTL_DMPCNT_DUMP      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMPCNT_IMAGESIZE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define MODULE_NAME_MAX 260

typedef struct _INPUT_DUMP_INFO
{
	ULONG ProcessId;
	CHAR ModuleName[MODULE_NAME_MAX];
} INPUT_DUMP_INFO, * PINPUT_DUMP_INFO;

static int FixPeFile(const char* filepath)
{
	HANDLE hFile = CreateFileA(filepath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		log(LogType_Error, "CreateFile failed: cannot open file");
		return 1;
	}

	DWORD fileSize = GetFileSize(hFile, NULL);
	BYTE* buffer = (BYTE*)malloc(fileSize);
	DWORD bytesRead;

	if (!buffer || !ReadFile(hFile, buffer, fileSize, &bytesRead, NULL))
	{
		log(LogType_Error, "ReadFile failed");

		if (buffer)
		{
			free(buffer);
		}

		CloseHandle(hFile);

		return 1;
	}

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)buffer;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)(buffer + dosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeader);

	for (unsigned int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		sectionHeader[i].PointerToRawData = sectionHeader[i].VirtualAddress;
		sectionHeader[i].SizeOfRawData = sectionHeader[i].Misc.VirtualSize;
	}

	ntHeader->OptionalHeader.SizeOfImage = sectionHeader[ntHeader->FileHeader.NumberOfSections - 1].VirtualAddress
		+ sectionHeader[ntHeader->FileHeader.NumberOfSections - 1].Misc.VirtualSize;
	ntHeader->OptionalHeader.SizeOfHeaders = sectionHeader[0].PointerToRawData;

	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
	ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

	if (!WriteFile(hFile, buffer, fileSize, &bytesRead, NULL))
	{
		log(LogType_Error, "WriteFile failed");

		free(buffer);
		CloseHandle(hFile);

		return 1;
	}

	log(LogType_Success, "PE headers corrected for IDA");

	free(buffer);
	CloseHandle(hFile);

	return 0;
}
