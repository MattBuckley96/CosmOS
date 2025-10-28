# CosmOS

A small OS that I am creating for fun and to learn more about them

## Features
* VGA text mode printing
* Keyboard input
* Simple shell with built in commands
    - clear
    - exit
    - echo

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

