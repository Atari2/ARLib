extern "C" {
	double arlib_log(double arg) {
		double ret = 0.0;
		double one = 1.0;
		double limit = 0.29;
		__asm__ volatile(
			"fldln2 \n"			// log(2)
			"fldl	%1 \n"		// x : log(2)
			"fxam \n"
			"fnstsw \n"
			"fld %%st \n"		// x : x : log(2)
			"sahf \n"
			"jc	3f \n"		// in case x is NaN or +-Inf
			"4:	\n"
			"fsubl	%2 \n"		// x-1 : x : log(2)
			"fld %%st \n"		// x-1 : x-1 : x : log(2)
			"fabs \n"			// |x-1| : x-1 : x : log(2)
			"fcompl	%3 \n"	// x-1 : x : log(2)
			"fnstsw		 \n"	// x-1 : x : log(2)
			"andb	$0x45, %%ah \n"
			"jz	2f \n"
			"fxam \n"
			"fnstsw \n"
			"andb	$0x45, %%ah \n"
			"cmpb	$0x40, %%ah \n"
			"jne	5f \n"
			"fabs		 \n"	// log(1) is +0 in all rounding modes.
			"5: \n"
			"fstp %%st(1) \n"		// x-1 : log(2)
			"fyl2xp1		 \n"	// log(x)
			"fstl %0\n"
			"jmp rt \n"
			"2: \n"
			"fstp %%st(0)	 \n"	// x : log(2)
			"fyl2x		 \n"	// log(x)
			"fstl %0\n"
			"jmp rt \n"
			"3:  \n"
			"jp	4b \n"		// in case x is +-Inf
			"fstp %%st(1) \n"
			"fstp %%st(1) \n"
			"rt:"
			: "=m"(ret)
			: "g"(arg), "g"(one), "g"(limit)
			: "memory"
			);
		return ret;
	}
}