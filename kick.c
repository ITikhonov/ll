#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/keyboard.h>

extern llcall(void *p);
extern uint64_t *llsp;

extern uint64_t names[512];
extern void *addrs[512];
extern void *caddrs[512];
extern int lens[512];
extern char types[512];
extern void save();

struct termios oldkey, newkey;
int oldkbmode;

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
			read(0,&c,8);
			if(c==0x1b) {down(); exit(0);}
			if(c=='s') {save();}
			if(keyhook>-1) {
				*(--llsp)=c;
				llcall(caddrs[keyhook]);
			}
		}
	}
}

uint64_t kick(uint64_t f) {
	switch(f) {
	case 0x100: wait(); break;
	case 0x101: keyhook=*llsp++; break;
	case 0x102: printf("%lx ", *llsp++); fflush(stdout); break;
	}
	return 0;
}

