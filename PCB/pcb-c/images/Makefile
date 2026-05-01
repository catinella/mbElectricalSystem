# Makefile

#
# If you installed Rust locally, don't forget to load the environment
# source "$HOME/.cargo/env"
#

SRCs   := $(shell ls *.asciiDiag)
TGTs   := $(SRCs:.asciiDiag=.svg)
SVGBOB  = svgbob_cli

all:		$(TGTs)

%.svg:	%.asciiDiag
		@echo "[GEN] $@"
		@$(SVGBOB) $< > $@

clean:
		@rm -fv $(TGTs)
