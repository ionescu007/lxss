#include "precomp.h"

EXTERN_C DRIVER_INITIALIZE DriverEntry;
EXTERN_C DRIVER_UNLOAD DriverUnload;
VFS_MOUNT_NAMESPACE_INITIALIZE_CALLBACK LxDrvMountInitialize;

NTSTATUS
LxDrvGetRoutines (
    VOID
);

VOID
DriverUnload (
    _In_ PDRIVER_OBJECT DriverObject
    )
{
    //
    // Nothing to do
    //
    UNREFERENCED_PARAMETER(DriverObject);
    return;
}

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status;
    LXP_SUBSYSTEM_INFORMATION subsystemInformation;
    UNREFERENCED_PARAMETER(RegistryPath);

    //
    // Begin by getting the LxCore Interface we need
    //
    status = LxDrvGetRoutines();
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    //
    // Define our unload routine
    //
    DriverObject->DriverUnload = DriverUnload;

    //
    // Register our own namespace initialization routine
    //
    subsystemInformation.MountNamespaceInitialize = LxDrvMountInitialize;
    status = g_LxInitialize(DriverObject, &subsystemInformation);
    if (status != STATUS_TOO_LATE)
    {
        return status;
    }

    //
    // We are ready for instance creation
    //
    return STATUS_SUCCESS;
}
