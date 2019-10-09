CC          = powerpc-eabi-gcc
LD          = $(CC)
OBJCOPY     = powerpc-eabi-objcopy
CFILES      = *.c
SRCDIR		= src
OBJDIR 		= obj
BINDIR		= bin
VC_VERSIONS	= NACJ NACE NARJ NARE
NAME		= homeboy
RESDESC		= res.json

ADDRESS     = 0x90000800
ALL_CFLAGS      = -c -mcpu=750 -meabi -mhard-float -G 0 -O3 -ffunction-sections -fdata-sections $(CFLAGS)
ALL_CPPFLAGS	= $(CPPFLAGS)
ALL_LDFLAGS     = -T build.ld -G 0 -nostartfiles -specs=nosys.specs -Wl,--section-start,.init=$(ADDRESS) $(LDFLAGS)
ALL_OBJCOPYFLAGS	= -S -O binary --set-section-flags .bss=alloc,load,contents $(OBJCOPYFLAGS)

HOMEBOY			= $(foreach v,$(VC_VERSIONS),hb-$(v))

HB-NACJ		= $(COBJ-hb-NACJ) $(ELF-hb-NACJ)
HB-NACE		= $(COBJ-hb-NACE) $(ELF-hb-NACE)
HB-NARJ		= $(COBJ-hb-NARJ) $(ELF-hb-NARJ)
HB-NARE		= $(COBJ-hb-NARE) $(ELF-hb-NARE)

all			: $(HOMEBOY)
clean       :
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY		: all clean

define bin_template
SRCDIR-$(1)      = src/$(2)
OBJDIR-$(1)      = obj/$(1)
BINDIR-$(1)      = bin/$(1)
CSRC-$(1)       := $$(foreach s,$$(CFILES),$$(wildcard $$(SRCDIR-$(1))/$$(s)))
COBJ-$(1)        = $$(patsubst $$(SRCDIR-$(1))/%,$$(OBJDIR-$(1))/%.o,$$(CSRC-$(1)))
ELF-$(1)         = $$(BINDIR-$(1))/$(2).elf
BIN-$(1)         = $$(BINDIR-$(1))/$(2).bin
OUTDIR-$(1)      = $$(OBJDIR-$(1)) $$(BINDIR-$(1))
BUILD-$(1)		 = $(1)
CLEAN-$(1)		 = clean-$(1)
$$(ELF-$(1))		 : LDFLAGS += -Wl,--defsym,init=$$(ADDRESS)
$$(BUILD-$(1))       : $$(BIN-$(1))
$$(CLEAN-$(1))       :
	rm -rf $$(OUTDIR-$(1))

$$(COBJ-$(1))     : $$(OBJDIR-$(1))/%.o: $$(SRCDIR-$(1))/% | $$(OBJDIR-$(1))
	$(CC) $$(ALL_CPPFLAGS) $$(ALL_CFLAGS) $$< -o $$@
$$(SOBJ-$(1))		: $$(OBJDIR-$(1))/%.o: $$(SRCDIR-$(1))/% | $$(OBJDIR-$(1))
	$(AS) -c -MMD -MP $$(ALL_CPPFLAGS) $$< -o $$@
$$(RESOBJ-$(1))		: $$(OBJDIR-$(1))/%.o: $$(RESDIR-$(1))/% | $$(OBJDIR-$(1))
	$(GRC) $$< -d $(RESDESC) -o $$@
$$(ELF-$(1))      : $$(COBJ-$(1)) | $$(BINDIR-$(1))
	$(LD) $$(ALL_LDFLAGS) $$^ -o $$@
$$(BIN-$(1))      : $$(ELF-$(1)) | $$(BINDIR-$(1))
	$(OBJCOPY) $$(ALL_OBJCOPYFLAGS) $$< $$@
$$(OUTDIR-$(1))   :
	mkdir -p $$@
endef

$(foreach v,$(VC_VERSIONS),$(eval $(call bin_template,hb-$(v),homeboy)))

$(HB-NACJ)  	: CPPFLAGS	?=	-DVC_VERSION=NACJ
$(HB-NACE)  	: CPPFLAGS	?=	-DVC_VERSION=NACE
$(HB-NARJ)  	: CPPFLAGS	?=	-DVC_VERSION=NARJ
$(HB-NARE)  	: CPPFLAGS	?=	-DVC_VERSION=NARE