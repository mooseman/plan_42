nasm -g -f obj -dTINY=1 lib.asm
tcc -v -mt -w -O2 -d -Z -c mbload.c
tlink /3/v/x/c c:\tc\lib\c0t.obj mbload.obj lib.obj,mbload.exe,,c:\tc\lib\cs.lib
