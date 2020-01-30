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

namespace {
    /// <summary>
    /// Provides a RAII type that will call CoInitialize() and its corresponding
    /// CoUinitialized() upon this class going out of scope.
    /// </summary>
    /// <remarks>
    /// This code was shamelessly stolen from 
    /// <a href="https://devblogs.microsoft.com/oldnewthing/?p=39243">The Old New Thing</a>
    /// by Raymond Chen
    /// </remarks>
    class CCoInitializeEx {
    public:
        CCoInitializeEx(DWORD flags) : m_hr{CoInitializeEx(NULL, flags)} { }
        ~CCoInitializeEx() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
        operator HRESULT() const { return m_hr; }
        HRESULT m_hr;
    };

    std::string GetCurrentDateTime()
    {
        std::stringstream stream;
        std::time_t currentTime = std::time(nullptr);
        std::tm currentTimeUtc = *std::gmtime(&currentTime);
        stream << std::put_time(&currentTimeUtc, "%FT%T");

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

    bool SetRegisterationInfo(ITaskDefinition* task)
    {
        CComPtr<IRegistrationInfo> registrationInfo;
        HRESULT result = task->get_RegistrationInfo(&registrationInfo);
        if (FAILED(result)) {
            std::cerr << "Failed to registration information: ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        std::string currentTime = GetCurrentDateTime();
        registrationInfo->put_Author(_bstr_t("Limotto Productions"));
        registrationInfo->put_Description(
            _bstr_t(
                "Fix an issue where the Windows desktop does not play a fade transition effect "
                "when changing wallpapers by enabling Active Desktop."));
        registrationInfo->put_Version(_bstr_t("1.0"));
        registrationInfo->put_Date(_bstr_t(currentTime.c_str()));

        return true;
    }

    bool SetSettings(ITaskDefinition* task)
    {
        CComPtr<ITaskSettings> settings;
        HRESULT result = task->get_Settings(&settings);
        if (FAILED(result)) {
            std::cerr << "Failed to get task settings: ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        settings->put_StartWhenAvailable(VARIANT_TRUE);

        return true;
    }

    bool SetTriggers(ITaskDefinition* task)
    {
        CComPtr<ITriggerCollection> triggerCollection;
        HRESULT result = task->get_Triggers(&triggerCollection);
        if (FAILED(result)) {
            std::cerr << "Failed to get task's triggers: ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        // Add the login trigger to the task's triggers
        CComPtr<ITrigger> trigger;
        result = triggerCollection->Create(TASK_TRIGGER_LOGON, &trigger);
        if (FAILED(result)) {
            std::cerr << "Failed to create trigger for this task: ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        CComPtr<ILogonTrigger> logonTrigger;
        result = trigger->QueryInterface(&logonTrigger);
        if (FAILED(result)) {
            std::cerr << "Failed to get logon trigger from newly created trigger for this task: ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        std::wstring userID = GetUserID();
        logonTrigger->put_UserId(_bstr_t(userID.c_str()));
        logonTrigger->put_Delay(_bstr_t("PT30S")); // Put a 30s delay, in case Explorer hasn't initialized immediately.

        return true;
    }

    bool SetAction(ITaskDefinition* task)
    {
        CComPtr<IActionCollection> actionCollection;
        HRESULT result = task->get_Actions(&actionCollection);
        if (FAILED(result)) {
            std::cerr << "Failed to get task's actions ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        // Add the executable action to the task's actions
        CComPtr<IAction> action;
        result = actionCollection->Create(TASK_ACTION_EXEC, &action);
        if (FAILED(result)) {
            std::cerr << "Failed to get task's actions ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        CComPtr<IExecAction> execAction;
        result = action->QueryInterface(&execAction);
        if (FAILED(result)) {
            std::cerr << "Failed to get executable action from newly-created action: ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        std::wstring execPath = GetExePath();
        std::wstring execPathWithoutName = execPath.substr(0, execPath.find_last_of('\\'));
        execAction->put_Path(_bstr_t(execPath.c_str()));
        execAction->put_WorkingDirectory(_bstr_t(execPathWithoutName.c_str()));
        execAction->put_Arguments(_bstr_t("run"));

        return true;
    }

    bool SaveTask(ITaskFolder* rootFolder, ITaskDefinition* task)
    {
        std::wstring currentUser = GetUserID();
        CComPtr<IRegisteredTask> registeredTask;
        HRESULT result = rootFolder->RegisterTaskDefinition(
            _bstr_t("Transition Fixer"),
            task,
            TASK_CREATE_OR_UPDATE,
            _variant_t(currentUser.c_str()),
            _variant_t(),
            TASK_LOGON_NONE,
            _variant_t(L""),
            &registeredTask
        );
        if (FAILED(result)) {
            std::cerr << "Failed to register task: ";
            std::wcerr << GetWin32Error(result);

            return false;
        }

        return true;
    }
}

bool InstallTask()
{
    // Initialize COM, using CCoInitializeEx so that we don't need to worry about
    // cleaning up after ourselves.
    CCoInitializeEx comInit{COINIT_MULTITHREADED};
    if (FAILED(comInit)) {
        std::cerr << "Failed to initialize COM: ";
        std::wcerr << GetWin32Error(comInit);

        return false;
    }

    // Setup general COM security so that we're impersonating the current user.
    HRESULT result = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);
    if (FAILED(result)) {
        std::cerr << "Failed to initialize COM security: ";
        std::wcerr << GetWin32Error(result);

        return false;
    }

    // Access the Windows Task Service API by creating an instance of it and attempt to connect
    // to the Task Scheduler service on the local machine.
    CComPtr<ITaskService> taskService;
    result = taskService.CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED(result)) {
        std::cerr << "Failed to create an instance of the ITaskService:";
        std::wcerr << GetWin32Error(result);

        return false;
    }

    result = taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(result)) {
        std::cerr << "Failed to connect to the Task Scheduler service of the local machine: ";
        std::wcerr << GetWin32Error(result);

        return false;
    }

    // Get a pointer to the root task folder, which is where we'll register our new task.
    CComPtr<ITaskFolder> rootFolder;
    result = taskService->GetFolder(_bstr_t(L"\\"), &rootFolder);
    if (FAILED(result)) {
        std::cerr << "Failed to get root folder of Task Scheduler: ";
        std::wcerr << GetWin32Error(result);

        return false;
    }

    // Create a task builder object to create the task
    CComPtr<ITaskDefinition> task;
    result = taskService->NewTask(0, &task);
    if (FAILED(result)) {
        std::cerr << "Failed to create task definition: ";
        std::wcerr << GetWin32Error(result);
    }

    // Start setting up the task
    if (!SetRegisterationInfo(task)) {
        return false;
    }
    if (!SetSettings(task)) {
        return false;
    }
    if (!SetTriggers(task)) {
        return false;
    }
    if (!SetAction(task)) {
        return false;
    }

    // Register the task
    if (!SaveTask(rootFolder, task)) {
        return false;
    }

	return true;
}

bool UninstallTask()
{
	return true;
}