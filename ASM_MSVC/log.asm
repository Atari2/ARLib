TITLE log.obj

public arlib_log
_TEXT SEGMENT
.data
one_cst dq 1.0
limit_cst dq 0.26
.code
arlib_log proc x: mmword
	fldln2			
	movq x, xmm0
	fld x	
	fxam
	fnstsw ax
	fld	st		
	sahf
	jc	$L3		
$L4:	
	fsub	one_cst	
	fld	st		
	fabs			
	fcomp	limit_cst
	fnstsw	ax
	and	ah, 45h
	jz	$L2
	fxam
	fnstsw ax
	and	ah, 45h
	cmp	ah, 40h
	jne	$L5
	fabs			
$L5:	
	fstp	st(1)		
	fyl2xp1
	fst x
	movq xmm0, x
	ret

$L2:	
	fstp	st(0)		
	fyl2x	
	fst x
	movq xmm0, x
	ret
$L3:
	jp	$L4
	fst x
	movq xmm0, x
	fstp	st(1)
	fstp	st(1)
	ret
arlib_log endp
_TEXT ENDS
END