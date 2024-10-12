
srcs := $(shell ls *_test.c)
exes := $(srcs:.c=)

INCOPTS = -I. -I../

ifeq ($(GDB), 1)
	CCOPTS = -o0 -g
else
	CCOPTS = -o3
endif

.PHONY: all clean cleanall

all:	$(exes)

%.o:		%.c ../werror.h
		@echo "[ CC ] $@"
		@gcc -Wall $(CCOPTS) $(INCOPTS) -c $< -o $@

%_test:	%_test.o %.o
		@echo "[ LD ] $@"
		@gcc -Wall $(CCOPTS) $^ -o $@

%.o:		../%.c ../%.h
		@echo "[ CC* ] $@"
		@gcc -Wall $(CCOPTS) $(INCOPTS) -c $< -I. -o $@

clean:
		@echo "[CLEAN]"
		@rm -fv *.o

cleanall:	clean
		@rm -fv $(exes)
	
		
