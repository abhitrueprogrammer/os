# Makefile for building a simple i686 OS kernel

.PHONY: all buildenv build-i686 run clean

all: build-i686

# 1. Build the Docker image
buildenv:
	sudo docker build buildenv -t techiekeith/osdev-buildenv-i686

# 2. Run build inside container
build-i686: buildenv
	sudo docker run --rm -it -v $(PWD):/root/env techiekeith/osdev-buildenv-i686 make kernel-build

# 3. Build kernel using cross-compiler inside container
kernel-build: boot.o kernel.o scheduler.o link check

boot.o: boot.s
	i686-elf-as boot.s -o boot.o

kernel.o: kernel.c scheduler.h
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

scheduler.o: scheduler.c scheduler.h
	i686-elf-gcc -c scheduler.c -o scheduler.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

link:
	i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o scheduler.o -lgcc

check:
	@if grub-file --is-x86-multiboot myos.bin; then \
	  echo "multiboot confirmed"; \
	else \
	  echo "the file is not multiboot"; \
	fi

#  QEMU run target
run:
	qemu-system-i386 -kernel myos.bin

# Clean object files and binary
clean:
	rm -f boot.o kernel.o scheduler.o myos.bin
