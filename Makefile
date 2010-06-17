CFLAGS=-g -fPIC -Wall -DGL_GLEXT_PROTOTYPES
LDFLAGS=-ldl -Wl,--export-dynamic -lGL -lGLU -lSDL
ASFLAGS=-g

all: ll kick.so state.ll test_opengl

ll: ll.o ll-i64.o loader.o dumper.o common.o compiler.o

ll.o: common.h
loader.o: common.h
dumper.o: common.h
common.o: common.h
compiler.o: common.h
kick.o: common.h

kick.o: CFLAGS=-rdynamic -fPIC

kick.so: kick.o opengl.o
	$(CC) -shared -nostartfiles -o kick.so kick.o opengl.o

state.ll: core.ll
	cat core.ll > state.ll

opengl.o: font.h

font.h: font_convert font.xcf
	convert font.xcf gray:- | ./font_convert 
	
	
clean:
	rm -f ll ll.o ll-i64.o kick.o kick.so state.ll

test_opengl: opengl.c font.h
	gcc $(CFLAGS) -o test_opengl -DTEST opengl.c $(LDFLAGS)

