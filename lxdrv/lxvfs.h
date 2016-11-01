
#pragma warning(disable:4201)
#pragma warning(disable:4214)

EXTERN_C_START

//
// Minor Devices in Memory Major Class
//
#define KMSG_MINOR          11

//
// Minor Devices in Miscellanous Class
//
#define ADSS_MINOR          50
#define ADSS_CLIENT_MINOR   51
#define LXDRV_MINOR         0xBAD

//
// Major Devices
//
#define MEM_MAJOR           1
#define MISC_MAJOR          10

//
// File Types
//
#define S_IFMT   0170000
#define S_IFSOCK 0140000
#define S_IFLNK  0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

//
// Owner Rights
//
#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

//
// Group Rights
//
#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

//
// Others Rights
//
#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

//
// Opaque types
//
typedef struct _LX_INSTANCE* PLX_INSTANCE;
typedef struct _VFS_MINOR_DEVICE* PVFS_MINOR_DEVICE;

//
// Namespace Initialization Callback and Registration
//
typedef
INT
(VFS_MOUNT_NAMESPACE_INITIALIZE_CALLBACK) (
    _In_ PLX_INSTANCE Instance
    );
typedef VFS_MOUNT_NAMESPACE_INITIALIZE_CALLBACK *PVFS_MOUNT_NAMESPACE_INITIALIZE_CALLBACK;
typedef struct _LXP_SUBSYSTEM_INFORMATION
{
    PVFS_MOUNT_NAMESPACE_INITIALIZE_CALLBACK MountNamespaceInitialize;
} LXP_SUBSYSTEM_INFORMATION, *PLXP_SUBSYSTEM_INFORMATION;

//
// VFS Minor Device Callbacks
//
typedef
INT
(VFS_MINOR_DEVICE_OPEN_CALLBACK) (
    _In_ PVOID CallContext,
    _In_ PVFS_MINOR_DEVICE MinorDevice,
    _In_ ULONG OpenFlags,
    _Out_ PVOID* OpenedFile
    );
typedef VFS_MINOR_DEVICE_OPEN_CALLBACK *PVFS_MINOR_DEVICE_OPEN_CALLBACK;
typedef
VOID
(VFS_MINOR_DEVICE_UNINITIALIZE_CALLBACK) (
    _In_ PVFS_MINOR_DEVICE MinorDevice
    );
typedef VFS_MINOR_DEVICE_UNINITIALIZE_CALLBACK *PVFS_MINOR_DEVICE_UNINITIALIZE_CALLBACK;
typedef struct _VFS_MINOR_DEVICE_CALLBACKS
{
    PVFS_MINOR_DEVICE_OPEN_CALLBACK Open;
    PVFS_MINOR_DEVICE_UNINITIALIZE_CALLBACK Uninitialize;
} VFS_MINOR_DEVICE_CALLBACKS, *PVFS_MINOR_DEVICE_CALLBACKS;

//
// VFS File Callbacks
//
typedef
INT
(VFS_FILE_IOCTL_CALLBACK) (
    _In_ PVOID CallContext,
    _In_ PVOID FileContext,
    _In_ ULONG Ioctl,
    _Inout_ PVOID Buffer
    );
typedef VFS_FILE_IOCTL_CALLBACK *PVFS_FILE_IOCTL_CALLBACK;
typedef struct _VFS_FILE_CALLBACKS
{
    PVOID Delete;
    PVOID Read;
    PVOID ReadDir;
    PVOID Write;
    PVOID WriteVector;
    PVOID Map;
    PVOID MapManual;
    PVFS_FILE_IOCTL_CALLBACK Ioctl;
    PVOID Flush;
    PVOID Sync;
    PVOID Release;
    PVOID ReadVector;
    PVOID Truncate;
    PVOID Seek;
    PVOID FilterPollRegistration;
    PVOID FAllocate;
    PVOID GetPath;
} VFS_FILE_CALLBACKS, *PVFS_FILE_CALLBACKS;

//
// VFS Entry Structure and Type
//
typedef enum _VFS_ENTRY_TYPE
{
    VfsDirectoryEntry,  // VfsMakeDirectory
    VfsMountEntry,      // VfsMount
    VfsNodeEntry,       // VfsMakeNode
    VfsSymlinkEntry,    // VfsMakeSymbolicLink
    VfsFileEntry        // VfsOpenFile
} VFS_ENTRY_TYPE;
typedef struct _VFS_ENTRY
{
    VFS_ENTRY_TYPE Type;
    UNICODE_STRING Name;
    union
    {
        struct
        {
            ULONG Uid;
            ULONG Gid;
            ULONG Mode;
        } Directory;
        struct
        {
            ANSI_STRING FileSystem;
            UNICODE_STRING TargetDevice;
            ANSI_STRING Options;
            ULONG Uid;
            ULONG Gid;
            ULONG Mode;
            ULONGLONG MountFlags;
            ULONG Unknown;
        } Mount;
        struct
        {
            ULONG Uid;
            ULONG Gid;
            ULONG Mode;

            //
            // kdev_t
            //
            ULONG MinorDeviceId : 20; 
            ULONG MajorDeviceId : 12;
        } Node;
        struct
        {
            UNICODE_STRING TargetEntry;
        } Symlink;
        struct
        {
            ULONG Mode;
        } File;
    };
} VFS_ENTRY, *PVFS_ENTRY;

//
// LxCore Initialization APIs
//
NTKERNELAPI
NTSTATUS
NTAPI
LxInitialize (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PLXP_SUBSYSTEM_INFORMATION SubsystemInformation
    );

NTKERNELAPI
INT
NTAPI
VfsInitializeStartupEntries (
    _In_ PLX_INSTANCE Instance,
    _In_ PVFS_ENTRY Entries,
    _In_ ULONG EntryCount
    );

//
// LxCore VFS Minor Device APIs
//
NTKERNELAPI
PVFS_MINOR_DEVICE
NTAPI
VfsDeviceMinorAllocate (
    _In_ PVFS_MINOR_DEVICE_CALLBACKS MinorDeviceCallbacks,
    _In_ ULONG MinorDeviceContextSize
    );

NTKERNELAPI
VOID
NTAPI
VfsDeviceMinorDereference (
    _In_ PVFS_MINOR_DEVICE MinorDevice
    );

NTKERNELAPI
VOID
NTAPI
LxpDevMiscRegister (
    _In_ PLX_INSTANCE Instance,
    _In_ PVFS_MINOR_DEVICE MinorDevice,
    _In_ ULONG MinorDeviceNumber
    );

//
// LxCore VFS File APIs
//
NTKERNELAPI
PVOID
NTAPI
VfsFileAllocate (
    _In_ ULONG FileContextSize,
    _In_ PVFS_FILE_CALLBACKS FileCallbacks
    );

//
// LxCore Utility APIs
//
NTKERNELAPI
INT
NTAPI
LxpUtilTranslateStatus (
    _In_ NTSTATUS Status
    );

EXTERN_C_END

#ifdef __cplusplus
extern decltype(LxInitialize)*                      g_LxInitialize;
extern decltype(LxpDevMiscRegister)*                g_LxpDevMiscRegister;
extern decltype(VfsFileAllocate)*                   g_VfsFileAllocate;
extern decltype(VfsInitializeStartupEntries)*       g_VfsInitializeStartupEntries;
extern decltype(VfsDeviceMinorAllocate)*            g_VfsDeviceMinorAllocate;
extern decltype(VfsDeviceMinorDereference)*         g_VfsDeviceMinorDereference;
extern decltype(LxpUtilTranslateStatus)*            g_LxpUtilTranslateStatus;
#endif

