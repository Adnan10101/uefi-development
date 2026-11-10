# UEFI Development Playground


This project uses:

- EDK II (UEFI Development Framework)
- GCC toolchain
- Ubuntu Linux
- QEMU
- OVMF firmware

### Building

Clone EDK II:

    git clone https://github.com/tianocore/edk2.git

Initialize submodules:

    cd edk2
    git submodule update --init

Build BaseTools:

    make -C BaseTools

Configure the EDK II environment and build
the individual UEFI applications.