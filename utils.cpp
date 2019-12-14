#include "utils.h"

#include <atlstr.h>
#include <Windows.h>

std::wstring GetWin32Error(DWORD errorCode)
{
	LPWSTR errorText = nullptr;
	DWORD result = FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&errorText),
		0,
		nullptr);
	if (result == 0) {
		// This call failed for some reason...
		return std::wstring();
	}

	std::wstring error(errorText);
	LocalFree(errorText);

	return error;
}

std::wstring GetLastWin32Error() 
{
	DWORD lastError = GetLastError();

	return GetWin32Error(lastError);
}
