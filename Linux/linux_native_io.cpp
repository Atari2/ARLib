#include "linux_native_io.h"
#ifdef UNIX
    #include <sys/stat.h>
    #include <cstdio>
    #include "../Types.h"
namespace ARLib {
size_t UnixFileSize(FILE* fp) {
    int fd = fileno(fp);
    struct stat st {};
    int ec = fstat(fd, &st);
    if (ec < 0) return 0;
    return static_cast<size_t>(st.st_size);
}
}    // namespace ARLib
#endif
