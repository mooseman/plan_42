;Enable A20 via keyboard controller

    call .wait		

    mov al,0D1h    ;Send 0D1h command (A20 control)
    out 64h,al

    call .wait

    mov al,0dfh
    out 60h,al     ;Send 0DFh command (Enable)

    jmp short .end

.wait:
    mov cx, 0FFFFh
.w:
    in al,64h      ;Get keybord controller status
    test al,2
    loopnz .w      ;Wait until ready
    ret
.end:



;enable_A20: ;(from http://www.mega-tokyo.com/os/os-faq-memory.html#enable_a20)
; cli

; call    a20wait
; mov     al,0xAD
; out     0x64,al

; call    a20wait
; mov     al,0xD0
; out     0x64,al

; call    a20wait2
; in      al,0x60
; push    eax

; call    a20wait
; mov     al,0xD1
; out     0x64,al

; call    a20wait
; pop     eax
; or      al,2
; out     0x60,al

;         call    a20wait
; mov     al,0xAE
; out     0x64,al

; call    a20wait
; ret

; a20wait:
; .l0:mov     ecx,65536
; .l1:in      al,0x64
; test    al,2
; jz      .l2
; loop    .l1
; jmp     .l0
; .l2:ret


; a20wait2:
; .l0:mov     ecx,65536
; .l1:in      al,0x64
; test    al,1
; jnz     .l2
;         loop    .l1
; jmp     .l0
; .l2:ret