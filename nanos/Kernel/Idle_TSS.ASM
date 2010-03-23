.link:   dw   0    ;Task Link
         dw   0    ;reserved

         dd   0    ;esp0
         dw   0    ;ss0
         dw   0    ;reserved
         dd   0    ;esp1
         dw   0    ;ss1
         dw   0    ;reserved
         dd   0    ;esp2
         dw   0    ;ss2
         dw   0    ;reserved

         dd   (mem_PD << 12) ;cr3/PDBR

         dd   kernel_start	;eip = start
         dd   202h    ;eflags

         dd   0    ;eax
         dd   0    ;ecx
         dd   0    ;edx
         dd   0    ;ebx

         dd   0FFCh ;esp
         dd   0    ;ebp

         dd   0    ;esi
         dd   0    ;edi

         dw   0    ;es
         dw   0    ;    Reserved
         dw   kern_sel ;cs
         dw   0    ;    Reserved
         dw   Stack_sel;ss
         dw   0    ;    Reserved
         dw   0    ;ds
         dw   0    ;    Reserved
         dw   0    ;fs
         dw   0    ;    Reserved
         dw   0    ;gs
         dw   0    ;    Reserved
         dw   0    ;ldt
         dw   0    ;    Reserved

         dw   0    ;trap(bit0)
         dw   (.iobase - .tss)    ;IO map Base Address
.iobase:

;error checking
times    68h-(.iobase-.tss)  nop
times    (.iobase-.tss)-68h  nop
;both should result in 0 bytes of nop