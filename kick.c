#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <fcntl.h>

extern uint64_t *llsp;
extern uint64_t llstack[16];

static void stack() {
	int n=(&llstack[16])-llsp;
	uint64_t *p=llsp;
	printf("\nstack[%u]:",n);
	while(n--) { printf(" %lx", *p++); }
	printf("\n");
}

void init() {
	printf("init!!!\n");
}

void down() {
	printf("down!!!\n");
}

void sdl_init();
void sdl_poll();
void sdl_clear();
int sdl_char(int c, int x, int y);
void sdl_color(uint64_t c);


uint64_t kick(uint64_t f) {
	switch(f) {
	case 0x102: printf("%lx ", *llsp++); fflush(stdout); break;
	case 0x103: stack(); break;
	case 0x104: if(*llsp) putchar(*llsp); llsp++; fflush(stdout); break;

	case 0x204: { void *p=(void*)(*llsp++); return (uint64_t)realloc(p,*llsp++); }

	case 0x300: sdl_init(); break;
	case 0x301: sdl_poll(); break;
	case 0x302: sdl_clear(); break;
	case 0x303: {
			int c=*llsp++;
			int p=*llsp++;
			return sdl_char(c,p&0xffff,p>>16);
		};
	case 0x304: sdl_color(*llsp++); break;
	}
	return 0;
}
