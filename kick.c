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

extern llcall(void *p);
extern uint64_t *llsp;

extern uint8_t names[];
extern uint8_t addrs[];
extern uint8_t lens[];
extern uint8_t types[];

extern void *caddrs[512];

int history;

struct termios oldkey, newkey;
int oldkbmode;

extern uint64_t llstack[16];

static void stack() {
	int n=(&llstack[16])-llsp;
	uint64_t *p=llsp;
	printf("\nstack[%u]:",n);
	while(n--) { printf(" %lx", *p++); }
	printf("\n");
}

void init() {
        tcgetattr(STDIN_FILENO,&oldkey);
        newkey.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
        newkey.c_iflag = IGNPAR;
        newkey.c_oflag = oldkey.c_oflag;
        newkey.c_lflag = 0;
        newkey.c_cc[VMIN]=1;
        newkey.c_cc[VTIME]=0;
        tcflush(STDIN_FILENO, TCIFLUSH);
        tcsetattr(STDIN_FILENO,TCSANOW,&newkey);

	//ioctl(0,KDGKBMODE,&oldkbmode);
	//ioctl(0,KDSKBMODE,K_MEDIUMRAW);

	history=open("changes",O_RDWR|O_CREAT,0644);
	stack();
}

void down() {
	tcsetattr(STDIN_FILENO,TCSANOW,&oldkey);
	printf("down!!!\n");
}

int keyhook=-1;


static void wait() {
	struct pollfd fds[2] = {{.fd=0,.events=POLLIN}};
	uint64_t c=0;
	if(poll(fds,1,-1)>0) {
		if(fds[0].revents&POLLIN) {
			read(0,&c,1);
			if(c==0x1b) {down(); exit(0);}
			if(keyhook>-1) {
				*(--llsp)=c;
				llcall(caddrs[keyhook]);
			}
		}
	}
}

void sdl_init();
void sdl_poll();
void sdl_clear();
int sdl_char(int c, int x, int y);
void sdl_color(uint64_t c);


uint64_t kick(uint64_t f) {
	switch(f) {
	case 0x100: wait(); break;
	case 0x101: keyhook=*llsp++; break;
	case 0x102: printf("%lx ", *llsp++); fflush(stdout); break;
	case 0x103: stack(); break;
	case 0x104: if(*llsp) putchar(*llsp); llsp++; fflush(stdout); break;

	case 0x200: return (uint64_t)names;
	case 0x201: return (uint64_t)addrs;
	case 0x202: return (uint64_t)lens;
	case 0x203: return (uint64_t)types;
	case 0x204: { void *p=(void*)(*llsp++); return (uint64_t)realloc(p,*llsp++); }
	case 0x205: write(history,llsp,1); llsp++; break;
	case 0x206: { uint8_t c=0; read(history,&c,1); return c;}

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
