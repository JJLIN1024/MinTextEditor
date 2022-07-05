CC = gcc
CFLAGS = -std=c99 -Wall
objects = main.o terminal.o editor.o buffer.o
target = min

all: $(target)

$(target): $(objects)
	$(CC) $(CFLAGS) $(objects) -o $@

$(objects): dbg.h

terminal.o: terminal.h editor.h

editor.o: editor.h buffer.h

buffer.o: buffer.h

clean:
	rm $(target) $(objects)

.PHONY : clean