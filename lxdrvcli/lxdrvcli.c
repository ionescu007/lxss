#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

__int32_t
main (
    __int32_t ArgumentCount,
    char* Arguments[]
    )
{
    __int32_t lxFd;
    __int32_t error;
    __int32_t testBuffer;

    //
    // Print banner and usage information
    //
    printf("LxDrvCli v1.0.0 -- (c) Copyright 2016 Alex Ionescu\n");
    printf("Visit http://github.com/ionescu007/lxss for more information.\n\n");
    if (ArgumentCount != 1)
    {
        printf("lxdrvcli\n");
        printf("Opens /dev/lxdrv and sends it the 0xBEEF IOCTL\n");
        return EINVAL;
    }

    //
    // Open our device
    //
    lxFd = open("/dev/lxdrv", O_RDWR);
    if (lxFd < 1)
    {
         printf("Couldn't open handle to lxdrv device: %s\n", strerror(errno));
         return errno;
    }
    else
    {
         printf("Opened handle: %d\n", lxFd);
    }

    //
    // Send it a random number, to prove it will override it
    //
    testBuffer = 50;
    error = ioctl(lxFd, 0xBEEF, &testBuffer);
    if (error != 0)
    {
         printf("IOCTL failed: %d %s\n", error, strerror(errno));
         close(lxFd);
         return errno;
    }

    //
    // Show the output and close the file
    //
    printf("Response from driver: %d\n", testBuffer);
    close(lxFd);
    return 0;
}
