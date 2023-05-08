
#ifndef UVM_H
#define UVM_H

__BEGIN_DECLS

int setPid(int pid);
int commitDisplay(const int fd, const int commit);
int32_t attachUvmBuffer(int bufferFd);
int32_t dettachUvmBuffer(int bufferFd);
int32_t getVideoType(int bufferFd);

__END_DECLS

#endif
