nasmw -f coff -o syslib.o syslib.asm
gcc -ffreestanding -c -o init.o init.c -m32 -msse2 -mno-rtd
ld -n -Ttext 0 --oformat coff-go32 -o module.o syslib.o init.o
pause
