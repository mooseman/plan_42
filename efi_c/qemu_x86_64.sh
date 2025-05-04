#!/usr/bin/env bash
set -e

IMG=$1        # ../build/sea-dos.img
CODE=$2       # /usr/share/OVMF/OVMF_CODE_4M.fd
VARS=/usr/share/OVMF/OVMF_VARS_4M.fd

qemu-system-x86_64 \
  -machine q35 \
  -m 256M \
  -drive if=pflash,format=raw,unit=0,readonly=on,file="$CODE" \
  -drive if=pflash,format=raw,unit=1,file="$VARS" \
  -drive if=ide,media=disk,format=raw,file="$IMG" \
  -serial stdio \
  -vga std -display gtk,gl=on,zoom-to-fit=off,window-close=on \
  -rtc base=localtime -net none


