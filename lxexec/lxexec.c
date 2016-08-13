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
    __int32_t error, socketFd;
    __int8_t verboseMode;
    size_t size;
    struct sockaddr_un addr;

    //
    // Print banner and usage information
    //
    printf("LxExec v1.0.0 -- (c) Copyright 2016 Alex Ionescu\n");
    printf("Visit http://github.com/ionescu007/lxss for more information.\n\n");
    if ((ArgumentCount < 2) || (strcmp(Arguments[1], "-h") == 0))
    {
        printf("lxexec <windows path to executable> [-v] [-h]\n");
        printf("Options:\n");
        printf("  -v    Enable verbose mode\n");
        printf("  -h    This cruft\n");
        printf("Launches the Win32 binary specified in the path. "
               "See man page for examples.\n");
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
    // Create a Unix Domain Socket
    //
    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == -1)
    {
        perror("ERROR: Could not create UNIX socket");
        exit(-1);
    }

    //
    // Connect it to the /tmp/lxexec-socket that initex.so creates
    //
    if (verboseMode) printf("Created UNIX socket 0x%08lX\n", socketFd);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/tmp/lxexec-socket", sizeof(addr.sun_path) - 1);
    error = connect(socketFd, (struct sockaddr*)&addr, sizeof(addr));
    if (error == -1)
    {
        perror("ERROR: Could not connect to lxexec-socket. Is lxexec.so loaded?");
        exit(-1);
    }

    //
    // Write the path to the Win32 binary we want to execute
    //
    if (verboseMode) printf("Connected UNIX socket!\n");
    size = write(socketFd, Arguments[1], strlen(Arguments[1]));
    if (size != strlen(Arguments[1]))
    {
        perror("ERROR: Could not write path to domain socket\n");
        exit(-1);
    }

    //
    // All done
    //
    if (verboseMode) printf("Sent 0x%08lX bytes succesfully\n", size);
    return 0;
}
