# kfs_2


# Fundamentals:

# Booting process in linux and GRUB:

    https://github.com/nu11secur1ty/All-Stages-of-Linux-Booting-Process-Architecture 
    https://en.wikipedia.org/wiki/GNU_GRUB

# Making a cross compiler:

    https://wiki.osdev.org/GCC_Cross-Compiler

# KFS x86

    https://wiki.osdev.org/Bare_Bones

# Multiboot2 header specification and implementation

    https://nongnu.askapache.com/grub/phcoder/multiboot.pdf 
    https://intermezzos.github.io/book/first-edition/multiboot-headers.html

# Steps:

    In a linux machine, set up a cross-compiler for x86 architecture.
    If you are using NASM, you should build it aswell (does not come with binutils).(can use nasm of the host)
    Create a multiboot header based on the grub specification.
    Create C code that write to the screen and compile it.
    Create Linker script that will link the object files, generate and align the sections.
    Compile and Link with appropriate flags to get kernel binary
    Use grub-mkrescue to generate a bootable file .iso


Here are the exact command-line steps, one by one, for downloading the source code of Binutils and GCC, into a $HOME/src directory:

1. Create the source directory:

```
    mkdir -p $HOME/src
    cd $HOME/src
```

2. Download Binutils 2.41:

```
    curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz
```

3. Download GCC 13.2.0:

```
    curl -O https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz
```

4. Extract both archives

```
    tar -xf binutils-2.41.tar.xz
    tar -xf gcc-13.2.0.tar.xz
```


Set up environment variables (manually for each terminal session):

```
    export TARGET=i686-elf
    export PREFIX="$HOME/opt/cross"
    export PATH="$PREFIX/bin:$PATH"
```

🛠 Step 1: Build and Install Binutils
Create a separate build directory for Binutils:

```
    mkdir -p $HOME/src/binutils-build
    cd $HOME/src/binutils-build
```

Configure Binutils:

```
    ../binutils-2.41/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
```

Build Binutils:

```
    make
```

Install Binutils:

```
    make install
```

You now have i686-elf-as, i686-elf-ld, etc., installed.

Step 2: Build and Install GCC (minimal C only, no headers yet)
Download prerequisites (from within the GCC source dir):

```
    cd $HOME/src/gcc-13.2.0
    ./contrib/download_prerequisites
```

Create a separate build directory for GCC:

```
    mkdir -p $HOME/src/gcc-build
    cd $HOME/src/gcc-build
```

Configure GCC:

```
    ../gcc-13.2.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers
```

Build the minimal cross-compiler:

```
    make all-gcc
```

Build libgcc (without libc headers):

```
    make all-target-libgcc
```

Install both:

```
    make install-gcc
    make install-target-libgcc
```

Test It

```
    $PREFIX/bin/i686-elf-gcc --version
````

You should see something like i686-elf-gcc (GCC) 13.2.0.


sudo /home/obeaj/opt/cross/bin/i686-elf-as boot.s -o boot.o
sudo /home/obeaj/opt/cross/bin/i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
sudo /home/obeaj/opt/cross/bin/i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc



export PATH="$HOME/opt/cross/bin:$PATH"
source ~/.bashrc