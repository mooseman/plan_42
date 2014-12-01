;
; Exec/Init.inc
;

; This finalises the initialisation of the CPU now that we're in Protected Mode.
; The Segment registers are loaded with PM-friendly values, and other registers
; are set up as needed.
;
; This is also the time to initialise the global Data area - at least, that part
; that wasn't set up by Boot. There was no real need to store the data in the
; Boot image - most of it is run-time generated anyway.
;
; And since this is a multi-tasking system that uses TSSs, the Task Register
; needs to be set up, to store the current context before switching to the new
; one. That means allocating a TSS, and initialising it with sensible values.

Exec.Init:
                ; Initialise EFlags
                PUSH   DWORD    x86.EFlags.1
                POPFD

                ; Point to global Data
                MOV             AX,Selector(GDT.Data, GDT, RPL0)
                MOV             DS,AX           ; Try to keep this here

                ; Point to Screen
                MOV             AX,Selector(GDT.VGA, GDT, RPL0)
                MOV             ES,AX           ; This will be used for lots
                MOV             FS,AX           ; Try to keep this here

                ; Point to GDT. (With GS! How apt!)
                MOV             AX,Selector(GDT.Alias, GDT, RPL0)
                MOV             GS,AX           ; Try to keep this here

                MOV    DWORD    [Data.RAM],Demo.First

                MOV     BYTE    [Data.Key.Shifts],0

                ; Now allocate Exec's TSS. Use Exec's LDT!
                MOV             AX,Selector(GDT.Exec.LDT, GDT, RPL0)
                CALL            Exec.Alloc.TSS  ; Assume this will work!
                MOV             AH,0            ; Not a System TSS
                CALL            Exec.Alloc.TSS.Enable

                ; Now any Task Switch will let Exec resume later
                LTR             BX

                RET
