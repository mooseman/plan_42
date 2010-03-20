; tmr.inc   Support for 8254 timer
; Version 1.1, Nov 29, 1997
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________

TMR_CTRL	EQU	0x43	;I/O for control
TMR_CNT0	EQU	0x40	;I/O for counter 0
TMR_CNT1	EQU	0x41	;I/O for counter 1
TMR_CNT2	EQU	0x42	;I/O for counter 2

TMR_SC0		EQU	0	;Select channel 0
TMR_SC1		EQU	0x40	;Select channel 1
TMR_SC2		EQU	0x80	;Select channel 2

TMR_low		EQU	0x10	;RW low byte only
TMR_high	EQU	0x20	;RW high byte only
TMR_both	EQU	0x30	;RW both bytes

TMR_MD0		EQU	0	;Mode 0
TMR_MD1		EQU	0x2	;Mode 1
TMR_MD2		EQU	0x4	;Mode 2
TMR_MD3		EQU	0x6	;Mode 3
TMR_MD4		EQU	0x8	;Mode 4
TMR_MD5		EQU	0xA	;Mode 5

TMR_BCD		EQU	1	;BCD mode

TMR_LATCH	EQU	0	;Latch command

TMR_READ	EQU	0xF0	;Read command
TMR_CNT		EQU	0x20	;    CNT bit  (Active low, subtract it)
TMR_STAT	EQU	0x10	;    Status bit  (Active low, subtract it)
TMR_CH2		EQU	0x8	;    Channel 2 bit
TMR_CH1		EQU	0x4	;    Channel 1 bit
TMR_CH0		EQU	0x2	;    Channel 0 bit

; To initialize a counter, you OUT a byte to TMR_CTRL.  You construct that
; byte by adding:
;
;      one of TMR_SC0 .. TMR_SC2 to select the counter
; plus one of TMR_low .. TMR_both to select which bytes will be active
; plus one of TMR_MD0 .. TMR_MD5 to select the mode
; plus (optionally) TMR_BCD to enable BCD (rather than binary) counting.
;
; After the OUT to TMR_CTRL, you must do one or two (low, high or both)
; OUTs to the correct TMR_CNTn to place an initial count in the counter.
;_____________________________________________________________________________
;
; To read a counter using a latch command, OUT a byte to TMR_CTRL which
; consists of  TMR_LATCH plus one of TMR_SC0 .. TMR_SC2.  Then IN one or
; two bytes from the correct TMR_CNTn.
;
; Each channel has three parts:  A count register, which always holds the last
; initial count you wrote;  A counter, which does the actual counting;  and A
; latch.
;
; When you do a latch (or read) command, the counter is not affected.  Only
; the latch is stopped.  If you read a channel without a latch (or read)
; command, you usually get the right answer, but occasionally catch bits in
; transition (Two bits changing from 01 to 10 might look like 00 if you
; catch them at the moment of transition).
;_____________________________________________________________________________
;
; To read multiple counters and/or channel statuses, use a read command.  OUT
; to TMR_CTRL a value consisting of:
;
;                      TMR_READ
; plus one or more of  TMR_CH0 .. TMR_CH2 selecting all channels to be read
; minus one or both of TMR_CNT .. TMR_STAT to read count and/or status
;
; When reading both count and status, the first IN will be status.
;_____________________________________________________________________________
