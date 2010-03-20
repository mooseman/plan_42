; showdate.asm
;
; prints the date and time to stdout
; equivalent to the following C++ program:
;
;#include <iostream.h>
;#include <time.h>
;
;int main()
;{
;    time_t t;
;    time(&t);                  // get the current time
;    cout << ctime(&t);         // convert to string and print
;    return 0;
;}
;
; written on Thu  05-11-2000  by Edward J. Beroset
;  and donated to the public domain by the author
;
; This code may be assembled and linked using Borland's TASM:
;   tasm /la /m2 showdate
;   tlink /Tdc showdate
;
STDOUT                  equ     01h     ; handle of standard output device

DOS_GET_DATE            equ     02ah    ; get system date
DOS_GET_TIME            equ     02ch    ; get system time
DOS_WRITE_HANDLE        equ     040h    ; write to handle
DOS_TERMINATE           equ     04ch    ; terminate with error code

DOSINT macro function, subfunction
        IFB <subfunction>
                mov     ah,(function AND 0ffh)
        ELSE
                mov     ax,(function SHL 8) OR (subfunction AND 0ffh)
        ENDIF
        int     21h                     ; invoke DOS function
endm


MODEL tiny
;.STACK 100h
.CODE

;****************************************************************************
;                                                                      main
;
; calls showdate routne and exists with 00 error code
;
; Entry:
;
; Exit:
;
; Trashed:
;       none
;
;****************************************************************************
main proc far
        .STARTUP                ; sets up DS and stack
        call    showdate        ;
        .EXIT 0                 ; return with errcode=0
main endp

;****************************************************************************
;                                                                  showdate
;
; fetches the DOS system date, prints it to stdout and exits
; the format of the output is identical to that of the Posix ctime()
; function:
;
;      Thu May 11 16:11:30 2000
;
; The day of week and month are always 3 characters long.  The time of
; day is in 24hour form (e.g. 16:11:30 is a few minutes after four in
; the afternoon) and the year is always four digits.  The whole thing is
; followed by a newline character (line feed = 0ah), making 25
; characters total.
;
; Note that ctime() returns 26 characters which is all of the above,
; followed by a terminating NUL char but this program does not emit a
; NUL.)
;
; Entry:
;       DS points to segment for our data tables
;
; Exit:
;       carry may be set if last write failed
;
; Trashed:
;       none
;
;****************************************************************************
showdate proc
        push    ax bx cx dx     ;
        DOSINT  DOS_GET_DATE    ;
; returns the following
;       cx = year (1980-2099)
;       dh = month (1-12) == (Jan..Dec)
;       dl = day (1-31)
;       al = day of week (0-6) == (Sun..Sat)

        push    cx              ;
        push    dx              ; save the return values

        ; write the day of week
        mov     dx, offset dayname  ;
        mov     cx,3            ; number of bytes to write
        call    WriteSubstring  ;

        ; write the month
        pop     ax              ; recall month/day
        push    ax              ; and save it again
        mov     al,ah           ; isolate just month
        mov     dx, offset monthname - 3  ; monthname array is 1-based
        mov     cx,3            ; number of bytes to write
        call    WriteSubstring  ;

        ; write the day of the month
        pop     ax              ;
        call    WriteNumber     ;
        call    WriteSpace      ;

        ; write the hour
        DOSINT  DOS_GET_TIME    ; ch = hour, cl = min,
                                ; dh = sec, dl = hundredths

        push    dx              ; save seconds
        push    cx              ; save minutes
        mov     al,ch           ;
        call    WriteNumber     ;
        call    WriteColon      ;

        ; write the minutes
        pop     ax              ;
        call    WriteNumber     ;
        call    WriteColon      ;

        ; write the seconds
        pop     ax              ;
        mov     al,ah           ;
        call    WriteNumber     ;
        call    WriteSpace      ;

        ; write the year (century first)
        pop     ax              ;
        xor     dx,dx           ; clear other reg before divide
        mov     cx,100          ; ax = ax/100, dx = remainder
        div     cx              ;
        push    dx              ; save remainder
        call    WriteNumber     ;

        ; write the year (year within century)
        pop     ax              ;
        call    WriteNumber     ;
        mov     dx,offset newlinechar
        call    PrintOne        ;
        pop     dx cx bx ax     ; restore stack
        ret                     ;
