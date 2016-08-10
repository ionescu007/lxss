# lxss
Fun with the Windows Subsystem for Linux (WSL/LXSS)

## LxLaunch
LxLaunch is a simple launcher which either launches /usr/bin/python if no parameters are given to it, or launches the given ELF executable if an argument is entered. The binary launches as a child of the init daemon, under the current/default Lx instance.

## Lx
The Lx directory contains a number of WinDBG scripts to dump the current LX state. Copy them to your c:\lx directory, or create a directory symlink (mklink /d) to wherever you've copied them, then execute in WinDBG as follows: $$>< c:\lx\lx.wds
