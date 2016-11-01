#include "precomp.h"

#define GetLxRoutineWithType(mod, func)     (decltype(##func)*)RtlFindExportedRoutineByName(mod, #func)
#define GetSystemRoutineWithType(func)      (decltype(##func)*)MmGetSystemRoutineAddress(&func##String)

UNICODE_STRING MmLoadSystemImageString      = RTL_CONSTANT_STRING(L"MmLoadSystemImage");
UNICODE_STRING MmUnloadSystemImageString    = RTL_CONSTANT_STRING(L"MmUnloadSystemImage");
UNICODE_STRING LxCoreDriverNameString       = RTL_CONSTANT_STRING(L"\\SystemRoot\\System32\\drivers\\lxcore.sys");

decltype(LxInitialize)*                     g_LxInitialize;
decltype(LxpDevMiscRegister)*               g_LxpDevMiscRegister;
decltype(VfsFileAllocate)*                  g_VfsFileAllocate;
decltype(VfsInitializeStartupEntries)*      g_VfsInitializeStartupEntries;
decltype(VfsDeviceMinorAllocate)*           g_VfsDeviceMinorAllocate;
decltype(VfsDeviceMinorDereference)*        g_VfsDeviceMinorDereference;
decltype(LxpUtilTranslateStatus)*           g_LxpUtilTranslateStatus;

NTSTATUS
LxDrvGetRoutines (
    VOID
    )
{
    PVOID lxcoreBase;
    NTSTATUS status;
    PVOID section;

    //
    // Get required NT OS Functions
    //
    auto mmLoadSystemImage = GetSystemRoutineWithType(MmLoadSystemImage);
    auto mmUnloadSystemImage = GetSystemRoutineWithType(MmUnloadSystemImage);
    if ((mmLoadSystemImage == NULL) || (mmUnloadSystemImage == NULL))
    {
        status = STATUS_NOT_FOUND;
        goto Quickie;
    }

    //
    // Find the LxCore.sys driver, without taking a reference on it
    //
    section = NULL;
    lxcoreBase = NULL;
    status = mmLoadSystemImage(&LxCoreDriverNameString,
                               NULL,
                               NULL,
                               0x80000000,
                               &section,
                               &lxcoreBase);
    if (status != STATUS_IMAGE_ALREADY_LOADED_AS_DLL)
    {
        //
        // We failed in some way, or ended up loading it
        //
        if (section != NULL)
        {
            //
            // We don't actually want to be the ones loading it
            //
            mmUnloadSystemImage(section);
        }

        goto Quickie;
    }

    //
    // Now get all of the routines we need
    //
    g_LxInitialize = GetLxRoutineWithType(lxcoreBase, LxInitialize);
    g_LxpDevMiscRegister = GetLxRoutineWithType(lxcoreBase, LxpDevMiscRegister);
    g_VfsFileAllocate = GetLxRoutineWithType(lxcoreBase, VfsFileAllocate);
    g_VfsInitializeStartupEntries = GetLxRoutineWithType(lxcoreBase, VfsInitializeStartupEntries);
    g_VfsDeviceMinorAllocate = GetLxRoutineWithType(lxcoreBase, VfsDeviceMinorAllocate);
    g_VfsDeviceMinorDereference = GetLxRoutineWithType(lxcoreBase, VfsDeviceMinorDereference);
    g_LxpUtilTranslateStatus = GetLxRoutineWithType(lxcoreBase, LxpUtilTranslateStatus);
    if ((g_LxInitialize == NULL) ||
        (g_LxpDevMiscRegister == NULL) ||
        (g_VfsFileAllocate == NULL) ||
        (g_VfsInitializeStartupEntries == NULL) ||
        (g_VfsDeviceMinorAllocate == NULL) ||
        (g_VfsDeviceMinorDereference == NULL) ||
        (g_LxpUtilTranslateStatus == NULL))
    {
        status = STATUS_NOT_FOUND;
        goto Quickie;
    }

    //
    // All routines found
    //
    status = STATUS_SUCCESS;

Quickie:
    //
    // Return lookup result
    //
    return status;
}
