#include "event_log.h"

#include <iostream>
#include <string>
#include <sstream>

#include <Windows.h>

#include "EventLog/TransitionFixerEventProvider.h"
#include "utils.h"

namespace {
	constexpr LPCWSTR APP_NAME = L"TransitionFixer";
	constexpr LPCWSTR KEY_PATH = L"SYSTEM\\CurrentControlset\\Services\\EventLog\\Application\\TransitionFixer";

	bool LogMessage(WORD eventType, const std::wstring& message)
	{
		DWORD eventID;
		switch (eventType) {
			case EVENTLOG_ERROR_TYPE:
				eventID = MSG_ERROR;
				break;

			case EVENTLOG_WARNING_TYPE:
				eventID = MSG_WARNING;
				break;

			case EVENTLOG_INFORMATION_TYPE:
			default:
				eventID = MSG_INFO;
				break;
		}

		HANDLE eventLog = RegisterEventSourceW(NULL, APP_NAME);
		if (eventLog == NULL) {
			std::cerr << "Failed to open event log: ";
			std::wcerr << GetLastWin32Error();

			return false;
		}

		LPCWSTR messagePtr = message.c_str();
		BOOL succeeded = ReportEventW(
			eventLog,
			eventType,
			0,
			eventID,
			0,
			1,
			0,
			&messagePtr,
			0);
		if (!succeeded) {
			std::cerr << "Failed to write to event log: ";
			std::wcerr << GetLastWin32Error();

			DeregisterEventSource(eventLog);
			return false;
		}

		DeregisterEventSource(eventLog);
		return true;
	}
}

bool InstallEventLogSource()
{
	// Create the registry key needed to register our event source into the Windows Event Viewer.
	HKEY eventKey;
	DWORD result = RegCreateKeyExW(
		HKEY_LOCAL_MACHINE,
		KEY_PATH,
		0, 0,
		REG_OPTION_NON_VOLATILE,
		KEY_SET_VALUE,
		0,
		&eventKey,
		0);
	if (result != ERROR_SUCCESS) {
		std::cerr << "Failed to register event log source: ";
		std::wcerr << GetWin32Error(result);

		return false;
	}

	std::wstring exePath = GetExePath();
	if (exePath.empty()) {
		std::cerr << "Failed to get executable path: ";
		std::wcerr << GetLastWin32Error();

		RegCloseKey(eventKey);
		return false;
	}

	// Set up the values for the key
	result = RegSetValueExW(
		eventKey,
		L"EventMessageFile",
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(exePath.c_str()),
		// NOTE: https://stackoverflow.com/a/9278794/3145126
		static_cast<DWORD>((exePath.size() + 1) * sizeof(wchar_t)));
	if (result != ERROR_SUCCESS) {
		std::cerr << "Failed to register event log source: ";
		std::wcerr << GetWin32Error(result);

		RegCloseKey(eventKey);
		return false;
	}

	const DWORD SUPPORTED_TYPES =
		EVENTLOG_ERROR_TYPE |
		EVENTLOG_WARNING_TYPE |
		EVENTLOG_INFORMATION_TYPE;
	result = RegSetValueExW(
		eventKey,
		L"TypesSupported",
		0,
		REG_DWORD,
		reinterpret_cast<const BYTE*>(&SUPPORTED_TYPES),
		sizeof(DWORD));
	if (result != ERROR_SUCCESS) {
		std::cerr << "Failed to register event log source: ";
		std::wcerr << GetWin32Error(result);

		RegCloseKey(eventKey);
		return false;
	}

	RegCloseKey(eventKey);
	return true;
}

bool IsEventLogSourceInstalled()
{
	HKEY key;
	LSTATUS status = RegOpenKeyExW(
		HKEY_LOCAL_MACHINE,
		KEY_PATH,
		0,
		KEY_READ,
		&key
	);
	if (status != ERROR_SUCCESS) {
		// This implies that the key does not exist.
		return false;
	}

	// The key did exist. Don't forget to clean up after ourselves, and close it.
	RegCloseKey(key);
	return true;
}

bool UninstallEventLogSource()
{
	// If the event log source is not even installed, we can exit safely.
	if (!IsEventLogSourceInstalled()) {
		std::cerr << "Note: Event log source not found, skipping...\n";
		return true;
	}

	LSTATUS result = RegDeleteKeyW(HKEY_LOCAL_MACHINE, KEY_PATH);
	if (result != ERROR_SUCCESS) {
		std::wstringstream error;
		error
			<< L"Failed to uninstall event log source: "
			<< GetWin32Error(result);

		LogError(error.str());
		return false;
	}

	return true;
}

void LogInfo(const std::wstring& message)
{
	if (IsEventLogSourceInstalled()) {
		LogMessage(EVENTLOG_INFORMATION_TYPE, message);
	}

	std::wcerr << message << L"\n";
}

void LogError(const std::wstring& message)
{
	if (IsEventLogSourceInstalled()) {
		LogMessage(EVENTLOG_ERROR_TYPE, message);
	}

	std::wcerr << message << L"\n";
}


