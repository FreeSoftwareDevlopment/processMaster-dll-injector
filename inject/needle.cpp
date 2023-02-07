#include <Windows.h>
#include "mtx.hxx"
#include <filesystem>
#include <detours/detours.h>
#include "detoursInternal.h"
#include <string>

DWORD WINAPI mthread(LPVOID p) {
	wchar_t fname[MAX_PATH]{ 0 };
	GetModuleFileNameW(NULL, fname, MAX_PATH);
	std::filesystem::path x = fname;
	x = x.filename();
	while (1) {

		if (GetAsyncKeyState(VK_F6) & 0x80000) {
			mtxRn crashOk;
			if (crashOk.startMtx((L"InjectedTo" + x.wstring()).c_str())) {
				if (MessageBoxW(NULL, L"🐋 Do you want to crash this?", fname, MB_YESNO | MB_DEFBUTTON2 | MB_ICONHAND | MB_TOPMOST) == IDYES) {
					volatile int* pInt = 0x00000000;
					*pInt = 0x02;
				}
				crashOk.release();
			}
		}
		if (GetAsyncKeyState(VK_F8) & 0x80000) {
			break;
		}
		Sleep(100);
	}
	return 0;
}

static int(WINAPI* TrueMsgA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) = MessageBoxA;

int WINAPI FakeMsgA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	std::string capt = "Detoured ";
	capt += lpCaption;
	return TrueMsgA(hWnd, lpText, capt.c_str(), uType | MB_TOPMOST);
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)  // reserved
{
	(void)hinst;
	(void)reserved;

	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	// Perform actions based on the reason for calling.
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		det::loadDetours(&(PVOID&)TrueMsgA, FakeMsgA);
		CreateThread(NULL, NULL, mthread, hinst, NULL, NULL);
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		det::disMountDe(&(PVOID&)TrueMsgA, FakeMsgA);
		if (reserved != nullptr)
		{
			break; // do not do cleanup if process termination scenario
		}

		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