showdate endp

;****************************************************************************
;                                                            WriteSubstring
;
; writes a short substring to stdout
; specifically, prints CL characters, starting at DS:(DX+CL*AL)
;
; Entry:
;       DS:DX ==> pointer to base of string array
;       CL    =   size of each string
;       AL    =   string selector (i.e. which string)
;
; Exit:
;       CY set if there was an error writing last byte
;       if CY clear,
;               AX = 1 (number of bytes written)
;       else
;               AX = error code
;
; Trashed:
;       BX CX DX
;
;****************************************************************************
WriteSubstring proc
        mul     cl              ; ax = cl * al
        add     dx,ax           ; offset now points to appropriate day string
        call    PrintIt         ;
WriteSubstring endp
        ; deliberately fall through
;****************************************************************************
;                                                                WriteSpace
;
; writes a single space character (20h) to stdout
;
; Entry:
;       DS points to data table segment
;
; Exit:
;       CY set if there was an error writing last byte
;       if CY clear,
;               AX = 1 (number of bytes written)
;       else
;               AX = error code
;
; Trashed:
;       BX CX DX
;
;****************************************************************************
WriteSpace proc
        mov     dx,offset spacechar;
WriteSpace endp
        ; deliberately fall through
;****************************************************************************
;                                                                  PrintOne
;
; prints a single character pointed to by DS:DX
;
; Entry:
;       DS:DX ==> points to the character to be printed
;
; Exit:
;       CY set if there was an error writing last byte
;       if CY clear,
;               AX = 1 (number of bytes written)
;       else
;               AX = error code
;
; Trashed:
;       BX CX DX
;
;****************************************************************************
PrintOne proc
        mov     cx,1            ;
PrintOne endp
        ; deliberately fall through
;****************************************************************************
;                                                                   PrintIt
;
; prints the passed string to stdout
;
; Entry:
;       DS:DX ==> points to string to be printed
;       CX    =   number of bytes to be printed
;
; Exit:
;       CY set if there was an error writing to stdout
;       if CY clear,
;               AX = number of bytes written
;       else
;               AX = error code
;
; Trashed:
;       none
;
;****************************************************************************
PrintIt proc
        mov     bx,STDOUT       ;
        DOSINT  DOS_WRITE_HANDLE  ; write to the file
        ret                     ;
PrintIt endp
              
;****************************************************************************
;                                                                WriteColon
;
; writes a colon character to stdout
;
; Entry:
;       DS points to data segment
;
; Exit:
;       CY set if there was an error writing to stdout
;       if CY clear,
;               AX = 1 (number of bytes written)
;       else
;               AX = error code
;
; Trashed:
;       none
;
;****************************************************************************
WriteColon proc
        mov     dx,offset colonchar;
        jmp     PrintOne        ;
WriteColon endp

;****************************************************************************
;                                                               WriteNumber
;
; prints the number in AL to stdout as two decimal digits
;
; Entry:
;       AL      = number to be printed.  It must be in the range 00-99
;
; Exit:
;       CY set if there was an error writing to stdout
;       if CY clear,
;               AX = 2 (number of bytes written)
;       else
;               AX = error code
;
; Trashed:
;       BX CX DX
;
;****************************************************************************
WriteNumber proc
        xor     ah,ah           ; clear out high half
        mov     cl,10           ; prepare to convert to decimal (base 10)
        div     cl              ; divide it out
        or      ax,3030h        ; convert to ASCII digits
        push    ds              ; remember DS for later
        push    ax              ; push converted chars on stack
        mov     dx,ss           ;
        mov     ds,dx           ; ds = ss
        mov     dx,sp           ; print data from stack
        mov     cx,2            ; two characters only
        call    PrintIt         ;
        pop     bx              ; fix stack
        pop     ds              ; restore ds pointer
        ret                     ;
WriteNumber endp

;.DATA
        dayname         db "SunMonTueWedThuFriSat"
        monthname       db "JanFebMarAprMayJunJulAugSepOctNovDec"
        spacechar       db " "
        colonchar       db ":"
        newlinechar     db 0ah   ; in C this is \n

end
