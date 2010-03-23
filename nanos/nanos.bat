echo off
cls
cd loader
echo loader:
nasmw -f coff -o loader.o loader.asm -l loader.lst
cd ..
cd kernel
echo kernel:
nasmw -f coff -o kernel.o kernel.asm -l kernel.lst
cd ..
echo list:
copy kernel\kernel.lst ..\bochs\
copy loader\loader.lst ..\bochs\
echo jloc:
jloc.exe jloc.txt nanos.bin nanos.map
pause
echo copy:
copy nanos.bin a:
pause