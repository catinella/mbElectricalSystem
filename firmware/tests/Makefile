

SRCs := $(shell ls test[0-9]*.c)
EXEs := $(SRCs:%.c=%)

INCLUDE = -I../

FAKE_CONSOLE := $(shell tty)


.PHONY: clean cleanall

all:				$(EXEs)

#-------------------------------------------------------------------------------------------------------------------------------
#                                   S O U R C E   F I L E S   U N D E R   T E S T
#-------------------------------------------------------------------------------------------------------------------------------
mbesSerialConsole-0.o:	../mbesSerialConsole.c ../mbesSerialConsole.h
				@echo "[ CC ] $@"
				@gcc -Wall -g -c $< $(INCLUDE) "-DFAKE_CONSOLE=\"$(FAKE_CONSOLE)\"" -o $@

mbesUtilities-0.o:	../mbesUtilities.c ../mbesUtilities.h
				@echo "[ CC ] $@"
				@gcc -Wall -g -c $< $(INCLUDE) -o $@

#-------------------------------------------------------------------------------------------------------------------------------

test0-mbes%.o:		test0-mbes%.c mbes%-0.o mbesSerialConsole-0.o
				@echo "[ CC ] $@"
				@gcc -Wall -g -c $< $(INCLUDE) -o $@

test0-mbes%:		test0-mbes%.o mbes%-0.o mbesSerialConsole-0.o
				@echo "[ LD ] $@"
				@gcc -Wall $^ -o $@

clean:
				@echo "[CLEAN]"
				@rm -fv *.o

cleanall:			clean
				@rm -fv $(EXEs)

debug:
				@echo "-->$(FAKE_CONSOLE)"
