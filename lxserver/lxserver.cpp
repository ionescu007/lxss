#include <initguid.h>
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <comutil.h>
#include "..\inc\nt.h"
#include "..\inc\lxssmanager.h"
#include "..\inc\adss.h"

HANDLE CaptureImpersonationToken();

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
    BOOLEAN verboseMode, tokenMode, extensionMode;
    ULONG i;
    BOOL bRes;
    HANDLE hToken;
    ADSS_IPC_CONNECTION_MARSHAL_FORK_TOKEN_MSG tokenMsg;
    HANDLE clientHandle;
    UNREFERENCED_PARAMETER(Arguments);

    //
    // Print banner and help if we got invalid arguments
    //
    wprintf(L"LxServer v1.1.5 -- (c) Copyright 2016 Alex Ionescu\n");
    wprintf(L"Visit http://github.com/ionescu007/lxss for more information.\n\n");

    //
    // Check argument settings
    //
    verboseMode = tokenMode = extensionMode = 0;
    for (i = 0; i < ArgumentCount; i++)
    {
        if (strcmp(Arguments[i], "-v") == 0)
        {
            verboseMode = 1;
        }
        else if (strcmp(Arguments[i], "-h") == 0)
        {
            wprintf(L"USAGE: LxServer [-v | -t | -e]\n");
            wprintf(L"-e    Extension mode (don't enable ADSS Bus access)\n");
            wprintf(L"-t    Token donation mode (send a token for fork())\n");
            wprintf(L"-v    Verbose mode\n");
            wprintf(L"-h    Display help\n");
            return -1;
        }
        else if (strcmp(Arguments[i], "-t") == 0)
        {
            tokenMode = 1;
        }
        else if (strcmp(Arguments[i], "-e") == 0)
        {
            extensionMode = 1;
        }
    }

    //
    // Check if extension mode is NOT used (i..e: standalone client on Lx side)
    //
    if (extensionMode == 0)
    {
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
                             L"SYSTEM\\CurrentControlSet\\Services\\lxss",
                             NULL,
                             NULL,
                             0,
                             KEY_ALL_ACCESS,
                             NULL,
                             &lxKey,
                             &disposition);
        if (!(SUCCEEDED(hr)) || (disposition == REG_CREATED_NEW_KEY))
        {
            wprintf(L"Failed to access LXSS key. "
                    L"Are you running as Admin? Is LXSS installed?\n");
            return hr;
        }

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
            //
            // Enable it -- a reboot will be required
            //
            wprintf(L"Root ADSS Bus Access is disabled."
                    L"It will now be enabled -- please reboot.\n");
            accessAllowed = 1;
            hr = RegSetValueExW(lxKey,
                                L"RootAdssbusAccess",
                                0,
                                type,
                                (LPBYTE)&accessAllowed,
                                accessSize);
            return hr;
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
                              RPC_C_AUTHN_LEVEL_CONNECT,
                              SecurityDelegation,
                              NULL,
                              EOAC_DYNAMIC_CLOAKING,
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
        clientHandle = ULongToHandle(waitForConnectionMsg.ClientHandle);
        if (verboseMode)
        {
            wprintf(L"Connected to client on port handle 0x%p\n",
                    clientHandle);
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
            hr = NtReadFile(clientHandle,
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

            //
            // Check if the client has disconnected by now
            //
            if (hr == STATUS_CONNECTION_DISCONNECTED)
            {
                //
                // The client disconnected -- go and wait for a new one
                //
                if (verboseMode) wprintf(L"lxss client has disconnected\n");
                CloseHandle(clientHandle);
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
                // Check if token mode is active and we received a token request
                //
                if ((tokenMode == 1) && (readBuffer[0] == '\0'))
                {
                    hToken = CaptureImpersonationToken();
                    if (hToken == NULL)
                    {
                        wprintf(L"Error opening thread token: 0x%08lX\n",
                                GetLastError());
                        CloseHandle(clientHandle);
                        continue;
                    }

                    //
                    // Marshal it
                    //
                    tokenMsg.TokenHandle = HandleToUlong(hToken);
                    bRes = DeviceIoControl(clientHandle,
                                           IOCTL_ADSS_IPC_CONNECTION_MARSHAL_FORK_TOKEN,
                                           &tokenMsg,
                                           sizeof(tokenMsg),
                                           &tokenMsg,
                                           sizeof(tokenMsg),
                                           NULL,
                                           NULL);
                    if (bRes == FALSE)
                    {
                        CloseHandle(hToken);
                        wprintf(L"Error marshalling thread token: 0x%08lX\n",
                                GetLastError());
                        continue;
                    }

                    //
                    // Write the ID such that the LX side can unmarshal it
                    //
                    hr = NtWriteFile(clientHandle,
                                     hEvent,
                                     NULL,
                                     NULL,
                                     &ioStatusBlock,
                                     &tokenMsg.TokenId,
                                     sizeof(tokenMsg.TokenId),
                                     &byteOffset,
                                     NULL);
                    if (hr == STATUS_PENDING)
                    {
                        //
                        // Wait for one
                        //
                        WaitForSingleObject(hEvent, INFINITE);
                        hr = ioStatusBlock.Status;
                    }

                    //
                    // Close the handle
                    //
                    CloseHandle(hToken);
                }
                else
                {
                    //
                    // Launch whatever the client sent us
                    //
                    if (verboseMode) wprintf(L"Launching %S...\n", readBuffer);
                    WinExec(readBuffer, SW_SHOWDEFAULT);
                }
            }
            else
            {
                wprintf(L"Unexpected read error: 0x%08lX\n", hr);
                break;
            }
        }
    }

    //
    // We should never get here
    //
    return 0;
}

