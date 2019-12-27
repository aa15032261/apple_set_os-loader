ARCH	= x86_64

TARGET	= bootx64.efi
FORMAT 	= efi-app-$(ARCH)
OBJS	= ./lib/int_event.o ./lib/int_graphics.o ./lib/int_mem.o ./lib/int_dpath.o ./lib/int_print.o ./lib/pci_db.o bootx64.o

INC		= /usr/include/efi
CFLAGS	= -I$(INC) -I$(INC)/$(ARCH) \
		 -DGNU_EFI_USE_MS_ABI -fPIC -fshort-wchar -ffreestanding \
		 -fno-stack-protector -maccumulate-outgoing-args \
		 -Wall -D$(ARCH) -Werror -m64 -mno-red-zone

LDFLAGS	= -T /usr/lib/elf_$(ARCH)_efi.lds -Bsymbolic -shared -nostdlib -znocombreloc \
		  /usr/lib/crt0-efi-$(ARCH).o

all: $(TARGET)

bootx64.so: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name) /usr/lib/libgnuefi.a

%.efi: %.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc -S --target=$(FORMAT) $^ $@

clean:
	rm -f $(TARGET) *.so $(OBJS)
