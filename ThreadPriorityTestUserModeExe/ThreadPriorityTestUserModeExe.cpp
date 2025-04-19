#include <Windows.h>
#include <stdio.h>
#include "..\ThreadPriorityIncrement\ThreadPriority.h"

int Error(int code = GetLastError()) {
	printf("Error: %d\n", code);
	return 1;
}

int main(int argc, CHAR *argv[]) {

	if (argc < 3) {
		printf("Usage: ThreadPriorityTestUserModeExe <threadId> <priority> \n");
		return 1;
	}

	HANDLE hFile = CreateFile(L"\\\\.\\ThreadPriority", GENERIC_READ | GENERIC_WRITE, 
		0, nullptr, OPEN_EXISTING, 0, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		printf("Error: %d\n", GetLastError());
		return 1;
	}

	printf("Handle opened.\ n");

	ThreadPriorityData data;
	data.hThread = OpenThread(THREAD_SET_INFORMATION, FALSE, atoi(argv[1]));

	if (data.hThread == nullptr) {
		return Error();
	}

	data.Priority = atoi(argv[2]);
	
	DWORD dummy;

	BOOL ok = DeviceIoControl(hFile, IOCTL_THREAD_PRIORITY, &data, sizeof(data), nullptr, 0, &dummy, nullptr);

	if (!ok) return Error();

	printf("Succeeded !!!! \n");

	CloseHandle(hFile);
	return 0;
}