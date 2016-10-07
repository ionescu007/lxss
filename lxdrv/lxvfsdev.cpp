#include "precomp.h"

//
// The standard VFS Entries for Bash On Windows
//
extern "C" VFS_ENTRY LxssStandardEntries[9];
extern "C" VFS_ENTRY VfsMyEntries[1];

//
// Custom VFS File Context Structure for our Device
//
typedef struct _MY_FILE_CONTEXT
{
    PVFS_MINOR_DEVICE AssociatedDevice;
} MY_FILE_CONTEXT, *PMY_FILE_CONTEXT;

//
// VFS File Callbacks for File Contexts on our Device
//
VFS_FILE_IOCTL_CALLBACK VfsMyIoctl;
VFS_FILE_CALLBACKS VfsMyFileCallbacks =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    VfsMyIoctl,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

//
// VFS Minor Device Callbacks for our Device
//
VFS_MINOR_DEVICE_OPEN_CALLBACK VfsMyOpen;
VFS_MINOR_DEVICE_UNINITIALIZE_CALLBACK VfsMyUninitialize;
VFS_MINOR_DEVICE_CALLBACKS VfsMyCallbacks =
{
    VfsMyOpen,
    VfsMyUninitialize
};

//
// VFS Minor Device for our Device
//
PVFS_MINOR_DEVICE VfsMyDevice;

INT
VfsMyIoctl (
    _In_ PVOID File,
    _In_ ULONG Ioctl,
    _Inout_ PVOID Buffer
    )
{
    NTSTATUS status;
    PMY_FILE_CONTEXT fileContext;

    //
    // Make sure this is really our file
    //
    fileContext = reinterpret_cast<PMY_FILE_CONTEXT>(File);
    NT_ASSERT(fileContext->AssociatedDevice == VfsMyDevice);

    //
    // Ignore unknown codes
    //
    if (Ioctl != 0xBEEF)
    {
        status = STATUS_INVALID_PARAMETER;
        goto Quickie;
    }

    //
    // Make sure the user address is valid
    //
    __try
    {
        //
        // Aligned at 4 bytes and in user-mode
        //
        ProbeForWrite(Buffer, sizeof(ULONG), sizeof(ULONG));

        //
        // Write our response
        //
        *(PULONG)Buffer = 42;
        status = STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        //
        // Catch exception
        //
        status = GetExceptionCode();
        goto Quickie;
    }

Quickie:
    //
    // Return the Linux error code
    //
    return g_LxpUtilTranslateStatus(status);
}

INT
VfsMyOpen (
    _In_ PVOID Context,
    _In_ PVFS_MINOR_DEVICE Device,
    _In_ ULONG Flags,
    _Out_ PVOID* File
)
{
    PMY_FILE_CONTEXT fileContext;
    NTSTATUS status;
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Flags);

    //
    // Allocate a file context
    //
    fileContext = static_cast<PMY_FILE_CONTEXT>(g_VfsFileAllocate(sizeof(*fileContext),
                                                                  &VfsMyFileCallbacks));

    if (fileContext == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Quickie;
    }

    //
    // Rememer which device we're on
    //
    fileContext->AssociatedDevice = Device;

    //
    // Return the file back to VFS
    //
    *File = fileContext;
    status = STATUS_SUCCESS;

Quickie:
    //
    // Return the Linux error code
    //
    return g_LxpUtilTranslateStatus(status);
}

VOID
VfsMyUninitialize (
    _In_ PVFS_MINOR_DEVICE Device
    )
{
    //
    // Instance is being torn-down. We have nothing to do.
    //
    NT_ASSERT(Device == VfsMyDevice);
    NOTHING;
}

INT
LxDrvMountInitialize (
    _In_ PLX_INSTANCE Instance
    )
{
    LONG error;

    //
    // Initialize the standard Lxss entries for WSL/BashOnWindows
    //
    C_ASSERT(RTL_NUMBER_OF(LxssStandardEntries) == 9);
    error = g_VfsInitializeStartupEntries(Instance,
                                          LxssStandardEntries, 
                                          RTL_NUMBER_OF(LxssStandardEntries));
    if (error != 0)
    {
        goto Quickie;
    }

    //
    // Allocate a minor device
    //
    VfsMyDevice = g_VfsDeviceMinorAllocate(&VfsMyCallbacks, 0x100);
    if (VfsMyDevice == NULL)
    {
        error = g_LxpUtilTranslateStatus(STATUS_INSUFFICIENT_RESOURCES);
        goto Quickie;
    }

    //
    // Register it under /dev/misc with a minor code of 0xBAD
    //
    g_LxpDevMiscRegister(Instance, VfsMyDevice, LXDRV_MINOR);

    //
    // Registration takes a reference, drop the allocation one
    //
    g_VfsDeviceMinorDereference(VfsMyDevice);

    //
    // Initialize the custom VFS entries
    //
    C_ASSERT(RTL_NUMBER_OF(VfsMyEntries) == 1);
    error = g_VfsInitializeStartupEntries(Instance,
                                          VfsMyEntries,
                                          RTL_NUMBER_OF(VfsMyEntries));
    if (error != 0)
    {
        goto Quickie;
    }

    //
    // Everything seems to have worked
    //
    error = 0;

Quickie:
    //
    // All done
    //
    return error;
}

