CFLAGS=-g -Wall
LDFLAGS=-ldl -Wl,--export-dynamic
ASFLAGS=-g

all: ll ss ff kick.so

ll: ll.o ll-i64.o

kick.o: CFLAGS=-rdynamic -fPIC

kick.so: kick.o
	$(CC) -shared -nostartfiles -o kick.so kick.o
	
clean:
	rm ll ll.o ll-i64.o kick.o kick.so 
