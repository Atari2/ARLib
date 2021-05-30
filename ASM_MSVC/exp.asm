TITLE exp.obj

public arlib_exp
_TEXT SEGMENT
.data
fpu_control_word word 2
fpu_new_control word 2
.code
arlib_exp proc x: mmword
	movq x, xmm0
	fld x
	fxam
	fstsw ax
	mov dh, 45h
	and dh, ah
	cmp dh, 5h
	je $L1
	fldl2e
	fmulp
	fld st
	fstcw fpu_control_word
	mov ax, fpu_control_word
	mov dx, 0f3ffh
	and ax, dx
	mov dx, 0c00h
	or ax, dx
	mov fpu_new_control, ax
	fldcw fpu_new_control
	frndint
	fsubr st(0), st(1)
	f2xm1
	fld1
	faddp
	fscale
	fstp x
	fstp st
	movq xmm0, x
	fldcw fpu_control_word
	ret
$L1:
	test eax, 200h
	jz $L2
	fstp x
	fstp st
	fldz
$L2:
	ret
arlib_exp endp
_TEXT ENDS
END