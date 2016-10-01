# lxss
Fun with the Windows Subsystem for Linux (WSL/LXSS)

![Screenshot](lxclient.PNG)
![Screenshot](lxlaunch.PNG)

## LxLaunch
LxLaunch is a simple launcher which either launches /usr/bin/python if no parameters are given to it, or launches the given ELF executable if an argument is entered. The binary launches as a child of the init daemon, under the current/default Lx instance. Additional command-line arguments can be entered for the ELF process, and further command-line arguments can be nested in quotes if the ELF binary launches its own sub-binary with additional command-line options. For example:
```lxlaunch.exe /bin/bash -c "ls -al"```

## LxServer
LxServer is a simple Win32 (x64) server that waits for connections over the ADSS Bus from a compatible LXSS client (such as lxclient), and launches (WinExec) any input received as long as it fits within MAX_PATH. If needed, it configures the registry to enable ADSS Bus usage from root LXSS binaries other than the init daemon (see presentation slides), after which an initial reboot will be required. Launch it with -v for verbose information.

## LxClient
LxClient is a simple Linux (ELF) client that connects over the ADSS Bus to a compatible LXSS server (such as lxserver), and sends whatever the input command line argument is (which is expected to be a Win32 valid path to an executable, or other input that WinExec accepts) to the server. Launch it with -v following the Win32 path, for verbose information.

## LxExt
LxExt is an ld.preload extension for the init daemon, which creates a UNIX Domain socket /tmp/lxexec-socket and waits for connections. Once data is received, it passes it straight through the lxserver ADSS server on the Win32 side. This removes the need to have the Registry key set for ADSS Bus Access.

Note that due to bugs in the Linux 1.0 support for Visual Studio, G++ seems to fail to link the .so, and ignores C++ standard setttings such that typeof() does not work. You should compile this in the LX environment directly with the following command-line:
```gcc lxext.c -ldl -lpthread -shared -fPIC -Wall -Wno-unknown-pragmas -o ~/lxext.so```

Finally, you will need add the following line into ```/etc/ld.so.preload``` to allow lxext.so to load:
``` /home/<username>/lxext.so``` making sure to replace <username> with your LX user name and running the editor as root.

## LxExec
LXExec is a client for LxExt. It connects to the UNIX Domain Socket and sends the input command line to it. This avoids needing to run as root, such as lxclient before. Simply run it as ```lxexec calc``` as an example, making sure that lxserver is running on the Win32 side.



LsExec is a client for LxExt. It connects to the UNIX Domain Socket and sends the input command line to it. This avoids needing to run as root, such as lxclient before.

## Lx
The Lx directory contains a number of WinDBG scripts to dump the current LX state. Copy them to your c:\lx directory, or create a directory symlink (mklink /d) to wherever you've copied them, then execute in WinDBG as follows: $$>< c:\lx\lx.wds
