#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/linux.h"
#include "../inc/adss.h"

__int32_t
main (
    __int32_t ArgumentCount,
    char* Arguments[]
    )
{
    ADSS_BUS_CLIENT_CONNECT_SERVER_PARAMETERS connectMsg;
    __int32_t lxBusFd, error;
    __int8_t verboseMode;
    size_t size;

    //
    // Print banner and usage information
    //
    printf("LxClient v1.0.0 -- (c) Copyright 2016 Alex Ionescu\n");
    printf("Visit http://github.com/ionescu007/lxss for more information.\n\n");
    if ((ArgumentCount < 2) || (strcmp(Arguments[1], "-h") == 0))
    {
        printf("lxclient <windows path to executable> [-v] [-h]\n");
        printf("Options:\n");
        printf("  -v    Enable verbose mode\n");
        printf("  -h    This cruft\n");
        printf("Launches the Win32 binary specified in the path. "
               "See man page for examples. Must run as root!\n");
        return EINVAL;
    }

    //
    // Check if verbosity was requested
    //
    verboseMode = 0;
    if ((ArgumentCount == 3) && (strcmp(Arguments[2], "-v") == 0))
    {
        verboseMode = 1;
    }

    //
    // Open a handle to the /dev/lxss virtual device inode. The user must be
    // running as root and the ADSS Bus Access registry key must have been set.
    //
    if (verboseMode) printf("Opening handle to ADSS Bus...\n");
    lxBusFd = open("/dev/lxss", O_DIRECT | O_RDWR);
    if ((lxBusFd == 0) || (lxBusFd == -1))
    {
        if (errno == EACCES)
        {
            printf("ERROR: lxclient must be run as root\n");
        }
        else if (errno == EPERM)
        {
            printf("ERROR: Root Adss Bus Access not enabled\n");
        }
        else
        {
            printf("Error connecting to ADSS bus: %d\n", errno);
        }
        return errno;
    }

    //
    // Connect to the "lxserver" server port
    //
    if (verboseMode) printf("Connecting to lxserver...\n");
    connectMsg.Timeout = 0xFFFFFFFF;
    connectMsg.ServerName = "lxserver";
    connectMsg.Flags = 1;
    error = ioctl(lxBusFd, IOCTL_ADSS_BUS_CLIENT_CONNECT_SERVER, &connectMsg);
    if (error != 0)
    {
        printf("Error connecting to lxserver: %d\n", errno);
        return errno;
    }

    //
    // Print fd on success
    //
    if (verboseMode)
    {
        printf("Server connection established with File Descriptor: %d\n",
               connectMsg.ServerHandle);
        printf("Setting File Descriptor to O_WRONLY...\n");
    }

    //
    // Set the correct file descriptor mode for access
    //
    error = fcntl(connectMsg.ServerHandle, F_SETFD, O_WRONLY);
    if (error != 0)
    {
        printf("Error setting F_SETFD(O_WRONLY): %d\n", errno);
        return errno;
    }

    //
    // Now send the actual message
    //
    if (verboseMode) printf("Sending message to server...\n");
    size = write(connectMsg.ServerHandle, Arguments[1], strlen(Arguments[1]));
    if (verboseMode) printf("Sent %d bytes\n", size);

    //
    // All done
    //
    return 0;
}
