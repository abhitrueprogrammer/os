# OS Project

![Terminal Screenshot](screenshots/terminal.png)

## Prerequisites

Before running this project, make sure you have the following installed on your Linux system:

- **Docker** - Used to build the cross-compilation environment using [gcc-cross-i686-elf](https://hub.docker.com/r/techiekeith/gcc-cross-i686-elf) docker image
- **QEMU** - Required to run the OS kernel (`qemu-system-i386`)
- **GRUB tools** - For multiboot verification

## How to Run

To run the project run the following commands in a Linux OS:

```bash
make
make run
```

## What it does

1. `make` - Builds a Docker container with the i686 cross-compilation toolchain and compiles the kernel
2. `make run` - Runs the compiled kernel using QEMU emulator

## Build Process

The build process involves:

1. Creating a Docker environment with i686-elf cross-compiler
2. Assembling the boot loader (`boot.s`)
3. Compiling the kernel (`kernel.c`)
4. Linking everything together with the custom linker script
5. Verifying multiboot compliance
