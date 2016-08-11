#pragma once

size_t read(int fd, void* buf, size_t count);
size_t write(int fd, const void* buf, size_t count);
int ioctl(int d, int request, ...);
int open(const char* pathname, int flags);
int fcntl(int fd, int cmd, ...);

#define F_SETFD 2

#define O_WRONLY 0x01
#define O_RDWR   0x02
#define O_DIRECT 0x80000

