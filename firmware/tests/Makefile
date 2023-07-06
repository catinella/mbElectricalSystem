

CCOPTS  = -Wall -g

SRCs   := $(shell ls test[0-9]*.c)
EXEs   := $(SRCs:%.c=%)

INCLUDE = -I../

FAKE_CONSOLE := $(shell tty)


.PHONY: clean cleanall

all:				$(EXEs)

#-------------------------------------------------------------------------------------------------------------------------------
#                                        C U S T O M I Z E D   R U L E S
#-------------------------------------------------------------------------------------------------------------------------------
virtualSelectors:		virtualSelectors.o
				@echo "[ LD* ] $@"
				@gcc -Wall $< -o $@

virtualSelectors.o:	virtualSelectors.c
				@echo "[ CC* ] $@"
				@gcc $(CCOPTS) -c $(INCLUDE) $< -o $@

test0-virtualSelectors.o:	test0-virtualSelectors.c
					@echo "[ CC* ] $@"
					@gcc $(CCOPTS) -c $(INCLUDE) $< -o $@
	
test0-virtualSelectors:	test0-virtualSelectors.o
				@echo "[ LD* ] $@"
				@gcc -Wall $< -o $@

#-------------------------------------------------------------------------------------------------------------------------------
#                                   S O U R C E   F I L E S   U N D E R   T E S T
#-------------------------------------------------------------------------------------------------------------------------------
mbesSerialConsole-0.o:	../mbesSerialConsole.c ../mbesSerialConsole.h
				@echo "[ CC- ] $@"
				@gcc $(CCOPTS) -c $< $(INCLUDE) "-DFAKE_CONSOLE=\"$(FAKE_CONSOLE)\"" -DMOCK=1 -o $@

mbesUtilities-0.o:	../mbesUtilities.c ../mbesUtilities.h
				@echo "[ CC- ] $@"
				@gcc $(CCOPTS) -c $< $(INCLUDE) -DMOCK=1 -o $@

mbesSelector-0.o:		../mbesSelector.c ../mbesSelector.h
				@echo "[ CC- ] $@"
				@gcc $(CCOPTS) -c $< $(INCLUDE) -DMOCK=1 -o $@

#-------------------------------------------------------------------------------------------------------------------------------
#                                      R U L E S   F O R   T E S T 0   F I L E S
#-------------------------------------------------------------------------------------------------------------------------------

test0-mbes%.o:		test0-mbes%.c mbes%-0.o mbesSerialConsole-0.o
				@echo "[ CC ] $@"
				@gcc $(CCOPTS) -c $< $(INCLUDE) -DMOCK=1 -o $@

test0-mbes%:		test0-mbes%.o mbes%-0.o mbesSerialConsole-0.o
				@echo "[ LD ] $@"
				@gcc -Wall $^ -o $@

#-------------------------------------------------------------------------------------------------------------------------------
#                                        S T A N D A R D   G L O B A L   R U L E S
#-------------------------------------------------------------------------------------------------------------------------------
clean:
				@echo "[CLEAN]"
				@rm -fv *.o

cleanall:			clean
				@rm -fv $(EXEs) virtualSelectors

debug:
				@echo ""
