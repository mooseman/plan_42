;
; Boot/RAM.inc
;

; This module uses the BIOS to determine how much RAM the system has.
; It saves the information away for the system to process after it has entered
; Protected Mode - BIOS functions aren't available then.

Boot.RAM:
                XOR             EAX,EAX           ; Zero high part of EAX
                MOV             AX,Data.Base >> 4 ; Point to global Data
                MOV             DS,AX
                MOV             ES,AX             ; With both Segments

                INT             BIOS.RAMTop.Int   ; Get size of Low RAM in kiB
                SHL             EAX,10            ; Turn into number of bytes
                MOV             [Data.RAMTop],EAX ; Top of (Real) RAM

                XOR             EBX,EBX           ; Start with beginning of RAM
                MOV             DI,Data.RAMMap    ; Store records here
.Loop:
                MOV             EAX,BIOS.RAMMap.Fn ; Function call
                MOV             CX,BIOS.RAMMap_size
                MOV             EDX,BIOS.RAMMap.Magic
                MOV             [DI+BIOS.RAMMap.Flags],BYTE BIOS.RAMMap.Flags.DontIgnore
                INT             BIOS.RAMMap.Int ; Call BIOS
                JC              .End            ; Carry? Finished

                CMP             EAX,BIOS.RAMMap.Magic ; BIOS function supported?
                JNE             .End            ; Nope.

                CMP             CL,BIOS.RAMMap.Flags ; Don't mind optional Flags DWORD
                JB              .Skip           ; Ignore if too small
                JE              .Test           ; No .Flags entry, so valid

                TEST            [DI+BIOS.RAMMap.Flags],BYTE BIOS.RAMMap.Flags.DontIgnore
                JZ              .Skip           ; Ignore if told to
.Test:
                MOV             EAX,[DI+BIOS.RAMMap.Length+0]
                OR              EAX,[DI+BIOS.RAMMap.Length+4]
                JZ              .Skip           ; Zero-length entry?

                INC     WORD    [Data.RAMMap.Num]  ; One more Map entry

                ADD             DI,BIOS.RAMMap_size ; Next record
                CMP             DI,Data.Size-BIOS.RAMMap_size ; Any more room?
                JA              .End            ; Nope. Leave
.Skip:
                TEST            EBX,EBX         ; End of sequence?
                JNZ             .Loop           ; Not yet...
.End:
