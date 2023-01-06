#include <Windows.h>
#include "mtx.hxx"
#include <filesystem>

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

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpvReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		CreateThread(NULL, NULL, mthread, hinstDLL, NULL, NULL);
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:

		if (lpvReserved != nullptr)
		{
			break; // do not do cleanup if process termination scenario
		}

		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
