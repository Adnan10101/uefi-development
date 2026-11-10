### Creating and running a program in the UEFI Framework for my reference 


mkdir -p <folder>
create the .c and .inf files

then source edksetup.sh

add the following inside MdeModulePkg/MdeModulePkg.dsc:
Under [Components]
<folder>/<.inf file>

build \
  -a X64 \
  -t GCC \
  -m <folder>/<inf file> \
  -p MdeModulePkg/MdeModulePkg.dsc

if successful the following command will run:

find Build -name <folder/.efi file created>

exisiting QEMU setup:

mkdir -p /tmp/efi-boot/EFI/BOOT

cp Build/MdeModule/DEBUG_GCC/X64/<.efi file> /tmp/efi-boot/EFI/BOOT/BOOTX64.EFI

<!-- cp /usr/share/OVMF/OVMF_VARS_4M.fd ./OVMF_VARS.fd

qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
  -drive if=pflash,format=raw,file=./OVMF_VARS.fd \
  -drive file=fat:rw:/tmp/efi-boot,format=raw \
  -net none -->



qemu-system-x86_64   -bios /usr/share/ovmf/OVMF.fd   -drive file=fat:rw:/tmp/efi-boot,format=raw   -net none -serial stdio