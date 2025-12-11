 # CosmOS

CosmOS is a small operating system developed by me to learn more about them and to have fun doing so.

It is primarily written in C with some minimal assembly for the x86 archetecture. 

## Features
* VGA text mode printing
* Keyboard input
* Custom File System (Orion)
* Simple shell with unix-like commands
    - clear
    - exit
    - echo
    - mkfs
    - ls
    - stat
    - cat
    - cd
    - fsinfo
    - help
 
## Dependencies
To build the project, you will need the following dependencies:
* make
* i686-elf cross compiler (gcc & ld)
* grub-mkrescue
* qemu-img
* qemu-system-i386

## Quick Start
To build kernel.iso use the following command:
```bash
sudo make iso
```

Or alternatively use the following command to run the iso in qemu:
```bash
sudo make run
```

To clean up the build files, use:
```bash
sudo make clean
```

