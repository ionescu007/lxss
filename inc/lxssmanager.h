#pragma once

#include <initguid.h>
DEFINE_GUID(lxGuid, 0x4F476546, 0xB412, 0x4579, 0xB6, 0x4C, 0x12, 0x3D, 0xF3, 0x31, 0xE3, 0xD6);
DEFINE_GUID(lxSessionGuid, 0x536A6BCF, 0xFE04, 0x41D9, 0xB9, 0x78, 0xDC, 0xAC, 0xA9, 0xA9, 0xB5, 0xB9);
DEFINE_GUID(lxInstanceGuid, 0x8F9E8123, 0x58D4, 0x484A, 0xAC, 0x25, 0x7E, 0xF7, 0xD5, 0xF7, 0x44, 0x8F);

typedef HRESULT
(STDMETHODCALLTYPE *PLX_SESSION_GET_CURRENT_INSTANCE) (
    _In_ struct _LX_SESSION** This,
    _Out_ PVOID* InstanceOut
    );

typedef HRESULT
(STDMETHODCALLTYPE *PLX_SESSION_START_DEFAULT_INSTANCE) (
    _In_ struct _LX_SESSION** This,
    _In_ const IID& InstanceIid,
    _Out_ PVOID* InstanceOut
    );

typedef struct _LX_SESSION
{
    PVOID Self;
    PVOID AddRef;
    PVOID Release;
    PLX_SESSION_GET_CURRENT_INSTANCE GetCurrentInstance;
    PLX_SESSION_START_DEFAULT_INSTANCE StartDefaultInstance;
    PVOID SetState;
    PVOID QueryState;
    PVOID InitializeFileSystem;
    PVOID Destroy;
} LX_SESSION, *PLX_SESSION;

typedef struct _LXSS_STD_HANDLE
{
    ULONG Handle;
    BOOLEAN Pipe;
} LXSS_STD_HANDLE, *PLXSS_STD_HANDLE;

typedef struct _LXSS_STD_HANDLES
{
    LXSS_STD_HANDLE StdIn;
    LXSS_STD_HANDLE StdOut;
    LXSS_STD_HANDLE StdErr;
} LXSS_STD_HANDLES, *PLXSS_STD_HANDLES;

typedef struct _LXSS_CONSOLE_DATA
{
    ULONG InputHandle;
    ULONG OutputHandle;
    ULONG ControlHandle;
    USHORT Width;
    USHORT Height;
} LXSS_CONSOLE_DATA, *PLXSS_CONSOLE_DATA;

#define LX_CREATE_PROCESS_PRINT_UPDATE_INFO_FLAG        0x01
typedef HRESULT
(STDMETHODCALLTYPE *PLX_INSTANCE_CREATE_LX_PROCESS) (
    _In_ struct _LX_INSTANCE** This,
    _In_ PCCH CommandLine,
    _In_ ULONG ArgumentCount,
    _In_ PCCH* Arguments,
    _In_ ULONG EnvironmentCount,
    _In_ PCCH* Environment,
    _In_ PCCH CurrentDirectory,
    _In_ ULONG Flags,
    _In_ PLXSS_STD_HANDLES StdHandles,
    _In_ PLXSS_CONSOLE_DATA ConsoleData,
    _In_ ULONG Uid,
    _Out_ PULONG ProcessHandle
    );

typedef HRESULT
(STDMETHODCALLTYPE *PLX_INSTANCE_CREATE_LX_PROCESS_V2) (
    _In_ struct _LX_INSTANCE** This,
    _In_ PCCH CommandLine,
    _In_ ULONG ArgumentCount,
    _In_ PCCH* Arguments,
    _In_ ULONG EnvironmentCount,
    _In_ PCCH* Environment,
    _In_ PCCH CurrentDirectory,
    _In_ ULONG Flags,
    _In_ PLXSS_STD_HANDLES StdHandles,
    _In_ PLXSS_CONSOLE_DATA ConsoleData,
    _In_ ULONG Uid,
    _Out_ PULONG ProcessHandle,
    _Outptr_opt_ PULONG ServerHandle
    );

typedef HRESULT
(STDMETHODCALLTYPE *PLX_INSTANCE_REGISTER_ADSS_BUS_SERVER)(
    _In_ struct _LX_INSTANCE** This,
    _In_ PCCH ServerName,
    _Out_ PULONG ServerHandle
    );

typedef struct _LX_INSTANCE
{
    PVOID QueryInterface;
    PVOID AddRef;
    PVOID Release;
    PVOID GetConfiguration;
    PVOID GetId;
    PVOID QueryState;
    PVOID SetState;
    PLX_INSTANCE_CREATE_LX_PROCESS CreateLxProcess;
    PLX_INSTANCE_REGISTER_ADSS_BUS_SERVER RegisterAdssBusServer;
    PVOID ConnectAdssBusServer;
    PVOID Destroy;
    PVOID GetState;
    PVOID StartSelf;
    PVOID StopSelf;
    PVOID GetSuspendState;
} LX_INSTANCE, *PLX_INSTANCE;

typedef struct _LX_INSTANCE_V2
{
    PVOID QueryInterface;
    PVOID AddRef;
    PVOID Release;
    PVOID GetId;
    PVOID QueryState;
    PVOID SetState;
    PLX_INSTANCE_CREATE_LX_PROCESS_V2 CreateLxProcess;
    PLX_INSTANCE_REGISTER_ADSS_BUS_SERVER RegisterAdssBusServer;
    PVOID ConnectAdssBusServer;
    PVOID Destroy;
    PVOID GetState;
    PVOID Start;
    PVOID Stop;
    PVOID GetSuspendState;
    PVOID StopSelf;
} LX_INSTANCE_V2, *PLX_INSTANCE_V2;
