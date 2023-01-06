#include "mtx.hxx"
#include <Windows.h>

bool mtxRn::startMtx(const wchar_t* name)
{
	hMutexOneInstance = (::CreateMutexW(NULL, TRUE, name));
	bool bAlreadyRunning((::GetLastError() == ERROR_ALREADY_EXISTS));
	if (hMutexOneInstance == NULL || bAlreadyRunning)
	{
		if (hMutexOneInstance)
		{
			::ReleaseMutex(hMutexOneInstance);
			::CloseHandle(hMutexOneInstance);
		}
		return 0;
	}
	return 1;
}

void mtxRn::release()
{
	if (hMutexOneInstance)
	{
		::ReleaseMutex(hMutexOneInstance);
		::CloseHandle(hMutexOneInstance);
	}
}

mtxRn::~mtxRn()
{
	release();
}
