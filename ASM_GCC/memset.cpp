extern "C"
{
    using size_t = decltype(sizeof(void*));
    void* arlib_memset(void* dst, int value, size_t size) {
        __asm__ volatile(R"(        
        movq    %%rdi, %%rax
        testq   %%rdx, %%rdx
        je      .LBB0_18
        cmpq    $8, %%rdx
        jae     .LBB0_3
        xorl    %%ecx, %%ecx
        jmp     .LBB0_17
.LBB0_3:
        movzbl  %%sil, %%r8d
        cmpq    $32, %%rdx
        jae     .LBB0_5
        xorl    %%ecx, %%ecx
        jmp     .LBB0_14
.LBB0_5:
        movq    %%rdx, %%rcx
        andq    $-32, %%rcx
        movd    %%r8d, %%xmm0
        punpcklbw       %%xmm0, %%xmm0            # xmm0 = xmm0[0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7]
        pshuflw $0, %%xmm0, %%xmm0                # xmm0 = xmm0[0,0,0,0,4,5,6,7]
        pshufd  $0, %%xmm0, %%xmm0                # xmm0 = xmm0[0,0,0,0]
        leaq    -32(%%rcx), %%rdi
        movq    %%rdi, %%r10
        shrq    $5, %%r10
        addq    $1, %%r10
        movl    %%r10d, %%r9d
        andl    $7, %%r9d
        cmpq    $224, %%rdi
        jae     .LBB0_7
        xorl    %%edi, %%edi
        jmp     .LBB0_9
.LBB0_7:
        andq    $-8, %%r10
        negq    %%r10
        xorl    %%edi, %%edi
.LBB0_8:                                # =>This Inner Loop Header: Depth=1
        movdqu  %%xmm0, (%%rax,%%rdi)
        movdqu  %%xmm0, 16(%%rax,%%rdi)
        movdqu  %%xmm0, 32(%%rax,%%rdi)
        movdqu  %%xmm0, 48(%%rax,%%rdi)
        movdqu  %%xmm0, 64(%%rax,%%rdi)
        movdqu  %%xmm0, 80(%%rax,%%rdi)
        movdqu  %%xmm0, 96(%%rax,%%rdi)
        movdqu  %%xmm0, 112(%%rax,%%rdi)
        movdqu  %%xmm0, 128(%%rax,%%rdi)
        movdqu  %%xmm0, 144(%%rax,%%rdi)
        movdqu  %%xmm0, 160(%%rax,%%rdi)
        movdqu  %%xmm0, 176(%%rax,%%rdi)
        movdqu  %%xmm0, 192(%%rax,%%rdi)
        movdqu  %%xmm0, 208(%%rax,%%rdi)
        movdqu  %%xmm0, 224(%%rax,%%rdi)
        movdqu  %%xmm0, 240(%%rax,%%rdi)
        addq    $256, %%rdi                      # imm = 0x100
        addq    $8, %%r10
        jne     .LBB0_8
.LBB0_9:
        testq   %%r9, %%r9
        je      .LBB0_12
        leaq    (%%rdi,%%rax), %%r10
        addq    $16, %%r10
        shlq    $5, %%r9
        xorl    %%edi, %%edi
.LBB0_11:                               # =>This Inner Loop Header: Depth=1
        movdqu  %%xmm0, -16(%%r10,%%rdi)
        movdqu  %%xmm0, (%%r10,%%rdi)
        addq    $32, %%rdi
        cmpq    %%rdi, %%r9
        jne     .LBB0_11
.LBB0_12:
        cmpq    %%rdx, %%rcx
        je      .LBB0_18
        testb   $24, %%dl
        je      .LBB0_17
.LBB0_14:
        movq    %%rcx, %%rdi
        movq    %%rdx, %%rcx
        andq    $-8, %%rcx
        movd    %%r8d, %%xmm0
        punpcklbw       %%xmm0, %%xmm0            # xmm0 = xmm0[0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7]
        pshuflw $0, %%xmm0, %%xmm0                # xmm0 = xmm0[0,0,0,0,4,5,6,7]
.LBB0_15:                               # =>This Inner Loop Header: Depth=1
        movq    %%xmm0, (%%rax,%%rdi)
        addq    $8, %%rdi
        cmpq    %%rdi, %%rcx
        jne     .LBB0_15
        cmpq    %%rdx, %%rcx
        je      .LBB0_18
.LBB0_17:                               # =>This Inner Loop Header: Depth=1
        movb    %%sil, (%%rax,%%rcx)
        addq    $1, %%rcx
        cmpq    %%rcx, %%rdx
        jne     .LBB0_17
.LBB0_18:
)" ::
                         : "memory");
        return dst;
    }
}
