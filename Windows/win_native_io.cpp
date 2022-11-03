#include "win_native_io.h"
#ifdef ON_WINDOWS
#include "../Pair.h"
#include <Windows.h>
#include <cstdio>
#include <fcntl.h>
#include <io.h>

namespace ARLib {

    static bool GenericWrite(HANDLE hFile, LPCVOID buffer, DWORD nBytes, LPDWORD bytesWritten) {
        return WriteFile(hFile, buffer, nBytes, bytesWritten, nullptr);
    }
    static HANDLE FileToHandle(FILE* fp) {
        return reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(fp)));
    }
    bool WriteChar(char c, FILE* fp) {
        const char* fc = &c;
        HANDLE hdl = FileToHandle(fp);
        [[maybe_unused]] DWORD nBytesWritten{};
        return GenericWrite(hdl, fc, sizeof(char), &nBytesWritten);
    }

    static Pair<DWORD, DWORD> ModeToAccessFlags(const char* mode) {
        size_t len = ARLib::strlen(mode);
        DWORD desiredAccess = 0;
        DWORD creat = 0;
        for (size_t i = 0; i < len; i++) {
            switch (mode[i]) {
            case 'w':
                desiredAccess |= GENERIC_WRITE;
                creat = CREATE_ALWAYS;
                break;
            case 'r':
                desiredAccess |= GENERIC_READ;
                creat = OPEN_EXISTING;
                break;
            case 'b':
                // no effect
                break;
            case 'a':
                desiredAccess |= FILE_APPEND_DATA;
                creat = OPEN_ALWAYS;
                break;
            case 'x':
                creat = CREATE_NEW;
                break;
            case '+':
                // update?
                break;
            default:
                break;
            }
        }
        return Pair{desiredAccess, creat};
    }

    static int ModeToFlags(const char* mode) {
        size_t len = ARLib::strlen(mode);
        int flags = _O_TEXT;
        for (size_t i = 0; i < len; i++) {
            switch (mode[i]) {
            case 'w':
                if (flags & _O_RDONLY) {
                    flags = (flags & ~_O_RDONLY);
                    flags |= _O_RDWR;
                } else {
                    flags |= _O_WRONLY;
                }
                break;
            case 'r':
                if (flags & _O_WRONLY) {
                    flags = (flags & ~_O_WRONLY);
                    flags |= _O_RDWR;
                } else {
                    flags |= _O_RDONLY;
                }
                break;
            case 'b':
                flags = (flags & ~_O_TEXT);
                flags |= _O_BINARY;
                break;
            case 'a':
                flags |= _O_APPEND;
                break;
            case '+':
                // update?
                break;
            default:
                break;
            }
        }
        return flags;
    }

    FILE* Win32OpenFile(const char* filename, const char* mode) {
        const auto [access, creat] = ModeToAccessFlags(mode);
        HANDLE hdl = CreateFile(filename, access, 0, NULL, creat, FILE_ATTRIBUTE_NORMAL, NULL);
        int fd = _open_osfhandle(reinterpret_cast<intptr_t>(hdl), ModeToFlags(mode));
        return _fdopen(fd, mode);
    }

    bool Win32CloseFile(FILE* fp) {
        // fclose must be used here because _fdopen moves the ownership of the handle to the FILE*.
        // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=msvc-170
        return std::fclose(fp);
    }

    bool Win32DeleteFile(const char* filename) {
        return DeleteFile(filename);
    }

    bool Win32RenameFile(const char* filename_old, const char* filename_new) {
        return MoveFile(filename_old, filename_new);
    }

    int Win32SeekFile(FILE* fp, int off, int whence) {
        auto mapWhence = [](int w) -> DWORD {
            switch (w) {
            case SEEK_CUR:
                return FILE_CURRENT;
            case SEEK_SET:
                return FILE_BEGIN;
            case SEEK_END:
                return FILE_END;
            default:
                return -1;
            };
        };
        return SetFilePointer(FileToHandle(fp), off, NULL, mapWhence(whence));
    }

    size_t Win32TellFile(FILE* fp) {
        return SetFilePointer(FileToHandle(fp), 0, NULL, FILE_CURRENT);
    }

    size_t Win32SizeFile(FILE* fp) {
        BY_HANDLE_FILE_INFORMATION info{};
        if (!GetFileInformationByHandle(FileToHandle(fp), &info)) { return INVALID_FILE_SIZE; };
        size_t high = static_cast<size_t>(info.nFileSizeHigh);
        size_t low = static_cast<size_t>(info.nFileSizeLow);
        constexpr size_t shift = (sizeof(DWORD) * BITS_PER_BYTE);
        static_assert(sizeof(size_t) == (sizeof(DWORD) * 2), "DWORD is too big");
        return (high << shift) | low;
    }

    char ReadChar(FILE* fp) {
        char c[1]{};
        DWORD bytesRead{};
        BOOL res = ReadFile(FileToHandle(fp), c, sizeof(char), &bytesRead, NULL);
        if (!res || bytesRead != sizeof(char)) return EOF;
        return c[0];
    }

    char* ReadLine(char* buf, int n, FILE* fp) {
        // FIXME: do this better
        for (int cur = 0; cur < n; cur++) {
            char c = ReadChar(fp);
            if (c == EOF) {
                return buf;
            } else if (c == '\n') {
                buf[cur] = c;
                return buf;
            } else {
                buf[cur] = c;
            }
        }
        return buf;
    }

    char ReadInChar() {
        return ReadChar(stdin);
    }

    size_t ReadFileGeneric(void* buffer, size_t size, size_t count, FILE* fp) {
        DWORD bytesRead{};
        if (!ReadFile(FileToHandle(fp), buffer, size * count, &bytesRead, NULL)) { return EOF; }
        return bytesRead;
    }

    size_t WriteFileGeneric(const void* buffer, size_t size, size_t count, FILE* fp) {
        DWORD nBytes{};
        if (GenericWrite(FileToHandle(fp), buffer, static_cast<DWORD>(size * count), &nBytes)) {
            return nBytes;
        } else {
            return EOF;
        }
    }

    size_t WriteOutGeneric(const void* buffer, size_t size, size_t count) {
        return WriteFileGeneric(buffer, size, count, stdout);
    }

    size_t WriteStringFileGeneric(const char* buf, FILE* fp) {
        const size_t size = sizeof(char);
        const size_t count = ARLib::strlen(buf);
        size_t ret = WriteFileGeneric(buf, size, count, fp);
        WriteChar('\n', stdout);
        return ret;
    }

    size_t WriteStringOutGeneric(const char* buf) {
        const size_t size = sizeof(char);
        const size_t count = ARLib::strlen(buf);
        size_t ret = WriteFileGeneric(buf, size, count, stdout);
        WriteChar('\n', stdout);
        return ret;
    }

} // namespace ARLib
#endif