; fibo-linux.asm
;
; written on Sat  04-23-2005  by Ed Beroset
; and released to the public domain by the author
;
; This program calculates all terms of the Fibonacci series
; up to maxTerms (a programmer-alterable constant).  The Fibonacci
; series' first two terms F(1) and F(2) are both equal to one.  Each
; term thereafter is simply the sum of the previous two terms.  This 
; program uses two arrays, num1 and num2 to represent two successive
; terms of the series.  Each array contains the ASCII representation
; of the number including leading zeroes.  This is done to speed the
; printing of the result and is much less costly in computer time
; than the alternative method of calculating everything in binary and
; converting to ASCII each time.
;
;****************************************************************************
; assemble using nasm, and link with ld:   
;
; nasm -o fibo-linux.o -f elf fibo-linux.asm
; ld -s -o fibo-linux fibo-linux.o
;
;****************************************************************************
; Alterable Constant
;****************************************************************************
; You can adjust this upward but be aware that the file size grows very 
; large very quickly.  The 150000th term of the Fibonacci sequence is 
; 31348 digits long, and the file containing all 150000 terms is 2.2G. 
; 
	maxTerms    equ 50000 ; number of terms of the series to calculate

;****************************************************************************
; Number digits to use.  This is based on a little bit of tricky math.
; One way to calculate F(n) (i.e. the nth term of the Fibonacci seeries)
; is to use the equation int(phi^n/sqrt(5)) where ^ means exponentiation
; and phi = (1 + sqrt(5))/2, the "golden number" which is a constant about 
; equal to 1.618.  To get the number of decimal digits, we just take the 
; base ten log of this number.  We can very easily see how to get the 
; base phi log of F(n) -- it's just n*lp(phi)+lp(sqrt(5)), where lp means 
; a base phi log.  To get the base ten log of this we just divide by the 
; base ten log of phi.  If we work through all that math, we get:
;
; digits = terms * log(phi) + log(sqrt(5))/log(phi)
;
; The constants below are slightly high to assure that we always have 
; enough room.  As mentioned above the 150000th term has 31348 digits,
; but this formula gives 31351.  Not too much waste there.
;
        digits	    equ (maxTerms*209+1673)/1000	

; This is just the number of digits for the term counter.  In theory this
; could be altered, but unless you want to spend many gigabytes on the
; output file, eight should be plenty.
	cntDigits   equ 8	; number of digits for counter

section .text
    global _start
;****************************************************************************
;****************************************************************************
_start:	
; initializes the two numbers and the counter.  Note that this assumes
; that num1 and num2 areas are contiguous!
;
	mov	eax,'0000'		; initialize to all ASCII zeroes
	mov	edi,num1		; 
	mov	ecx,1+digits/2	; four bytes at a time and one extra
	cld			; initialize from low to high memory
	rep	stosd		; write the data
	inc	eax		; make sure ASCII zero is in al
	mov	[num2 + digits - 1],al ; last digit is one
	mov	[counter + cntDigits - 1],al ; 
	mov	dword [msg2],middle

	mov	ebp,[msd2]
	jmp	.bottom		; done with initialization, so begin

.top
	; add num2 to num1
	mov	edi,num1+digits-1
	mov	esi,num2+digits-1
	mov	ecx,digits	; 
	mov	ebp,[msd1]	;
	call	AddNumbers	; num1 += num2
	mov	[msd1],ebp	;
	inc	ebp
	mov	ebx,num1+digits
	sub	ebx,ebp		; get length in ebx
	call	PrintLine	;
	dec	dword [term]	; decrement loop counter
	jz	.done		;

	; add num1 to num2
	mov	edi,num2+digits-1
	mov	esi,num1+digits-1
	mov	ecx,digits	;
	mov	ebp,[msd2]	;
	call	AddNumbers	; num2 += num1
	mov	[msd2],ebp	;
	inc	ebp
.bottom
	mov	ebx,num2+digits
	sub	ebx,ebp		; get length in ebx
	call	PrintLine	;
	dec	dword [term]	; decrement loop counter
	jnz	.top		;
.done
	call	CRLF		; finish off with CRLF
	mov	ebx,0		; exit code
	mov	eax,1h		; sys_exit
	int	80h		;

;****************************************************************************
;
; PrintLine
; prints a single line of output containing one term of the 
; Fibonacci sequence.  The first few lines look like this:
;
; Fibonacci(1): 1
; Fibonacci(2): 1
; Fibonacci(3): 2
; Fibonacci(4): 3
;
; INPUT:     ds:ebp ==> number string, ebx = string length
; OUTPUT:    CF set on error, EAX = error code if carry set
; DESTROYED: eax, ecx, edx, edi
;
;****************************************************************************
PrintLine:
	push	ebx
	mov	ecx,eol		; print combined CRLF and msg1
	mov	edx,msg1len+eollen   ; 
	call	PrintString	;

	mov	edi,counter	; print counter and msg2
	mov	ecx,cntDigits + msg2len	;
	call	PrintNumericString

	call	IncrementCount	; also increment the counter

	mov	ecx,ebp		; get pointer to number string
	pop	edx		; and length of it
	jmp	PrintString	;

