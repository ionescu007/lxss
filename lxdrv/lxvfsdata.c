#include "precomp.h"

VFS_ENTRY VfsMyEntries[1] =
{
    {
        VfsNodeEntry,
        RTL_CONSTANT_STRING(L"/dev/lxdrv"),
        .Node =
        {
            0, 0,
            S_IFCHR | S_IWUSR | S_IRUSR | S_IROTH | S_IWOTH,
            LXDRV_MINOR,
            MISC_MAJOR
        }
    },
};

VFS_ENTRY LxssStandardEntries[9] =
{
    {
        VfsNodeEntry,
        RTL_CONSTANT_STRING(L"/dev/kmsg"),
        .Node =
        {
            0, 0,
            S_IFCHR | S_IWUSR | S_IRUSR,
            KMSG_MINOR,
            MEM_MAJOR
        }
    },
    {
        VfsDirectoryEntry,
        RTL_CONSTANT_STRING(L"/run"),
        .Directory =
        {
            0, 0,
            S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH,
        }
    },
    {
        VfsMountEntry,
        RTL_CONSTANT_STRING(L"/run"),
        .Mount =
        {
            RTL_CONSTANT_STRING("tmpfs"),
            {0, 0, NULL},
            RTL_CONSTANT_STRING("mode=755"),
            0, 0, 0,
            S_ISGID | S_IXGRP | S_IWOTH,
            0
        }
    },
    {
        VfsDirectoryEntry,
        RTL_CONSTANT_STRING(L"/run/lock"),
        .Directory =
        {
            0, 0,
            S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH,
        }
    },
    {
        VfsMountEntry,
        RTL_CONSTANT_STRING(L"/run/lock"),
        .Mount =
        {
            RTL_CONSTANT_STRING("tmpfs"),
            {0, 0, NULL},
            {0, 0, NULL},
            0, 0, 0,
            S_ISGID | S_IXGRP | S_IROTH | S_IWOTH,
            0
        }
    },
    {
        VfsDirectoryEntry,
        RTL_CONSTANT_STRING(L"/run/shm"),
        .Directory =
        {
            0, 0,
            S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH,
        }
    },
    {
        VfsMountEntry,
        RTL_CONSTANT_STRING(L"/run/shm"),
        .Mount =
        {
            RTL_CONSTANT_STRING("tmpfs"),
            {0, 0, NULL},
            {0, 0, NULL},
            0, 0, 0,
            S_ISGID | S_IROTH | S_IWOTH,
            0
        }
    },
    {
        VfsDirectoryEntry,
        RTL_CONSTANT_STRING(L"/run/user"),
        .Directory =
        {
            0, 0,
            S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH,
        }
    },
    {
        VfsMountEntry,
        RTL_CONSTANT_STRING(L"/run/user"),
        .Mount =
        {
            RTL_CONSTANT_STRING("tmpfs"),
            {0, 0, NULL},
            RTL_CONSTANT_STRING("mode=755"),
            0, 0, 0,
            S_ISGID | S_IXGRP | S_IROTH | S_IWOTH,
            0
        }
    },
};

