#include <initguid.h>
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <comutil.h>
#include "..\inc\nt.h"
#include "..\inc\lxssmanager.h"
#include "..\inc\adss.h"

INT
main (
    _In_ ULONG ArgumentCount,
    _In_ PCCH Arguments[]
    )
{
    HRESULT hr;
    PLX_SESSION* iLxSession;
    PLX_INSTANCE* iLxInstance;
    ULONG serverHandle;
    ADSS_IPC_SERVER_WAIT_FOR_CONNECTION_MSG waitForConnectionMsg;
    HKEY lxKey;
    ULONG accessSize, accessAllowed, type, disposition;
    IO_STATUS_BLOCK ioStatusBlock;
    LARGE_INTEGER byteOffset;
    CHAR readBuffer[MAX_PATH];
    HANDLE hEvent;
    BOOLEAN verboseMode;
    UNREFERENCED_PARAMETER(Arguments);

    //
    // Print banner and help if we got invalid arguments
    //
    wprintf(L"LxServer v1.0.0 -- (c) Copyright 2016 Alex Ionescu\n");
    wprintf(L"Visit http://github.com/ionescu007/lxss for more information.\n\n");
    if (ArgumentCount > 1)
    {
        wprintf(L"USAGE: LxServer [-v]\n");
        wprintf(L"-v    Verbose mode\n");
        return -1;
    }

    //
    // Check if verbosity was requested
    //
    verboseMode = 0;
    if ((ArgumentCount == 2) && (strcmp(Arguments[1], "-v") == 0))
    {
        verboseMode = 1;
    }

    //
    // Configure parameters for registry value access
    //
    type = REG_DWORD;
    accessSize = sizeof(accessAllowed);
    accessAllowed = 0;

    //
    // Open, or create, the LXSS parameters key
    //
    hr = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                         L"SYSTEM\\CurrentControlSet\\Services\\lxss\\Parameters", 
                         NULL,
                         NULL,
                         0,
                         KEY_ALL_ACCESS,
                         NULL,
                         &lxKey,
                         &disposition);
    if (!SUCCEEDED(hr))
    {
        wprintf(L"Failed to access LXSS parameters key. "
                L"Are you running as Admin? Is LXSS installed?\n");
        return hr;
    }

    //
    // Check if a parameters key was already present
    //
    if (disposition == REG_OPENED_EXISTING_KEY)
    {
        //
        // Check its current value
        //
        hr = RegQueryValueExW(lxKey,
                              L"RootAdssbusAccess",
                              NULL,
                              &type,
                              (LPBYTE)&accessAllowed,
                              &accessSize);
        if ((accessAllowed == 0) || (hr == ERROR_NOT_FOUND))
        {
            wprintf(L"Root ADSS Bus Access is disabled."
                    L"It will now be enabled -- please reboot.\n");
        }
        else if (!SUCCEEDED(hr))
        {
            //
            // This should never happen
            //
            assert(SUCCEEDED(hr));
            return hr;
        }
    }

    //
    // Check if Root Bus Access must be enabled
    //
    if (accessAllowed == 0)
    {
        //
        // Enable it -- a reboot will be required
        //
        accessAllowed = 1;
        hr = RegSetValueExW(lxKey,
                            L"RootAdssbusAccess",
                            0,
                            type,
                            (LPBYTE)&accessAllowed,
                            accessSize);
        return hr;
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
    // Get the current running instance (ILxssInstance)
    //
    hr = (*iLxSession)->GetCurrentInstance(iLxSession,
                                           (PVOID*)&iLxInstance);
    if (!SUCCEEDED(hr) || (iLxInstance == NULL))
    {
        wprintf(L"Failed to get LX Instance -- make sure one is running?\n");
        return hr;
    }

    //
    // Now register an ADSS Bus Server
    //
    hr = (*iLxInstance)->RegisterAdssBusServer(iLxInstance,
                                               "lxserver",
                                               &serverHandle);
    if (!SUCCEEDED(hr))
    {
        wprintf(L"Failed to register 'lxserver'");
        return hr;
    }

    //
    // Sit in a loop waiting for connections
    //
    while (TRUE)
    {
        //
        // Wait for a connection...
        //
        if (verboseMode) wprintf(L"Waiting for client on the LXSS side...\n");
        waitForConnectionMsg.Timeout = INFINITE;
        hr = DeviceIoControl(ULongToHandle(serverHandle),
                             IOCTL_ADSS_IPC_SERVER_WAIT_FOR_CONNECTION,
                             &waitForConnectionMsg,
                             sizeof(waitForConnectionMsg),
                             &waitForConnectionMsg,
                             sizeof(waitForConnectionMsg),
                             NULL,
                             NULL);

        //
        // Handle possible error cases
        //
        hr = GetLastError();
        if (hr == ERROR_RETRY)
        {
            //
            // Timeout condition
            //
            wprintf(L"No connection after INFINITE seconds. "
                    L"Make sure LXSS client is running and try again!\n");
            return hr;
        }
        else if (!SUCCEEDED(hr))
        {
            //
            // This should never happen...
            //
            wprintf(L"Failed to send IOCTL to LX Driver?\n");
            assert(SUCCEEDED(hr));
            return hr;
        }

        //
        // Connection established!
        //
        if (verboseMode)
        {
            wprintf(L"Connected to client on port handle 0x%08lX\n",
                    waitForConnectionMsg.ClientHandle);
        }

        //
        // Initialize the read
        //
        byteOffset.QuadPart = 0;
        RtlZeroMemory(&ioStatusBlock, sizeof(ioStatusBlock));

        //
        // Build an event for it (reads are always sync)
        //
        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hEvent == NULL)
        {
            wprintf(L"Failed to create event. Must be out of memory.\n");
            return GetLastError();
        }

        //
        // Sit in a loop waiting for a client message
        //
        while (TRUE)
        {
            //
            // Read a client message
            //
            RtlZeroMemory(readBuffer, sizeof(readBuffer));
            hr = NtReadFile(ULongToHandle(waitForConnectionMsg.ClientHandle),
                            hEvent,
                            NULL,
                            NULL,
                            &ioStatusBlock,
                            &readBuffer,
                            sizeof(readBuffer),
                            &byteOffset,
                            NULL);

            //
            // The client may not have sent a message yet -- the ADSS Bus is
            // always asynchronous with respect to reads on the NT side.
            //
            if (hr == STATUS_PENDING)
            {
                //
                // Wait for one
                //
                WaitForSingleObject(hEvent, INFINITE);
                hr = ioStatusBlock.Status;
            }
            else if (hr == STATUS_CONNECTION_DISCONNECTED)
            {
                //
                // The client disconnected -- go and wait for a new one
                //
                if (verboseMode) wprintf(L"lxss client has disconnected\n");
                CloseHandle(ULongToHandle(waitForConnectionMsg.ClientHandle));
                CloseHandle(hEvent);
                break;
            }

            //
            // Check if the sender's buffer was too big
            //
            if ((hr == STATUS_BUFFER_TOO_SMALL) && (verboseMode))
            {
                wprintf(L"lxss client sent path > MAX_PATH. Ignoring.\n");
            }
            else if (SUCCEEDED(hr))
            {
                //
                // Launch whatever the client sent us
                //
                if (verboseMode) wprintf(L"Launching %S...\n", readBuffer);
                WinExec(readBuffer, SW_SHOWDEFAULT);
            }
            else
            {
                wprintf(L"Unexpected read error: 0x%08lX\n", hr);
            }
        }
    }

    //
    // We should never get here
    //
    return 0;
}