;****************************************************************************
;
; PrintNumericString 
; prints the numeric string at DS:EDI, suppressing leading zeroes
; max length is ECX
;
; INPUT:     ds:edi ==> number string, ecx = max string length
; OUTPUT:    CF set on error, EAX = error code if carry set
; DESTROYED: eax, ecx, edx, edi
;
;****************************************************************************
PrintNumericString:
	; first scan for the first non-zero byte
	mov	al,'0'		; look for ASCII zero
	cld			; scan from MSD to LSD
	repe	scasb		;
	mov	edx,ecx		;
	mov	ecx,edi		; points to one byte after
	dec	ecx		; back up one character
	inc	edx		;
	; deliberately fall through to PrintString

;****************************************************************************
; 
; PrintString 
; prints the string at ECX with length EDX to stdout
;
; INPUT:     ds:ecx ==> string, edx = string length
; OUTPUT:    CF set on error, EAX = error code if carry set
; DESTROYED: eax
;
;****************************************************************************
PrintString:
	mov	ebx, 1		; write to stdout
	mov     eax, 4h         ; write to file handle
	int	80h		; ignore return value
	ret			;

;****************************************************************************
;
; AddNumbers
; add number 2 at ds:esi to number 1 at es:edi of width ecx
; 
;
; INPUT:     es:edi ==> number1, ds:esi ==> number2, ecx= max width
;		ds:ebp ==> msd of number 1
; OUTPUT:    CF set on overflow
; DESTROYED: eax, esi, edi
;
;****************************************************************************
AddNumbers:
	dec	ebp		; 
	std			; go from LSB to MSB
	clc			;
	pushf			; save carry flag
.top
	mov	eax,0f0fh	; convert from ASCII BCD to BCD
	and  	al,[esi]	; get next digit of number2 in al
	and	ah,[edi]	; get next digit of number1 in ah
	popf			; recall carry flag
	adc	al,ah		; add these digits
	aaa			; convert to BCD
	pushf			;
	add	al,'0'		; convert back to ASCII BCD digit
	stosb			; save it and increment both counters
	dec	esi		;
;
	cmp	edi,ebp		; are we at a new significant digit?
	loopnz	.top		; keep going until we've got them all
	cmp	al,'0'		; is it a zero?
	jnz	.done		; yes, so keep 
	inc	ebp		;
.done
	popf			; recall carry flag
	ret			;

;****************************************************************************
; 
; IncrementCount
; increments a multidigit term counter by one
;
; INPUT:     none
; OUTPUT:    CF set on overflow
; DESTROYED: eax, ecx, edi
;
;****************************************************************************
IncrementCount:
	mov	ecx,cntDigits	;
	mov	edi,counter+cntDigits-1
	std			; go from LSB to MSB
	stc			; this is our increment
	pushf			; save carry flag
.top
	mov	ax,000fh	; convert from ASCII BCD to BCD
	and	al,[edi]	; get next digit of counter in al
	popf			; recall carry flag
	adc	al,ah		; add these digits
	aaa			; convert to BCD
	pushf			;
	add	al,'0'		; convert back to ASCII BCD digit
	stosb			; save and increment counter
	loop	.top		;
	popf			; recall carry flag
	ret			;
	
;****************************************************************************
;
; CRLF
; prints carriage return, line feed pair to stdout
;
; INPUT:     none
; OUTPUT:    CF set on error, AX = error code if carry set
; DESTROYED: ax, bx, cx, dx
;
;****************************************************************************
CRLF:	mov	ecx,eol		;
	mov	edx,eollen	;
	jmp	PrintString	;

;****************************************************************************
; data
;****************************************************************************
section .data
eol	db  10			; Linux-style end of line
eollen	equ $ - eol

msg1	db  'Fibonacci('	;
msg1len	equ $ - msg1
;****************************************************************************
term dd maxTerms		;
;****************************************************************************
counter times cntDigits db '0'

middle  equ '): '
msg2	dd  0 ;middle		;
msg2len	equ 3

msd1 dd num1+digits-1		; pointer to most significant digit of num1
msd2 dd num2+digits-1		; pointer to most significant digit of num2

num1 times digits db 0
num2 times digits db 0
overrun times 4 db 0		; extra space to assure we don't overwrite
		
