#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <string>

/// <summary>
/// Registers the event log source with the Windows Event Viewer
/// </summary>
/// <returns><see langword="true" /> if it succeeds, else <see langword="false" />.</returns>
bool InstallEventLogSource();

/// <summary>
/// Gets a value indicating whether the event log source has been installed
/// </summary>
/// <returns><see langword="true" /> if it exists, else <see langword="false" />.</returns>
bool IsEventLogSourceInstalled();

/// <summary>
/// Uninstalls the event log source from the Windows Event Viewer
/// </summary>
/// <returns><see langword="true" /> if it succeeds, else <see langword="false" />.</returns>
bool UninstallEventLogSource();

/// <summary>
/// Logs an info message to the event log.
/// </summary>
/// <param name="message">The message.</param>
void LogInfo(const std::wstring& message);

/// <summary>
/// Logs an error message to the event log.
/// </summary>
/// <param name="message">The message.</param>
void LogError(const std::wstring& message);

#endif