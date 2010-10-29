echo off
cls
echo nasm:
nasmw -o elf_load.bin -l elf_load.lst -f bin elf_load.asm
echo copy:
copy elf_load.bin a:\
echo bochs:
copy elf_load.lst ..\..\Bochs
pause