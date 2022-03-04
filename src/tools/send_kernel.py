import sys
import os
from tqdm import tqdm

if __name__ == '__main__':

    if len(sys.argv) < 3:
        print("Usage: python send_kernel.py <kernel image path> <serial device path>")
        exit(0)

    kernel_file = open(sys.argv[1], "rb", buffering = 0)
    kernel_size = os.path.getsize(sys.argv[1])
    tty = open(sys.argv[2], "wb", buffering = 0)


    print("Sending %s (%d bytes) to %s ...\n" % (sys.argv[1], kernel_size, sys.argv[2]))

    tty.write(kernel_size.to_bytes(4, "big"))

    progress = tqdm(total=kernel_size)
    for i in range(0, kernel_size):
        tty.write(kernel_file.read())
        progress.update(1)

    tty.close()
    kernel_file.close()


