#pragma once

EXTERN_C_START

//
// Undocumented Kernel (NT) APIs
//
NTKERNELAPI
PVOID
NTAPI
RtlFindExportedRoutineByName (
    _In_ PVOID ImageBase,
    _In_ PCCH RoutineNam
);

NTKERNELAPI
NTSTATUS
NTAPI
MmLoadSystemImage (
    _In_ PUNICODE_STRING ImageFileName,
    _In_ PUNICODE_STRING NamePrefix OPTIONAL,
    _In_ PUNICODE_STRING LoadedBaseName OPTIONAL,
    _In_ ULONG LoadFlags,
    _Out_ PVOID *Section,
    _Out_ PVOID *ImageBaseAddress
);

NTKERNELAPI
VOID
NTAPI
MmUnloadSystemImage (
    _In_ PVOID Section
);

EXTERN_C_END
