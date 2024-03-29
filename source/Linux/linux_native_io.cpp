#include "Linux/linux_native_io.hpp"
#ifdef UNIX
    #include <sys/stat.h>
    #include <cstdio>
    #include <unistd.h>
    #include "Types.hpp"
namespace ARLib {
size_t UnixFileSize(FILE* fp) {
    int fd = fileno(fp);
    struct stat st {};
    int ec = fstat(fd, &st);
    if (ec < 0) return 0;
    return static_cast<size_t>(st.st_size);
}
void UnixClose(int fd) {
    ::close(fd);
}
}    // namespace ARLib
#endif
