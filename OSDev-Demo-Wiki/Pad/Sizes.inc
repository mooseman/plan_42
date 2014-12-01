;
; Pad/Sizes.inc
;

; Calculate final Real Segment size
                SEGMENT         Real ALIGN=16

Real.Base       EQU             BIOS.Entry

                ALIGN           16

Real.Size       EQU             $ - $$

%assign         Demo.Size       Demo.Size + Real.Size
;-------------------------------------------------------------------------------
; Calculate final IDT Segment size
                SEGMENT         IDT  ALIGN=16

IDT.Start       EQU             Code.Base - IDT.Size

IDT.Limit       EQU             $ - $$ - 1

                ALIGN           16, DB 0

IDT.Size        EQU             $ - $$

%assign         Demo.Size       Demo.Size + IDT.Size
;-------------------------------------------------------------------------------
; Calculate final Ints Segment size
                SEGMENT         Ints  ALIGN=16

Ints.Base       EQU             IDT.Start + IDT.Size ; Equivalent to Code.Base

Ints.Limit      EQU             $ - $$ - 1

                ALIGN           16

Ints.Size       EQU             $ - $$

%assign         Demo.Size       Demo.Size + Ints.Size
;-------------------------------------------------------------------------------
; Calculate final User.LDT Segment size
                SEGMENT         User.LDT  ALIGN=16  NOBITS

User.LDT.Limit  EQU             $ - $$ - 1

                ALIGNB          16

User.LDT.Size   EQU             $ - $$
;-------------------------------------------------------------------------------
; Calculate final User.Data Segment size
                SEGMENT         User.Data  ALIGN=16  NOBITS

User.Data.Limit EQU             $ - $$ - 1

                ALIGNB          16

User.Data.Size  EQU             $ - $$
;-------------------------------------------------------------------------------
; Calculate final User.Code Segment size
                SEGMENT         User  ALIGN=16

User.Base       EQU             Ints.Base + Ints.Size

User.Limit      EQU             $ - $$ - 1

                ALIGN           16

User.Size       EQU             $ - $$

%assign         Demo.Size       Demo.Size + User.Size
;-------------------------------------------------------------------------------
; Calculate final Exec Segment size
                SEGMENT         Exec  ALIGN=16

Exec.Base       EQU             User.Base + User.Size

Exec.Limit      EQU             $ - $$ - 1

                ALIGN           16

Exec.Size       EQU             $ - $$

%assign         Demo.Size       Demo.Size + Exec.Size
;-------------------------------------------------------------------------------
; Calculate final Exec.LDT Segment size
                SEGMENT         Exec.LDT  ALIGN=16

Exec.LDT.Base   EQU             Exec.Base + Exec.Size

Exec.LDT.Limit  EQU             $ - $$ - 1

                ALIGN           16, DB 0

Exec.LDT.Size   EQU             $ - $$

%assign         Demo.Size       Demo.Size + Exec.LDT.Size
;-------------------------------------------------------------------------------
; Calculate final GDT Segment size
                SEGMENT         GDT  ALIGN=16

GDT.Base        EQU             Exec.LDT.Base + Exec.LDT.Size

GDT.Limit       EQU             $ - $$ - 1

; Note that GDT grows from here - no alignment necessary

%assign         Demo.Size       Demo.Size + GDT.Limit + 1
