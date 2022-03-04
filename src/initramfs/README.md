# Create cpio archive

```
rm initramfs.cpio
cd rootfs
find . | cpio -o -H newc > ../initramfs.cpio
cd ..
```