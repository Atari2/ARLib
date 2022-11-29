#pragma once
#include "../Compat.h"
#include "../Types.h"
#ifdef UNIX
extern "C" {
#ifndef __FILE_defined
#define __FILE_defined 1
typedef struct _IO_FILE FILE;
#endif
}
namespace ARLib {
    size_t UnixFileSize(FILE* fp);
}
#endif