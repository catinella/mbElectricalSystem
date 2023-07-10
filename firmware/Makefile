
CC       = "/opt/microchip/xc8/v2.41/bin/xc8-cc"
CLAGS    = -Wall -mcpu=ATmega16 -x c -O1 -MD -MP
SYMBOLS  = -D__ATmega16__ -DXPRJ_default=default  
LDFLAGS  = -Wl,--gc-sections

DEVPACK  = $(MPLABX_HOME)/release/packs/Microchip/ATmega_DFP/3.0.158/xc8
XC8FLAGS = -ffunction-sections -fdata-sections -fshort-enums -fno-common -funsigned-char -funsigned-bitfields \
           -mno-const-data-in-config-mapped-progmem -mconst-data-in-progmem 
EXETYPE  = -gdwarf-3
INCLUDE  = -I$(DEVPACK)/avr/include -I.

#-MT build/default/production/mbElectricalSystem.o 

include Makefile.conf

.PHONY: clean

mbElectricalSystem.o:	mbElectricalSystem.c
				@echo "[ CC ] $@"
				@$(CC) $(CLAGS) $(XC8FLAGS) -c $< $(SYMBOLS) -mdfp="$(MPLABX_HOME)" $(EXETYPE) $(INCLUDE) -o $@

mbes%.o:			mbes%.c mbes%.h
				@echo "[ CC ] $@"
				@$(CC) $(CLAGS) $(XC8FLAGS) -c $< $(SYMBOLS) -mdfp="$(MPLABX_HOME)" $(EXETYPE) -MD -MP $(INCLUDE) -o $@

clean:
				@echo "[CLEAN]"
				@rm -fv *.o *.d
