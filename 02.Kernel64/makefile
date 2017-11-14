############################
# 빌드 환경, 규칙설정
############################
NASM64 = nasm -f elf64
GCC64  = x86_64-pc-linux-gcc -c -m64 -ffreestanding
LD64   = x86_64-pc-linux-ld -melf_x86_64 -T ../elf_x86_64.x -nostdlib -e Main -Ttext 0x200000
OBJCOPY64 = x86_64-pc-linux-objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJECTDIRECTORY = Temp
SOURCEDIRECTORY = Source

###########################
# 빌드 항목, 빌드 방법 설정
# #########################
all: prepare Kernel64.bin

prepare:
	mkdir -p $(OBJECTDIRECTORY)

dep:
	@echo === Make Dependency File ===
	make -C $(OBJECTDIRECTORY) -f ../makefile Kernel64.elf
	@echo === Dependency Search Complete ===

ExecuteInternalBuild: dep
	make -C $(OBJECTDIRECTORY) -f ../makefile Kernel64.elf

Kernel64.bin: ExecuteInternalBuild
	$(OBJCOPY64) $(OBJECTDIRECTORY)/Kernel64.elf $@

clean:
	rm -f *.bin
	rm -f $(OBJECTDIRECTORY)/*.*

#################################################
# Make에 의해 다시 호출됨. Temp를 기준으로 수행됨. 
# ##############################################
# 빌드할 어셈블리어 엔트리 포인트 소스파일 정의. Temp 디렉터리 기준.
ENTRYPOINTSOURCEFILE = ../$(SOURCEDIRECTORY)/EntryPoint.s
ENTRYPOINTOBJECTFILE = EntryPoint.o
# 빌드할 C 소스파일 정의. Temp파일 기준.
CSOURCEFILE = $(wildcard ../$(SOURCEDIRECTORY)/*.c)
ASSEMBLYSOURCEFILES = $(wildcard ../$(SOURCEDIRECTORY)/*.asm)
COBJECTFILE = $(notdir $(patsubst %.c,%.o,$(CSOURCEFILE)))
ASSEMBLYOBJECTFILES = $(notdir $(patsubst %.asm,%.o,$(ASSEMBLYSOURCEFILES)))

# 어셈블리어 엔트리 포인트 빌드
$(ENTRYPOINTOBJECTFILE): $(ENTRYPOINTSOURCEFILE)
	$(NASM64) -o $@ $<

# .c -> o
%.o: ../$(SOURCEDIRECTORY)/%.c
	$(GCC64) -c $<

# .asm -> .o
%.o: ../$(SOURCEDIRECTORY)/%.asm
	$(NASM64) -o $@ $<

InternelDependency:
	$(GCC64) -MM $(SOURCEDIRECTORY) > Dependency.dep

Kernel64.elf: $(ENTRYPOINTOBJECTFILE) $(COBJECTFILE) $(ASSEMBLYOBJECTFILES)
	$(LD64) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif
