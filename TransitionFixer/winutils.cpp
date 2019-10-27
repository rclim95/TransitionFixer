#include "winutils.h"

#include <system_error>

#include <Windows.h>

void Windows::EnableActiveDesktop()
{
	HWND progmanWindow = FindWindow(L"Progman", NULL);
	if (progmanWindow == NULL) {
		// We couldn't find the window.
		throw std::system_error{
			static_cast<int>(GetLastError()), 
			std::system_category(), 
			"Unable to find window \"Progman\""};
	}

	// Send a message to the window to enable Active Desktop. For more information:
	// https://stackoverflow.com/questions/14773287/iactivedesktop-wallpaper-fade-effect-not-working-after-restart
	DWORD messageOutput = 0;
	LRESULT result = SendMessageTimeout(progmanWindow, 0x52C, NULL, NULL, 0, 500, &messageOutput);
	if (result == 0) {
		// We couldn't send this message.
		throw std::system_error{
			static_cast<int>(GetLastError()),
			std::system_category(),
			"Failed to send message \"0x52C\" to \"Progman\"." };
	}
}
