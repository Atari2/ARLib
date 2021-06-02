
extern "C" {
	double arlib_exp(double arg) {
		double ret = 0.0;
		__asm__ volatile(
			"fldl %1 \n"
			"fxam \n"
			"fstsw    %%ax \n"
			"movb    $0x45, %%dh \n"
			"andb    %%ah, %%dh \n"
			"cmpb    $0x05, %%dh \n"
			"je    1f \n"
			"fldl2e \n"
			"fmulp    \n"
			"fld    %%st \n"
			"frndint \n"
			"fsubr    %%st,%%st(1)    \n"
			"fxch \n"
			"f2xm1    \n"
			"fld1 \n"
			"faddp        \n"
			"fscale        \n"
			"fstpl %0 \n"
			"fstp %%st \n"
			"jmp 2f \n"
			"1: \n"
			"testl    $0x200, %%eax \n"
			"jz    2f           \n"
			"fstpl    %0 \n"
			"fldz                \n"
			"2: \n"
			: "=m"(ret)
			: "m"(arg)
			: "memory"
			);
		return ret;

	}
}