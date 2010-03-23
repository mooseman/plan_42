
nasmw -o boot.bin -f bin boot.asm -l boot.lst
C:\Nanos\Program\rawrite\rawwritewin.exe --write --drive 0 boot.bin
copy boot.lst ..\..\bochs\

