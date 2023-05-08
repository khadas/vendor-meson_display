#include <UvmDev.h>

int setPid(int pid)
{
    return UvmDev::getInstance().setPid(pid);
}

int commitDisplay(const int fd, const int commit)
{
    return UvmDev::getInstance().commitDisplay(fd, commit);
}

int32_t attachUvmBuffer(int bufferFd)
{
    return UvmDev::getInstance().attachBuffer(bufferFd);
}

int32_t dettachUvmBuffer(int bufferFd)
{
    return UvmDev::getInstance().dettachBuffer(bufferFd);
}

int32_t getVideoType(int bufferFd)
{
    return UvmDev::getInstance().getVideoType(bufferFd);
}
