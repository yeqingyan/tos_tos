#~/qemu-tos/arm-softmmu/qemu-system-arm -S -s -M raspi -cpu arm1176 -kernel kernel.img -serial /dev/null -device usb-kbd
~/qemu-tos/arm-softmmu/qemu-system-arm -S -s -M raspi -cpu arm1176 -kernel kernel.img -serial /dev/null -device usb-kbd -chardev file,path=/media/sf_Raspi/tos/fake_char,id=fakechar -device usb-serial,chardev=fakechar
