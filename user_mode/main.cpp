#include "functions.h"

int main(int argc, char* argv[])
{
	log(LogType_Info, "Initializing...");

	if (argc >= 2)
	{
		log(LogType_Info, "PE fix mode: correcting headers for IDA");
		return FixPeFile(argv[1]);
	}

	HANDLE hDevice = CreateFile(L"\\\\.\\DmpMemoryCnt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		log(LogType_Error, "Failed to open driver. Load kernel_mode.sys first.");
		return 1;
	}

	log(LogType_Success, "Driver device opened");

	DWORD processId;
	printf("Enter the process id: ");
	scanf("%d", &processId);

	INPUT_DUMP_INFO inputDumpInfo;
	inputDumpInfo.ProcessId = processId;

	printf("Enter the module name: ");
	scanf("%s", inputDumpInfo.ModuleName);

	ULONG imageSize;
	DWORD bytesReturned;

	if (!DeviceIoControl(hDevice, IOCTL_DMPCNT_IMAGESIZE, &inputDumpInfo, sizeof(inputDumpInfo), &imageSize,
		sizeof(imageSize), &bytesReturned, NULL))
	{
		log(LogType_Error, "DeviceIoControl failed. Check PID and module name.");
		CloseHandle(hDevice);
		return 1;
	}

	CHAR* image = (CHAR*)malloc(imageSize);
	if (!DeviceIoControl(hDevice, IOCTL_DMPCNT_DUMP, &inputDumpInfo, sizeof(inputDumpInfo), image, imageSize, &bytesReturned, NULL))
	{
		log(LogType_Error, "DeviceIoControl failed. Could not dump memory.");
		free(image);
		CloseHandle(hDevice);
		return 1;
	}

	log(LogType_Success, "Memory dump received from driver"); 

	CloseHandle(hDevice);

	const char* outPath = "processdumpcnt.bin";
	FILE* file;

	if (fopen_s(&file, outPath, "wb") != 0)
	{
		log(LogType_Error, "Failed to create output file");
		free(image);
		return 1;
	}

	fwrite(image, imageSize, 1, file);
	fclose(file);
	free(image);

	log(LogType_Success, "Dump saved to processdumpcnt.bin");

	system("pause");

	return FixPeFile(outPath);
}
