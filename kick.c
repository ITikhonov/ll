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
extern int lens[512];
extern char types[512];

void udpinit();
void udp();

struct termios oldkey, newkey;
int oldkbmode;

void init() {
	udpinit();

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

int s=-1;
int keyhook=-1;

static void wait() {
	struct pollfd fds[2] = {{.fd=0,.events=POLLIN},{.fd=s,.events=POLLIN}};
	uint64_t c=0;
	if(poll(fds,2,-1)>0) {
		printf("fds[0].revents\n",fds[0].revents);
		if(fds[0].revents&POLLIN) {
			read(0,&c,8);
			printf("key: %08lx\n",c);
			if(c==0x1b) {down(); exit(0);}
			if(keyhook>-1) {
				*(--llsp)=c;
				llcall(addrs[keyhook]);
			}
		}
		if(fds[1].revents&POLLIN) udp();
	}
}

uint64_t kick(uint64_t f) {
	switch(f) {
	case 0x100:
		wait(); break;
	case 0x101:;
		keyhook=*llsp++; break;
	}
	return 0;
}


#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>



void udp() {
	unsigned char b[1024];
	if(recv(s,b,1024,0)>0) {
		uint64_t sn=*(uint64_t*)(b+3);
		int n=find(sn);
		int l=*(uint16_t*)(b+1);

		switch(b[0]){
		case 'A':
		case 'D':
			types[n]=b[0];
			addrs[n]=realloc(addrs[n],lens[n]=l);
			memcpy(addrs[n],b+11,l);
			break;
		case 'F':
			types[n]=b[0];
			addrs[n]=realloc(addrs[n],lens[n]=l*8+5);
			{
				uint64_t *p=(uint64_t*)(b+11);
				uint64_t *d=(uint64_t*)(addrs[n]+5);
				makeitforth(addrs[n]);
				int i; for(i=0;i<l;i++) {
					if((char)*p) { *d=*p; } else { *d=find(*p)<<8; }
					d++; p++;
				}
			}
			break;
		case 'E':
			llcall(addrs[n]);
			break;
		default:;
		}
	}
}

void udpinit() {
	s=socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons(1233),.sin_addr={htonl(0x7f000001)}};
	bind(s,(struct sockaddr*)&a,sizeof(a));
	fcntl(s, F_SETFL, O_NONBLOCK);
}

