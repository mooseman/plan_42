nasmw -f coff -o test.o test.asm -l test.lst
ld-elf -s -T elf.ld -o test.elf test.o
copy test.elf a:\
objdump-elf -x test.elf
pause

