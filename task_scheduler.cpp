#include "task_scheduler.h"
#include "utils.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

#include <atlcomcli.h>
#include <comdef.h>
#include <lmcons.h>
#include <security.h>
#include <taskschd.h>
#include <Windows.h>

#include "wil/com.h"
#include "wil/result.h"
#include "wil/stl.h"

namespace {
    std::wstring GetCurrentDateTime()
    {
        std::wstringstream stream;
        std::time_t currentTime = std::time(nullptr);
        std::tm currentTimeUtc = *std::gmtime(&currentTime);
        stream << std::put_time(&currentTimeUtc, L"%FT%T");

        return stream.str();
    }

    std::wstring GetUserID()
    {
        constexpr size_t NameBufferSize = DNLEN + UNLEN + 1; // Include (+1) for Domain\Username backslash separator.
        WCHAR nameBuffer[NameBufferSize];
        ULONG nameBufferSize = NameBufferSize;
        BOOLEAN result = GetUserNameExW(NameSamCompatible, nameBuffer, &nameBufferSize);
        if (result) {
            return std::wstring(nameBuffer, nameBufferSize);
        }
        else {
            return std::wstring();
        }
    }

    void SetRegisterationInfo(ITaskDefinition* task)
    {
        wil::com_ptr_t<IRegistrationInfo> registrationInfo;
        THROW_IF_FAILED(task->get_RegistrationInfo(&registrationInfo));

        auto currentDateTime = GetCurrentDateTime();
        auto author = wil::make_bstr(L"Limotto Productions");
        auto description = wil::make_bstr(
            L"Fixes an issue where the Windows desktop does not play a fade transition effect "
            L"when changing wallpapers by enabling Active Desktop."
        );
        auto version = wil::make_bstr(L"1.0");
        auto date = wil::make_bstr(currentDateTime.c_str());

        registrationInfo->put_Author(author.get());
        registrationInfo->put_Description(description.get());
        registrationInfo->put_Version(version.get());
        registrationInfo->put_Date(date.get());
    }

    void SetSettings(ITaskDefinition* task)
    {
        wil::com_ptr_t<ITaskSettings> settings;
        THROW_IF_FAILED(task->get_Settings(&settings));

        settings->put_StartWhenAvailable(VARIANT_TRUE);
    }

    void SetTriggers(ITaskDefinition* task)
    {
        wil::com_ptr_t<ITriggerCollection> triggerCollection;
        THROW_IF_FAILED(task->get_Triggers(&triggerCollection));

        // Add the login trigger to the task's triggers
        wil::com_ptr_t<ITrigger> trigger;
        THROW_IF_FAILED(triggerCollection->Create(TASK_TRIGGER_LOGON, &trigger));
        wil::com_ptr_t<ILogonTrigger> logonTrigger = trigger.query<ILogonTrigger>();

        std::wstring userID = GetUserID();
        auto userIDBStr = wil::make_bstr(userID.c_str());
        auto delay = wil::make_bstr(L"PT30S"); // Put a 30s delay, in case Explorer hasn't initialized immediately.

        logonTrigger->put_UserId(userIDBStr.get());
        logonTrigger->put_Delay(delay.get()); 
    }

    void SetAction(ITaskDefinition* task)
    {
        // Get the action collections for this task
        wil::com_ptr_t<IActionCollection> actionCollection;
        THROW_IF_FAILED(task->get_Actions(&actionCollection));

        // Add the executable action to the task's actions
        wil::com_ptr_t<IAction> action;
        THROW_IF_FAILED(actionCollection->Create(TASK_ACTION_EXEC, &action));
        wil::com_ptr_t<IExecAction> execAction = action.query<IExecAction>();

        std::wstring execPath = GetExePath();
        std::wstring execPathWithoutName = execPath.substr(0, execPath.find_last_of('\\'));
        auto path = wil::make_bstr(execPath.c_str());
        auto workingDir = wil::make_bstr(execPathWithoutName.c_str());
        auto arguments = wil::make_bstr(L"run");

        execAction->put_Path(path.get());
        execAction->put_WorkingDirectory(workingDir.get());
        execAction->put_Arguments(arguments.get());
    }

    void SaveTask(ITaskFolder* rootFolder, ITaskDefinition* task)
    {
        std::wstring currentUser = GetUserID();
        auto taskName = wil::make_bstr(L"Transition Fixer");

        wil::com_ptr_t<IRegisteredTask> registeredTask;
        THROW_IF_FAILED(rootFolder->RegisterTaskDefinition(
            taskName.get(),
            task,
            TASK_CREATE_OR_UPDATE,
            _variant_t(currentUser.c_str()),
            _variant_t(),
            TASK_LOGON_NONE,
            _variant_t(L""),
            &registeredTask)
        );
    }
}

bool InstallTask()
{
    // Initialize COM, using wil::CoInitializeEx so that we don't need to worry about
    // cleaning up after ourselves.
    const auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);
    
    // Setup general COM security so that we're impersonating the current user.
    THROW_IF_FAILED(CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL));

    // Access the Windows Task Service API by creating an instance of it and attempt to connect
    // to the Task Scheduler service on the local machine.
    wil::com_ptr<ITaskService> taskService = wil::CoCreateInstance<ITaskService>(CLSID_TaskScheduler);
    THROW_IF_FAILED(taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t()));

    // Get a pointer to the root task folder, which is where we'll register our new task.
    auto rootFolderPath = wil::make_bstr(L"\\");
    wil::com_ptr<ITaskFolder> rootFolder;
    THROW_IF_FAILED(taskService->GetFolder(rootFolderPath.get(), &rootFolder));

    // Create a task builder object to create the task
    wil::com_ptr<ITaskDefinition> task;
    THROW_IF_FAILED(taskService->NewTask(0, &task));

    // Start setting up the task
    SetRegisterationInfo(task.get());
    SetSettings(task.get());
    SetTriggers(task.get());
    SetAction(task.get());
    SaveTask(rootFolder.get(), task.get());

	return true;
}

bool UninstallTask()
{
    // Initialize COM, using wil::CoInitializeEx so that we don't need to worry about
    // cleaning up after ourselves.
    const auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    // Setup general COM security so that we're impersonating the current user.
    THROW_IF_FAILED(CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL));

    // Access the Windows Task Service API by creating an instance of it and attempt to connect
    // to the Task Scheduler service on the local machine.
    wil::com_ptr<ITaskService> taskService = wil::CoCreateInstance<ITaskService>(CLSID_TaskScheduler);
    THROW_IF_FAILED(taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t()));

    // Get a pointer to the root task folder, which is where we'll register our new task.
    auto rootFolderPath = wil::make_bstr(L"\\");
    wil::com_ptr<ITaskFolder> rootFolder;
    THROW_IF_FAILED(taskService->GetFolder(rootFolderPath.get(), &rootFolder));

    auto taskName = wil::make_bstr(L"Transition Fixer");

    return SUCCEEDED(rootFolder->DeleteTask(taskName.get(), 0));
}