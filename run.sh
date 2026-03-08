qemu-system-i386 \
  -debugcon stdio \
  -machine q35,smbus=off \
  -cpu pentium3 \
  -m 128M \
  -nodefaults \
  -device ich9-ahci,id=ahci \
  -drive file=build/diskimage.dd,id=disk0,format=raw,if=none \
  -device ide-hd,drive=disk0,bus=ahci.0 \
  -vga std