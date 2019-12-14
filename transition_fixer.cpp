#include "transition_fixer.h"

#include <iostream>
#include <system_error>
#include <string>
#include <sstream>

#include <Windows.h>

#include "event_log.h"
#include "utils.h"

namespace {
	constexpr LPCWSTR PROGMAN_NAME = L"Progman";
	constexpr UINT DEFAULT_TIMEOUT = 500;
	constexpr UINT WM_ENABLE_ACTIVEDESKTOP = WM_USER + 0x12C;
}

bool ApplyFadeFix()
{
	// Look for the "Progman" handle. This is the handle to the window that is responsible for
	// displaying the user's wallpaper.
	HWND progmanHandle = FindWindowW(PROGMAN_NAME, nullptr);
	if (progmanHandle == nullptr) {
		std::wstringstream error;
		error << "Failed to locate " << PROGMAN_NAME << ": ";
		error << GetLastWin32Error();

		LogError(error.str());
		return false;
	}

	// Now send a message to it so that it'll enable Active Desktop.
	ULONGLONG output = 0;
	LRESULT result = SendMessageTimeoutW(progmanHandle,
						WM_ENABLE_ACTIVEDESKTOP,
						NULL, NULL, SMTO_NORMAL,
						DEFAULT_TIMEOUT,
						&output);
	if (result == 0) {
		std::wstringstream error;
		error << "Failed to send message to " << PROGMAN_NAME << ": ";
		error << GetLastWin32Error();

		LogError(error.str());
		return false;
	}

	return true;
}
