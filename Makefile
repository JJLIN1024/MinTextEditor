CC = gcc
CFLAGS = -std=c99 -Wall
objects = main.o terminal.o editor.o render.o event.o cursor.o data.o
target = min

all: $(target)

$(target): $(objects)
	$(CC) $(CFLAGS) $(objects) -o $@

$(objects): dbg.h

terminal.o: terminal.h editor.h

editor.o: data.h editor.h

render.o: editor.h render.h

event.o: editor.h event.h

cursor.o: editor.h cursor.h

clean:
	rm $(target) $(objects)

.PHONY : clean