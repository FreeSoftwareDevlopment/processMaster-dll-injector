#include <Windows.h>
#include <detours/detours.h>
#include <tlhelp32.h>
#include <cstdio>
#include "detoursInternal.h"

namespace det {
	void dUt() {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		auto i = GetCurrentProcessId();
		auto id = GetCurrentThreadId();
		HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, i);
		if (h != INVALID_HANDLE_VALUE) {
			THREADENTRY32 te;
			te.dwSize = sizeof(te);
			if (Thread32First(h, &te)) {
				do {
					if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
						sizeof(te.th32OwnerProcessID)) {
						if (te.th32OwnerProcessID != i || te.th32ThreadID == id) continue;
						auto h = OpenThread(THREAD_ALL_ACCESS & (~(WRITE_OWNER | DELETE)), NULL, te.th32ThreadID);
						if (h == NULL) continue;
						if (DetourUpdateThread(h) == NO_ERROR) {
							printf("Changed at Process 0x%04x Thread 0x%04x\n",
								te.th32OwnerProcessID, te.th32ThreadID);
						}
						else printf("ErrOnThread\n");
					}
					te.dwSize = sizeof(te);
				} while (Thread32Next(h, &te));
			}
			else {
				fputs("Invalid Handle", stderr);
			}
			CloseHandle(h);
		}
	}

	void loadDetours(PVOID* real, PVOID detoured) {
		DetourRestoreAfterWith();

		printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
			" Starting.\n");
		fflush(stdout);

		dUt();
		DetourAttach(real, detoured);
		LONG error = DetourTransactionCommit();

		if (error == NO_ERROR)
			printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
				" Detoured MsgB().\n");
		else 
			printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
				" Error detouring MsgB(): %ld\n", error);
	}

	void disMountDe(PVOID* real, PVOID detoured) {
		dUt();
		DetourDetach(real, detoured);
		LONG error = DetourTransactionCommit();

		printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
			" Removed MsgB() (result=%ld)\n", error);
		fflush(stdout);
	}
}
