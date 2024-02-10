
sources=$(wildcard *.c)
objects=$(sources:.c=.o)

all: $(objects)

%.o: %.c net_utils.h common.h
	$(CC) -o $@ -c $<

clean:
	@rm *.o

.PHONY: clean
