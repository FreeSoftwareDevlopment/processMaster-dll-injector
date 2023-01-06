#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <filesystem>
#include <psapi.h>
#include "toLowerCase.h"
#include "resource.h"

bool compare(const wchar_t* x, const wchar_t* y) {
	int l = lstrlenW(x);
	if (l != lstrlenW(y)) return 0;
	for (int xr = 0; xr < l; xr++) {
		if (toLowerChar(x[xr]) != toLowerChar(y[xr])) return 0;
	}
	return 1;
}

std::vector<DWORD> getProcId(const wchar_t* target) {
	std::vector<DWORD> pId;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hsnap, &pe32) == TRUE)
	{
		do {
			if (compare(pe32.szExeFile, target)) {
				if (pe32.th32ProcessID != NULL)
					pId.push_back(pe32.th32ProcessID);
			}
		} while (Process32Next(hsnap, &pe32));
	}
	CloseHandle(hsnap);
	return pId;
}

std::filesystem::path getRelativePath(const char* p) {
	char x[MAX_PATH]{ 0 };
	GetModuleFileNameExA(GetCurrentProcess(), NULL, x, MAX_PATH);
	std::filesystem::path cur = x;
	return cur.parent_path() / p;
}

int wmain(int argc, wchar_t* argv[])
{
	auto myHandle = GetModuleHandleA(NULL);
	if (IsDebuggerPresent()) {
#define sizeOfDebugInformationStringBuffer 30
		char x[sizeOfDebugInformationStringBuffer]{ 0 };
		LoadStringA(myHandle, IDS_STRINGDEBUGWARN, x, sizeOfDebugInformationStringBuffer);
		std::cout << x << "\n";
#undef sizeOfDebugInformationStringBuffer
	}

	auto p = getRelativePath("inject.dll");
	char dllPath[MAX_PATH]{ 0 };
	GetFullPathNameA(p.string().c_str(), MAX_PATH, dllPath, NULL);
	if (!std::filesystem::exists(dllPath)) {
		std::cerr << "Dll \"" << dllPath << "\" not found!\n";
		return ~(1 << 20);
	}

	if (argc <= 1) { std::cerr << "You need to give the names of the Victim Processes to Inject as argument!" << std::endl; return ~(1 << 21);}
	for (int currentProcessName = 1; currentProcessName < argc; currentProcessName++) {
		std::wcout << L"\nWorking on Process \"" << argv[currentProcessName] << L"\"\n";
		auto pId = getProcId(argv[currentProcessName]);

		size_t sizeOfPathToDll = strlen(dllPath) + 1;
		const char tError[] = "Can't access Process ID ";
		for (size_t current = 0; current < pId.size(); current++) {
			HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | SYNCHRONIZE, NULL, pId[current]);
			if (hProcess == NULL) {
				std::cerr << tError << pId[current] << "\n";
				continue;
			}
			LPVOID libRem = VirtualAllocEx(hProcess, NULL, sizeOfPathToDll, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (libRem == NULL) {
				std::cerr << tError << pId[current] << "\n";
				continue;
			}

			if (WriteProcessMemory(hProcess, libRem, dllPath, sizeOfPathToDll, NULL) != TRUE) {
				std::cerr << "Can't start PID " << pId[current] << "\n";
				continue;
			}
			HANDLE thread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryA, libRem, NULL, NULL);
			if (thread == NULL) {
				std::cerr << "Can't start thread on PID " << pId[current] << "\n";
				continue;
			}
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			VirtualFreeEx(hProcess, libRem, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			std::cout << "Added to PID " << pId[current] << "\n";
		}
		std::wcout << L"Completed Process \"" << argv[currentProcessName] << L"\"\n";
	}
	return 0;
}
