#pragma once

#include <winternl.h>

static const PULONG g_BuildNumber = (PULONG)0x7ffe0260;

extern "C"
NTSTATUS
NtWriteFile (
    _In_ HANDLE FileHandle,
    _In_ HANDLE Event,
    _In_ PVOID ApcRoutine,
    _In_ PVOID ApcContext,
    _In_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ PVOID Buffer,
    _In_ ULONG Length, 
    _In_opt_ PLARGE_INTEGER ByteOffset,
    _In_opt_ PULONG Key
    );

extern "C"
NTSTATUS
NtReadFile (
    _In_ HANDLE FileHandle,
    _In_ HANDLE Event,
    _In_ PVOID ApcRoutine,
    _In_ PVOID ApcContext,
    _In_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ PVOID Buffer,
    _In_ ULONG Length,
    _In_opt_ PLARGE_INTEGER ByteOffset,
    _In_opt_ PULONG Key
    );

