#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <Windows.h>

/// <summary>
/// Gets the path to this executable.
/// </summary>
/// <returns>The executable path or an empty string if it cannot be deduced.</returns>
std::wstring GetExePath();

/// <summary>
/// Gets the message for an error code returned by the Win32 error message.
/// </summary>
/// <param name="errorCode">The error code.</param>
/// <returns>A message describing the error code.</returns>
std::wstring GetWin32Error(DWORD errorCode);

/// <summary>
/// Gets the last error reported by the Win32 API that occurred.
/// </summary>
/// <returns>A message describing the error code.</returns>
std::wstring GetLastWin32Error();

#endif