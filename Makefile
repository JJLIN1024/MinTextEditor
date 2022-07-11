CC = gcc
CFLAGS = -std=c99 -Wall
objects = main.o terminal.o editor.o render.o event.o cursor.o data.o syntax.o
target = min

all: $(target)

$(target): $(objects)
	$(CC) $(CFLAGS) $(objects) -o $@

$(objects): dbg.h

terminal.o: terminal.h editor.h

editor.o: data.h render.h editor.h

render.o: editor.h render.h

event.o: editor.h data.h cursor.h render.h event.h

cursor.o: editor.h cursor.h

data.o: editor.h data.h

syntax.o: editor.h syntax.h

clean:
	rm $(target) $(objects)

.PHONY : clean