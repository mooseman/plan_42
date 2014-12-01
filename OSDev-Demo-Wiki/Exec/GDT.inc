;
; Exec/GDT.inc
;

; The GDT is the only Descriptor Table that can store certain system tables,
; like Local Descriptor Tables and TSSes. If you think about it, that's because
; at Task Switch time, the GDT is the only place those entries will be guaranteed
; to be, regardless of which Task is currently running.
; That logic also applies to Code Segments for interrupt handlers.
; And those Interrupt handlers may need access to global Data, and the Screen...
; Everything else can be stored in each Task's Local Descriptor Table.
;
; That means that the Executive has nothing: no Code to run, no Stacks to use,
; and no Data to reference. To solve this, we give the Executive its own LDT.

                SEGMENT         GDT  VSTART=0  ALIGN=16

                USE32

; This macro makes it easy to have a one-line invocation of GDT.Alloc
; The parameters are: GDT Base, GDT Limit
%macro          GDT.Vars                2
%00             ISTRUC                  GDT.Alloc
AT GDT.Alloc.Free,         DW           0
AT GDT.Alloc.Limit,        DW           %2
AT GDT.Alloc.Pseudo.Base,  DD           %1
                IEND
%endmacro

GDT.Alias.Base  EQU             GDT.Base
GDT.Alias.Limit EQU             (GDT.Size-1) >> 12     ; Let's use Granular limits
GDT.Alias.Type  EQU             Type.Mem(Data, DPL0, RW)
GDT.Alias.Gran  EQU             Gran.Mem(Gran, Small)

GDT.Data.Base   EQU             Data.Base
GDT.Data.Limit  EQU             Data.Size - 1
GDT.Data.Type   EQU             Type.Mem(Data, DPL0, RW)
GDT.Data.Gran   EQU             Gran.Mem(Byte, Big)

GDT.VGA.Base    EQU             Dev.VGA.Base
GDT.VGA.Limit   EQU             VGA.Rows * VGA.Cols * 2 - 1
GDT.VGA.Type    EQU             Type.Mem(Data, DPL3, RW) ; Let anyone access it!
GDT.VGA.Gran    EQU             Gran.Mem(Byte, Big)

GDT.Ints.Base   EQU             Ints.Base
GDT.Ints.Limit  EQU             Ints.Limit
GDT.Ints.Type   EQU             Type.Mem(Code, DPL0, RW) ; Uses lookup tables
GDT.Ints.Gran   EQU             Gran.Mem(Byte, Def32)

GDT.Exec.LDT.Base  EQU          Exec.LDT.Base
GDT.Exec.LDT.Limit EQU          Exec.LDT.Limit
GDT.Exec.LDT.Type  EQU          Type.Sys(LDT, DPL0, 286)
GDT.Exec.LDT.Gran  EQU          Gran.Mem(Byte, Small)

GDT:
GDT.NULL        GDT.Vars        GDT.Base, GDT.Limit
GDT.Alias       Desc.Mem        GDT.Alias.Base,    GDT.Alias.Limit,    GDT.Alias.Type,    GDT.Alias.Gran
GDT.Data        Desc.Mem        GDT.Data.Base,     GDT.Data.Limit,     GDT.Data.Type,     GDT.Data.Gran
GDT.VGA         Desc.Mem        GDT.VGA.Base,      GDT.VGA.Limit,      GDT.VGA.Type,      GDT.VGA.Gran
GDT.Ints        Desc.Mem        GDT.Ints.Base,     GDT.Ints.Limit,     GDT.Ints.Type,     GDT.Ints.Gran
GDT.Exec.LDT    Desc.Mem        GDT.Exec.LDT.Base, GDT.Exec.LDT.Limit, GDT.Exec.LDT.Type, GDT.Exec.LDT.Gran

GDT.Size        EQU             8192 * x86.Desc_size

%if (GDT.Alias-GDT) != DT.Alias
%error "Invalid GDT alias"
%endif
