;
; User/LDT.inc
;

; This defines the LDT that the User program needs.

                SEGMENT         User.LDT  START=0  ALIGN=16  NOBITS

                USE32

User.Stack3.Size EQU            0010h           ; Doesn't need much at all!
User.Stack0.Size EQU            0200h           ; Needs a bit more...

User.LDT:
User.LDT.Alloc  RESB            LDT.Alloc_size
User.LDT.Alias  RESB            x86.Desc_size
User.LDT.Stack0 RESB            x86.Desc_size
User.LDT.Stack3 RESB            x86.Desc_size
User.LDT.Data   RESB            x86.Desc_size
User.LDT.Code   RESB            x86.Desc_size

%if (User.LDT.Alias-User.LDT) != DT.Alias
%error "Invalid User LDT alias"
%endif
