TITLE memset.obj

public arlib_memset
_TEXT SEGMENT
.code
arlib_memset proc
        push rsi
        push rdx
        push rdi
        
        mov     rsi, rdx
        mov     rax, rcx
        mov     rdx, r8
        mov     rdi, rcx
        
        test    rdx, rdx
        je      $LBB0_18
        cmp     rdx, 8
        jae     $LBB0_3
        xor     ecx, ecx
        jmp     $LBB0_17
$LBB0_3:
        movzx   r8d, sil
        cmp     rdx, 32
        jae     $LBB0_5
        xor     ecx, ecx
        jmp     $LBB0_14
$LBB0_5:
        mov     rcx, rdx
        and     rcx, -32
        movd    xmm0, r8d
        punpcklbw       xmm0, xmm0              
        pshuflw xmm0, xmm0, 0                   
        pshufd  xmm0, xmm0, 0                   
        lea     rdi, [rcx - 32]
        mov     r10, rdi
        shr     r10, 5
        add     r10, 1
        mov     r9d, r10d
        and     r9d, 7
        cmp     rdi, 224
        jae     $LBB0_7
        xor     edi, edi
        jmp     $LBB0_9
$LBB0_7:
        and     r10, -8
        neg     r10
        xor     edi, edi
$LBB0_8:                                
        movdqu  xmmword ptr [rax + rdi], xmm0
        movdqu  xmmword ptr [rax + rdi + 16], xmm0
        movdqu  xmmword ptr [rax + rdi + 32], xmm0
        movdqu  xmmword ptr [rax + rdi + 48], xmm0
        movdqu  xmmword ptr [rax + rdi + 64], xmm0
        movdqu  xmmword ptr [rax + rdi + 80], xmm0
        movdqu  xmmword ptr [rax + rdi + 96], xmm0
        movdqu  xmmword ptr [rax + rdi + 112], xmm0
        movdqu  xmmword ptr [rax + rdi + 128], xmm0
        movdqu  xmmword ptr [rax + rdi + 144], xmm0
        movdqu  xmmword ptr [rax + rdi + 160], xmm0
        movdqu  xmmword ptr [rax + rdi + 176], xmm0
        movdqu  xmmword ptr [rax + rdi + 192], xmm0
        movdqu  xmmword ptr [rax + rdi + 208], xmm0
        movdqu  xmmword ptr [rax + rdi + 224], xmm0
        movdqu  xmmword ptr [rax + rdi + 240], xmm0
        add     rdi, 256
        add     r10, 8
        jne     $LBB0_8
$LBB0_9:
        test    r9, r9
        je      $LBB0_12
        lea     r10, [rdi + rax]
        add     r10, 16
        shl     r9, 5
        xor     edi, edi
$LBB0_11:                               
        movdqu  xmmword ptr [r10 + rdi - 16], xmm0
        movdqu  xmmword ptr [r10 + rdi], xmm0
        add     rdi, 32
        cmp     r9, rdi
        jne     $LBB0_11
$LBB0_12:
        cmp     rcx, rdx
        je      $LBB0_18
        test    dl, 24
        je      $LBB0_17
$LBB0_14:
        mov     rdi, rcx
        mov     rcx, rdx
        and     rcx, -8
        movd    xmm0, r8d
        punpcklbw       xmm0, xmm0              
        pshuflw xmm0, xmm0, 0                   
$LBB0_15:                               
        movq    qword ptr [rax + rdi], xmm0
        add     rdi, 8
        cmp     rcx, rdi
        jne     $LBB0_15
        cmp     rcx, rdx
        je      $LBB0_18
$LBB0_17:                               
        mov     byte ptr [rax + rcx], sil
        add     rcx, 1
        cmp     rdx, rcx
        jne     $LBB0_17
$LBB0_18:
        pop rdi
        pop rdx
        pop rsi
        ret
arlib_memset endp
_TEXT ENDS
END