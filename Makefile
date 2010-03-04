CFLAGS=-g -Wall
LDFLAGS=-ldl -Wl,--export-dynamic
ASFLAGS=-g

all: ll kick.so state.ll

ll: ll.o ll-i64.o

kick.o: CFLAGS=-rdynamic -fPIC

kick.so: kick.o
	$(CC) -shared -nostartfiles -o kick.so kick.o

state.ll: core.ll editor.ll
	cat core.ll editor.ll > state.ll
	
clean:
	rm ll ll.o ll-i64.o kick.o kick.so state.ll
