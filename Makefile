CC = gcc
CFLAGS = -Wall -O3
AR = ar

ifdef DJDIR
	# djgpp.
	OBJS = djgpp.o
	EXE = .exe
else
	# Assume Linux.
	OBJS = linux.o
	EXE = 
endif

LIBCDA = libcda.a
EXAMPLE = example$(EXE)

all: $(LIBCDA) $(EXAMPLE)

$(LIBCDA): $(OBJS)
	$(AR) rs $@ $^

$(EXAMPLE): example.o $(LIBCDA)
	$(CC) -o $@ $^

clean:
	rm -f $(LIBCDA) $(OBJS) 
	rm -f $(EXAMPLE) example.o
	rm -f *~	
