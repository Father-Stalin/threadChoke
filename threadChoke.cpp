
#include <iostream>
#include <windows.h>
#include <vector>
#include <TlHelp32.h>
#include <string.h>
using std::vector;
using std::cout;
using std::endl;


int Error(const char* text) {
	cout << text << " " << GetLastError() << endl;
	return 1;
}


vector<DWORD> GetProcessThreads(DWORD pid) {
	vector<DWORD> tids;

	auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return tids;

	THREADENTRY32 te = { sizeof(te) };
	if (Thread32First(hSnapshot, &te)) {
		do {
			if (te.th32OwnerProcessID == pid) {
				tids.push_back(te.th32ThreadID);
			}
		} while (Thread32Next(hSnapshot, &te));
	}

	CloseHandle(hSnapshot);
	return tids;
}


int main(int argc, char *argv[])
{
	if (argc < 2) {
		cout << "Incorrect args!" << endl;
		cout << "Usage: " << argv[0] << " PID" << "-s/-k :: --suspend/--kill" << endl;
		cout << "Suspends/ terminates threads of a process given the PID. Default is suspend" << endl;
		return 0;
	}
	DWORD procMode = PROCESS_SUSPEND_RESUME;
	DWORD threadMode = THREAD_SUSPEND_RESUME;
	char suspend[] = "--suspend";
	char shortSuspend[] = "-s";
	char kill[] = "--kill";
	char shortKill[] = "-k";

	if (argv[2]) {
		if(!strncmp(argv[2], kill, 10) || !strncmp(argv[2], shortKill, 10)){
			threadMode = THREAD_TERMINATE;
		}
	}
	int pid = atoi(argv[1]);
	HANDLE hProcess = OpenProcess(procMode, FALSE, pid);
	if (!hProcess)
		return Error("Failed to open process");

	auto tids = GetProcessThreads(pid);
	if (tids.empty()) {
		cout << "Failed to locate threads in process " << pid << endl;
		return Error("");
	}
	for (const DWORD tid : tids) {
		HANDLE hThread = OpenThread(threadMode, FALSE, tid);
		if (hThread && threadMode == THREAD_SUSPEND_RESUME) {
			SuspendThread(hThread);
			CloseHandle(hThread);
		}
		else if (hThread && threadMode == THREAD_TERMINATE) {
			TerminateThread(hThread,NULL);
			CloseHandle(hThread);
		}
	}

	CloseHandle(hProcess);
}
