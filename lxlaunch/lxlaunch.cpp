#include <initguid.h>
#include <Windows.h>
#include <comutil.h>
#include "..\inc\nt.h"
#include "..\inc\lxssmanager.h"
#include "..\inc\adss.h"

#ifndef DISABLE_NEWLINE_AUTO_RETURN
#define DISABLE_NEWLINE_AUTO_RETURN 0x08
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x200
#endif

PCCH LxssDefaultEnvironmentStrings[4] =
{
    "HOSTTYPE=x86_64",
    "TERM=xterm",
    "LANG=en_US.UTF-8",
    "SHELL=/bin/bash"
};

INT
main (
    _In_ ULONG ArgumentCount,
    _In_ PCCH Arguments[]
    )
{
    HRESULT hr;
    PLX_SESSION* iLxSession;
    PLX_INSTANCE* iLxInstance;
    LXSS_CONSOLE_DATA consoleData;
    LXSS_STD_HANDLES stdHandles;
    HANDLE inPipe;
    PCHAR currentDirectory = "/";
    PCCH imageFileName;
    PCCH* cmdLine;
    ULONG cmdCount;
    ULONG processHandle, serverHandle;
    ULONG exitCode;
    HANDLE inHandle, outHandle;
    ULONG oldInMode, oldOutMode;
    UINT oldInCp, oldOutCp;
    CONSOLE_SCREEN_BUFFER_INFO screenInfo;

    //
    // Reset all variables
    //
    processHandle = NULL;
    serverHandle = NULL;
    inPipe = NULL;
    inHandle = NULL;
    outHandle = NULL;
    oldInCp = 0;
    oldOutCp = 0;
    oldInMode = 0;
    oldOutMode = 0;

    //
    // Print banner and help at all times
    //
    wprintf(L"LxLaunch v1.1.8 -- (c) Copyright 2016 Alex Ionescu\n");
    wprintf(L"Visit http://github.com/ionescu007/lxss for more information.\n\n");
    wprintf(L"USAGE: LxLaunch [<path to ELF binary>]\n");
    wprintf(L"       Will launch /usr/bin/python if path not present\n\n");

    //
    // If no arguments are passed in, just launch python as a proof-of-concept
    //
    if (ArgumentCount == 1)
    {
        imageFileName = "/usr/bin/python";
        cmdLine = &imageFileName;
        cmdCount = 1;
    }
    else
    {
        //
        // Otherwise, launch the image in argument 1, and treat all the other
        // arguments as input for this image
        //
        imageFileName = Arguments[1];
        cmdLine = &Arguments[1];
        cmdCount = ArgumentCount - 1;
    }

    //
    // Initialize COM runtime
    //
    hr = CoInitializeEx(NULL, 0);
    if (!SUCCEEDED(hr))
    {
        wprintf(L"Failed to initialize COM runtime\n");
        return hr;
    }

    //
    // Setup QoS for the ALPC/RPC endpoint
    //
    hr = CoInitializeSecurity(NULL,
                              -1,
                              NULL,
                              NULL,
                              RPC_C_AUTHN_LEVEL_DEFAULT,
                              SecurityDelegation,
                              NULL,
                              EOAC_STATIC_CLOAKING,
                              NULL);
    if (!SUCCEEDED(hr))
    {
        wprintf(L"Failed to initialize COM security\n");
        return hr;
    }

    //
    // Spin up lxss manager (ILxssSession)
    //
    hr = CoCreateInstance(lxGuid,
                          NULL,
                          CLSCTX_LOCAL_SERVER,
                          lxSessionGuid,
                          (PVOID*)&iLxSession);
    if (!SUCCEEDED(hr))
    {
        wprintf(L"Failed to initialize ILxssSession\n");
        return hr;
    }

    //
    // Start an instance (ILxssInstance).
    // If one is running, we'll get a pointer to it.
    //
    hr = (*iLxSession)->StartDefaultInstance(iLxSession,
                                             lxInstanceGuid,
                                             (PVOID*)&iLxInstance);
    if (!SUCCEEDED(hr))
    {
        wprintf(L"Failed to start LX Instance -- check Developer mode?\n");
        return hr;
    }

    //
    // Create the console in/control pipe (not used)
    //
    inPipe = CreateNamedPipe(L"\\\\.\\pipe\\InPipe",
                             FILE_FLAG_OVERLAPPED | PIPE_ACCESS_DUPLEX,
                             PIPE_REJECT_REMOTE_CLIENTS,
                             1,
                             4096,
                             4096,
                             0,
                             NULL);
    if (inPipe == NULL)
    {
        wprintf(L"Failed to create named pipe\n");
        return GetLastError();
    }

    //
    // Standard handles will be console handles
    //
    RtlZeroMemory(&stdHandles, sizeof(stdHandles));

    //
    // Get, and validate, the output handle
    //
    outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetFileType(outHandle) != FILE_TYPE_CHAR)
    {
        //
        // This means we got a non-console handle
        //
        assert(outHandle > 0);
        CloseHandle(outHandle);
        wprintf(L"Output handle is not console\n");
        exitCode = GetLastError();
        goto Quickie;
    }

    //
    // Get, and validate, the output handle
    //
    inHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (GetFileType(inHandle) != FILE_TYPE_CHAR)
    {
        assert(inHandle > 0);
        CloseHandle(inHandle);
        wprintf(L"Input handle is not console\n");
        exitCode = GetLastError();
        goto Quickie;
    }

    //
    // Switch input to UTF-8
    //
    oldInCp = GetConsoleCP();
    SetConsoleCP(CP_UTF8);

    //
    // Switch output to UTF-8
    //
    oldOutCp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);

    //
    // Switch to VT-100 Input Console
    //
    GetConsoleMode(inHandle, &oldInMode);
    SetConsoleMode(inHandle, oldInMode &
                             ~(ENABLE_INSERT_MODE |
                               ENABLE_ECHO_INPUT |
                               ENABLE_LINE_INPUT |
                               ENABLE_PROCESSED_INPUT) |
                             (ENABLE_VIRTUAL_TERMINAL_INPUT |
                              ENABLE_WINDOW_INPUT));

    //
    // Switch to VT-100 Output Console
    //
    GetConsoleMode(outHandle, &oldOutMode);
    SetConsoleMode(outHandle, oldOutMode |
                                DISABLE_NEWLINE_AUTO_RETURN |
                                ENABLE_VIRTUAL_TERMINAL_PROCESSING |
                                ENABLE_PROCESSED_OUTPUT);

    //
    // Set the console size to the current window size
    //
    GetConsoleScreenBufferInfo(outHandle, &screenInfo);
    consoleData.Width = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;
    consoleData.Height = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1;

    //
    // Set the handles
    //
    consoleData.InputHandle = HandleToUlong(inPipe);
    consoleData.ControlHandle = HandleToUlong(inPipe);
    consoleData.OutputHandle = HandleToUlong(GetStdHandle(STD_OUTPUT_HANDLE));

    //
    // Check if this is RS2
    //
    if (*g_BuildNumber >= 14500)
    {
        //
        // Use the new interface which now accepts an unnamed server IPC handle
        //
        hr = ((PLX_INSTANCE_V2)*iLxInstance)->CreateLxProcess(
            iLxInstance,
            imageFileName,
            cmdCount,
            cmdLine,
            RTL_NUMBER_OF(LxssDefaultEnvironmentStrings),
            LxssDefaultEnvironmentStrings,
            currentDirectory,
            LX_CREATE_PROCESS_PRINT_UPDATE_INFO_FLAG,
            &stdHandles,
            &consoleData,
            0,
            &processHandle,
            &serverHandle);
    }
    else
    {
        //
        // Use the old interface
        //
        hr = (*iLxInstance)->CreateLxProcess(
            iLxInstance,
            imageFileName,
            cmdCount,
            cmdLine,
            RTL_NUMBER_OF(LxssDefaultEnvironmentStrings),
            LxssDefaultEnvironmentStrings,
            currentDirectory,
            LX_CREATE_PROCESS_PRINT_UPDATE_INFO_FLAG,
            &stdHandles,
            &consoleData,
            0,
            &processHandle);
    }

    //
    // Check the result
    //
    if (!SUCCEEDED(hr))
    {
        wprintf(L"Failed to launch %S\n", imageFileName);
        exitCode = GetLastError();
        goto Quickie;
    }

    //
    // Wait for Linux process to exit
    //
    WaitForSingleObject(UlongToHandle(processHandle), INFINITE);
    GetExitCodeProcess(UlongToHandle(processHandle), &exitCode);

Quickie:
    //
    // Cleanup handles, restore console settings
    //
    if (inHandle != NULL)
    {
        SetConsoleCP(oldInCp);
        SetConsoleMode(inHandle, oldInMode);
    }
    if (outHandle != NULL)
    {
        SetConsoleOutputCP(oldOutCp);
        SetConsoleMode(outHandle, oldOutMode);
    }
    if (processHandle != NULL)
    {
        CloseHandle(UlongToHandle(processHandle));
    }
    if (serverHandle != NULL)
    {
        CloseHandle(UlongToHandle(serverHandle));
    }
    if (inPipe != NULL)
    {
        CloseHandle(inPipe);
    }
    return exitCode;
}

