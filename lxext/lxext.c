#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/linux.h"
#include "../inc/adss.h"

typeof(open)* realOpen;

void*
ServerRoutine (
    void* Context
    )
{
    struct sockaddr_un addr;
    __int32_t lxBusFd, socketFd, clientFd, error;
    ADSS_BUS_CLIENT_CONNECT_SERVER_MSG connectMsg;
    size_t size;
    __uint8_t readBuffer[260];

    //
    // Get the ADSS file descriptor
    //
    lxBusFd = (__int32_t)(__int64_t)Context;

    //
    // Create a Unix domain socket
    //
    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd < 0)
    {
        return NULL;
    }

    //
    // Unlink the socket if it already exists, so that we can recreate it
    //
    unlink("/tmp/lxexec-socket");

    //
    // Bind it to the path that the lxexec binary looks for
    //
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/tmp/lxexec-socket", sizeof(addr.sun_path) - 1);
    error = bind(socketFd, (struct sockaddr*)&addr, sizeof(addr));
    if (error == -1)
    {
        return NULL;
    }

    //
    // Make the socket RW by everyone
    //
    error = fchmod(socketFd, 777);
    if (error == -1)
    {
        return NULL;
    }

    //
    // Listen for new connections, with a backlog of 1
    //
    error = listen(socketFd, 1);
    if (error == -1)
    {
        return NULL;
    }

    //
    // Sit in a loop accepting connections
    //
    while (1)
    {
        //
        // Accept the incoming connection
        //
        clientFd = accept(socketFd, NULL, NULL);
        if (clientFd == -1)
        {
            continue;
        }

        //
        // Read the input
        //
        memset(readBuffer, 0, sizeof(readBuffer));
        size = read(clientFd, readBuffer, sizeof(readBuffer));
        if (size <= 0)
        {
            close(clientFd);
            continue;
        }

        //
        // Connect to the "lxserver" server port
        //
        connectMsg.Timeout = 0xFFFFFFFF;
        connectMsg.ServerName = "lxserver";
        connectMsg.Flags = ADSS_CONNECT_WAIT_FOR_SERVER_FLAG;
        error = ioctl(lxBusFd, IOCTL_ADSS_BUS_CLIENT_CONNECT_SERVER, &connectMsg);
        if (error != 0)
        {
            close(clientFd);
            continue;
        }

        //
        // Set the correct file descriptor mode for access
        //
        error = fcntl(connectMsg.ServerHandle, F_SETFD, O_WRONLY);
        if (error != 0)
        {
            close(connectMsg.ServerHandle);
            close(clientFd);
            continue;
        }

        //
        // Now send the actual message
        //
        write(connectMsg.ServerHandle, readBuffer, size);
        close(connectMsg.ServerHandle);
        close(clientFd);
    }
}

__int32_t
open (
    const char* name,
    __int32_t flags
    )
{
    __int32_t fd;
    pthread_t new_thread;

    //
    // Get the real "open" symbol
    //
    if (realOpen == NULL)
    {
        realOpen = (typeof(open)*)dlsym(RTLD_NEXT, "open");
    }

    //
    // Call the real function
    //
    fd = realOpen(name, flags);

    //
    // Check if this is the init daemon opening /dev/lxss
    //
    if ((getpid() == 1) && (strcmp(name, "/dev/lxss") == 0))
    {
        //
        // Inject a thread in init waiting for connections to our socket
        //
        pthread_create(&new_thread, NULL, ServerRoutine, (void*)(__int64_t)fd);
        pthread_detach(new_thread);
    }

    //
    // Return the file descriptor
    //
    return fd;
}
