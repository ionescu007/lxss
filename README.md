# lxss
Fun with the Windows Subsystem for Linux (WSL/LXSS)

![Screenshot](lxclient.PNG)

## LxLaunch
LxLaunch is a simple launcher which either launches /usr/bin/python if no parameters are given to it, or launches the given ELF executable if an argument is entered. The binary launches as a child of the init daemon, under the current/default Lx instance.

## LxServer
LxServer is a simple Win32 (x64) server that waits for connections over the ADSS Bus from a compatible LXSS client (such as lxclient), and launches (WinExec) any input received as long as it fits within MAX_PATH. If needed, it configures the registry to enable ADSS Bus usage from root LXSS binaries other than the init daemon (see presentation slides), after which an initial reboot will be required. Launch it with -v for verbose information.

## LxClient
LxClient is a simple Linux (ELF) client that connects over the ADSS Bus to a compatible LXSS server (such as lxserver), and sends whatever the input command line argument is (which is expected to be a Win32 valid path to an executable, or other input that WinExec accepts) to the server. Launch it with -v following the Win32 path, for verbose information.

## Lx
The Lx directory contains a number of WinDBG scripts to dump the current LX state. Copy them to your c:\lx directory, or create a directory symlink (mklink /d) to wherever you've copied them, then execute in WinDBG as follows: $$>< c:\lx\lx.wds
