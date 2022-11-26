macro(CheckAvx)
    set(SAVED_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(AVX_FLAGS)

    include(CheckCXXSourceRuns)
    set(CMAKE_REQUIRED_FLAGS)

    if (MSVC)
        set(CMAKE_REQUIRED_FLAGS "/arch:AVX")
    else()
        set(CMAKE_REQUIRED_FLAGS "-mavx -mavx2")
    endif()

    check_cxx_source_runs(
    "
    // on latest msvc/gcc/clang at the time of writing (29/09/2022), 
    // with avx turned on zeroing a double produces:
    // vxorp{s|d}  xmm0, xmm0, xmm0
    // https://godbolt.org/z/rqavcvbnn
    // which triggers an invalid instruction exception on non-avx machines

    #ifdef _MSC_VER
    #include <Windows.h>
    #include <cstdio>
    int main() {
        __try {
            double a = 0.0;      
            return 0;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return -1;
        }
    }
    #else
    #include <signal.h>
    #include <cstdio>
    #include <cstdlib>

    void sigill_handler(int sig) {
        if (sig != SIGILL) return;
        exit(-1);
    }

    int main() {
        signal(SIGILL, sigill_handler);
        double a = 0.0;
        return 0;
    }
    #endif        
    "
    AVX_AVAILABLE
    )

    if (AVX_AVAILABLE)
        if (MSVC)
            set(AVX_FLAGS "/arch:AVX2")
        else()
            set(AVX_FLAGS -mavx -mavx2)
        endif()
    endif()
    set(CMAKE_REQUIRED_FLAGS ${SAVED_CMAKE_REQUIRED_FLAGS})
endmacro(CheckAvx)