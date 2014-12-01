;
; x86/Desc.inc
;

; Descriptor Tables hold Descriptor Table Entries, or Descriptors for short.
; To select a Descriptor, you load a Segment register with a Selector (see Seg.inc).

; The following is the generic structure of a Descriptor (not very useful!)
                STRUC           x86.Desc
                RESW            1
                RESW            1
                RESB            1
.Type           RESB            1       ; All Descriptors have a .Type field here
                RESW            1
                ENDSTRUC                ; All Descriptors have the same Desc_size

; The following is the structure of a System Descriptor
                STRUC           x86.Desc.Sys
.OffsetLo       RESW            1
.Selector       RESW            1
.Args           RESB            1
.Type           RESB            1
.OffsetHi       RESW            1
                ENDSTRUC

; The following is the structure of a Memory (Code/Data/Stack) Descriptor
                STRUC           x86.Desc.Mem
.LimitLo        RESW            1
.BaseLo         RESW            1
.BaseMid        RESB            1
.Type           RESB            1
.Gran           RESB            1
.BaseHi         RESB            1
                ENDSTRUC

; The .Type field in both .Sys and .Mem structures above have the same format.
; These are the bitfields of the .Type field
x86.Desc.Type.Present EQU       1000_0000b

; These are the different Descriptor Privilege Levels
x86.Desc.Type.DPL3    EQU       0110_0000b
x86.Desc.Type.DPL2    EQU       0100_0000b
x86.Desc.Type.DPL1    EQU       0010_0000b
x86.Desc.Type.DPL0    EQU       0000_0000b

x86.Desc.Type.Sys     EQU       0000_0000b
x86.Desc.Type.Mem     EQU       0001_0000b

x86.Desc.Type.Type    EQU       0000_1111b

;-------------------------------------------------------------------------------

; These are the .Type.Type field values and modifiers for System Descriptors
x86.Desc.Sys.Type.TSS   EQU     0000_0001b      ; Task State Segment
x86.Desc.Sys.Type.LDT   EQU     0000_0010b      ; Local Descriptor Table
x86.Desc.Sys.Type.Busy  EQU     0000_0011b      ; Busy TSS
x86.Desc.Sys.Type.Gate  EQU     0000_0111b      ; One of a number of Gates:
x86.Desc.Sys.Type.Call  EQU     0000_0100b        ; Call Gate (switches PL)
x86.Desc.Sys.Type.Task  EQU     0000_0101b        ; Task Gate (switches TSS)
x86.Desc.Sys.Type.Int   EQU     0000_0110b        ; Int Gate (disables interrupts)
x86.Desc.Sys.Type.Trap  EQU     0000_0111b        ; Trap Gate
x86.Desc.Sys.Type.386   EQU     0000_1000b      ; '386 variant of above
x86.Desc.Sys.Type.286   EQU     0000_0000b      ; '286 is the original ones
; The .386 modifier is used for 32-bit variants of the original '286 16-bit ones.
; Note it cannot be used for either .LDT or .Task

;-------------------------------------------------------------------------------

; These are the .Type.Type field values and modifiers for Memory Descriptors
x86.Desc.Mem.Type.Code     EQU  0000_1000b
x86.Desc.Mem.Type.Data     EQU  0000_0000b
x86.Desc.Mem.Type.Conform  EQU  0000_0100b ; Code can be executed at different PLs
x86.Desc.Mem.Type.Expand   EQU  0000_0100b ; Base is TOP, Limit is size BELOW that
x86.Desc.Mem.Type.Stack    EQU  0000_0100b ; (Best used for Stacks)
x86.Desc.Mem.Type.RW       EQU  0000_0010b ; Readable for Code, Writable for Data
x86.Desc.Mem.Type.NoRW     EQU  0000_0000b ; No Read for Code, no Write for Data
x86.Desc.Mem.Type.Accessed EQU  0000_0001b ; Set when Descriptor loaded

; These are the bitfields of the .Gran field in Memory Descriptors
x86.Desc.Mem.Gran.LimitHi EQU   0000_1111b

x86.Desc.Mem.Gran.Gran  EQU     1000_0000b ; Use Page-granular Limit
x86.Desc.Mem.Gran.Byte  EQU     0000_0000b ; Use byte-granular Limit

x86.Desc.Mem.Gran.Big   EQU     0100_0000b ; Use EIP and ESP for Code and Stacks
x86.Desc.Mem.Gran.Small EQU     0000_0000b ; Use IP and SP for Code and Stacks

x86.Desc.Mem.Gran.Def32 EQU     0100_0000b ; Default to 32-bit instructions
x86.Desc.Mem.Gran.Def16 EQU     0000_0000b ; Default to 16-bit instructions

x86.Desc.Mem.Gran.Avail EQU     0001_0000b ; Available for system use

;-------------------------------------------------------------------------------

; Helper macros for each of the .Types defined above. There are a number of
; fields that need to be set, making for long source lines, and it's easy to
; forget one! Each of these assume only one thing: they're .Present

; Parameters are: TSS or LDT or Call or Task or Int or Trap, DPL 0-3, 286 or 386
%define         Type.Sys(Type, DPL, x86) (\
                x86.Desc.Type.Present | x86.Desc.Type.Sys | \
                x86.Desc.Type.%+ DPL | x86.Desc.Sys.Type.%+ Type | x86.Desc.Sys.Type.%+ x86 \
                )

; Parameters are: Code or Data or Stack, DPL 0-3, and RW or NoRW
%define         Type.Mem(Type, DPL, RW) (\
                x86.Desc.Type.Present | x86.Desc.Type.Mem | \
                x86.Desc.Type.%+ DPL | x86.Desc.Mem.Type.%+ Type | x86.Desc.Mem.Type.%+ RW \
                )

; Parameters are: Gran or Byte, and Big or Small / Def32 or Def16
%define         Gran.Mem(Gran, Size) (\
                x86.Desc.Mem.Gran.%+ Gran | x86.Desc.Mem.Gran.%+ Size \
                )

; A helper macro, making it easy to allocate a System Descriptor in a Table.
; Unfortunately this macro requires that the Offset is less than 64K.
; Tne parameters are: Selector, Offset and Type
%macro          Desc.Sys                   3
%00             ISTRUC x86.Desc.Sys
AT x86.Desc.Sys.OffsetLo,  DW              %2
AT x86.Desc.Sys.Selector,  DW              %1
AT x86.Desc.Sys.Args,      DB              0
AT x86.Desc.Sys.Type,      DB              %3
AT x86.Desc.Sys.OffsetHi,  DW              0
                IEND
%endmacro

; A helper macro, making it easy to allocate a Memory Descriptor in a Table.
; The parameters are: Base, Limit, Type and Gran
%macro          Desc.Mem                   4
%00             ISTRUC x86.Desc.Mem
AT x86.Desc.Mem.LimitLo,   DW              (%2 >> 0) & 0FFFFh
AT x86.Desc.Mem.BaseLo,    DW              (%1 >> 0) & 0FFFFh
AT x86.Desc.Mem.BaseMid,   DB              (%1 >> 16) & 0FFh
AT x86.Desc.Mem.Type,      DB              %3
AT x86.Desc.Mem.Gran,      DB              %4 | ((%2 >> 16) & 0Fh)
AT x86.Desc.Mem.BaseHi,    DB              (%1 >> 24) & 0FFh
                IEND
%endmacro
