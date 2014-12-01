;
; Exec/LDT.inc
;

; This is the Local Descriptor Table for the Executive

                SEGMENT         Exec.LDT  VSTART=0  ALIGN=16

                USE32

; This macro makes it easy to have a one-line invocation of LDT.Alloc
; The parameters are: GDT Entry, LDT Limit
%macro          LDT.Vars                2
%00             ISTRUC                  LDT.Alloc
AT LDT.Alloc.Free,   DW                 0
AT LDT.Alloc.Limit,  DW                 %2
                     DB                 0
AT LDT.Alloc.Type,   DB                 x86.Desc.Sys.Type.LDT
AT LDT.Alloc.GDT,    DW                 %1
                IEND
%endmacro

Exec.LDT.Alias.Base  EQU        Exec.LDT.Base
Exec.LDT.Alias.Limit EQU        Exec.LDT.Limit
Exec.LDT.Alias.Type  EQU        Type.Mem(Data, DPL0, RW)
Exec.LDT.Alias.Gran  EQU        Gran.Mem(Byte, Small)

Exec.LDT.IDT.Base    EQU        IDT.Base
Exec.LDT.IDT.Limit   EQU        IDT.Max - 1
Exec.LDT.IDT.Type    EQU        Type.Mem(Data, DPL0, RW)
Exec.LDT.IDT.Gran    EQU        Gran.Mem(Byte, Small)

Exec.LDT.Stack.Top   EQU        Exec.Stack.Top            ; Let's play with Expand-Down Descriptors
Exec.LDT.Stack.Limit EQU        (0-Exec.Stack.Size) >> 12 - 1
Exec.LDT.Stack.Type  EQU        Type.Mem(Stack, DPL0, RW)
Exec.LDT.Stack.Gran  EQU        Gran.Mem(Gran, Big)

Exec.LDT.Code.Base   EQU        Exec.Base
Exec.LDT.Code.Limit  EQU        Exec.Limit
Exec.LDT.Code.Type   EQU        Type.Mem(Code, DPL0, NoRW)
Exec.LDT.Code.Gran   EQU        Gran.Mem(Byte, Def32)

Exec.LDT:
Exec.LDT.Vars   LDT.Vars        GDT.Exec.LDT, Exec.LDT.Limit
Exec.LDT.Alias  Desc.Mem        Exec.LDT.Alias.Base, Exec.LDT.Alias.Limit, Exec.LDT.Alias.Type, Exec.LDT.Alias.Gran
Exec.LDT.IDT    Desc.Mem        Exec.LDT.IDT.Base,   Exec.LDT.IDT.Limit,   Exec.LDT.IDT.Type,   Exec.LDT.IDT.Gran
Exec.LDT.Stack  Desc.Mem        Exec.LDT.Stack.Top,  Exec.LDT.Stack.Limit, Exec.LDT.Stack.Type, Exec.LDT.Stack.Gran
Exec.LDT.Code   Desc.Mem        Exec.LDT.Code.Base,  Exec.LDT.Code.Limit,  Exec.LDT.Code.Type,  Exec.LDT.Code.Gran

Exec.LDT.Double         EQU       $
Exec.LDT.Double.Stack   Desc.Mem  0, 0, 0, 0 ; Stack for Double Fault handler
Exec.LDT.Double.TSS     Desc.Mem  0, 0, 0, 0 ; Alias for TSS to read Backlink
Exec.LDT.Double.Alias   Desc.Mem  0, 0, 0, 0 ; Alias for Faulting TSS

Exec.LDT.BadStack       EQU       $
Exec.LDT.BadStack.Stack Desc.Mem  0, 0, 0, 0 ; Stack for BadStack Fault handler
Exec.LDT.BadStack.TSS   Desc.Mem  0, 0, 0, 0 ; Alias for TSS to read Backlink
Exec.LDT.BadStack.Alias Desc.Mem  0, 0, 0, 0 ; Alias for Faulting TSS

Exec.LDT.BadTSS         EQU       $
Exec.LDT.BadTSS.Stack   Desc.Mem  0, 0, 0, 0 ; Stack for BadTSS Fault handler
Exec.LDT.BadTSS.TSS     Desc.Mem  0, 0, 0, 0 ; Alias for TSS to read Backlink
Exec.LDT.BadTSS.Alias   Desc.Mem  0, 0, 0, 0 ; Alias for Faulting TSS

; Note that this LDT cannot grow during run-time - there's other stuff directly after it!

%if (Exec.LDT.Alias-Exec.LDT) != DT.Alias
%error "Invalid Executive LDT alias"
%endif
