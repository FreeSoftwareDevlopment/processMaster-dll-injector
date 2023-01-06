#pragma once

class mtxRn {
private:
	void* hMutexOneInstance;
public:
	bool startMtx(const wchar_t* name);
	void release();
	~mtxRn();
};
