#include "toLowerCase.h"
#include <wchar.h>

const wchar_t upperChar[] = L"QWERTZUIOPÜASDFGHJKLÖÄYXCVBNM";
const wchar_t lowerChar[] = L"qwertzuiopüasdfghjklöäyxcvbnm";
wchar_t toLowerChar(wchar_t c){
	static_assert(sizeof(upperChar) == sizeof(lowerChar), "Basic Character Strings for Convertion same size");
	for (int x = 0; x < wcslen(upperChar);x++) {
		if(upperChar[x] == c){
			c = lowerChar[x];
			break;
		}
	}
	return c;
}
