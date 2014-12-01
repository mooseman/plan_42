;
; User/Data.inc
;

; This module defines the Data Segment for the User Code. There will be one copy
; of this for every running program.

                  SEGMENT       User.Data  START=0  ALIGN=16  NOBITS

                  USE32

User.Data.Left    RESB          1               ; Left X co-ordinate
User.Data.Top     RESB          1               ; Top Y co-ordinate
User.Data.Width   RESB          1               ; Width of area
User.Data.Height  RESB          1               ; Height of area
User.Data.Row     RESD          1               ; Delta from one row to the next
