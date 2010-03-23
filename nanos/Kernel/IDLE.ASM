;Idle task

;This is what I call idle...

.idle_loop:
	hlt
	jmp	.idle_loop
