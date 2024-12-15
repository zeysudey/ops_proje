cc = gcc
cflags = -Wall -g
source = calculator addition division subtraction multiplication saver
all: $(source)

%: %.c
	$(cc) $(cflags) -o $@ $<

clean:
	rm -f $(source)