#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

/// <summary>
/// Registers the task with the Windows Task Scheduler
/// </summary>
bool InstallTask();

/// <summary>
/// Removes the task with the Windows Task Scheduler
/// </summary>
bool UninstallTask();

#endif