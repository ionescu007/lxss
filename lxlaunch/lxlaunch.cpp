#include <initguid.h>
#include <Windows.h>
#include <comutil.h>
#include "..\inc\nt.h"
#include "..\inc\lxssmanager.h"
#include "..\inc\adss.h"

const CHAR* LxssDefaultEnvironmentStrings[4] =
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
    PCCH cmdLine;
    ULONG processHandle;

    //
    // Print banner and help if we got invalid arguments
    //
    wprintf(L"LxLaunch v1.0.0 -- (c) Copyright 2016 Alex Ionescu\n");
    wprintf(L"Visit http://github.com/ionescu007/lxss for more information.\n\n");
    if (ArgumentCount > 2)
    {
        wprintf(L"USAGE: LxLaunch [<path to ELF binary>]\n");
        wprintf(L"       Will launch /usr/bin/python if path not present\n");
        return -1;
    }

    if (ArgumentCount == 1)
    {
        cmdLine = "/usr/bin/python";
    }
    else
    {
        cmdLine = Arguments[1];
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
    // Standard handles will be console handles. Console will be 80x25 with an
    // input/control pipe and STDOUT as the output.
    //
    RtlZeroMemory(&stdHandles, sizeof(stdHandles));
    consoleData.Width = 80;
    consoleData.Height = 25;
    consoleData.InputHandle = HandleToUlong(inPipe);
    consoleData.ControlHandle = HandleToUlong(inPipe);
    consoleData.OutputHandle = HandleToUlong(GetStdHandle(STD_OUTPUT_HANDLE));

    //
    // Now launch the process
    //
    hr = (*iLxInstance)->CreateLxProcess(iLxInstance,
                                         cmdLine,
                                         1,
                                         &cmdLine,
                                         4,
                                         LxssDefaultEnvironmentStrings,
                                         currentDirectory,
                                         1,
                                         &stdHandles,
                                         &consoleData,
                                         0,
                                         &processHandle);
    if (!SUCCEEDED(hr))
    {
        wprintf(L"Failed to launch %S\n", cmdLine);
        return GetLastError();
    }

    //
    // Wait for Linux process to exit
    //
    WaitForSingleObject(UlongToHandle(processHandle), INFINITE);
    return 0;
}

