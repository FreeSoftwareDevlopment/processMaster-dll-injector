#include <iostream>
#include <Windows.h>
#include <cstdio>

int main()
{
	std::cout << "Hello World!\nPress any Key...";
	getchar();
	MessageBoxA(NULL, "Hello World", "Hi", MB_OK | MB_ICONINFORMATION);
}
