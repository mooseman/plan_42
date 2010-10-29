%macro disp 2
    mov  cx, %2 - %1
    mov  si, %1
    call printstring
%endmacro
