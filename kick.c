#include <stdio.h>
#include <stdint.h>

extern llcall(void *p);
extern uint64_t *llsp;

extern uint64_t names[512];
extern void *addrs[512];
extern int lens[512];
extern char types[512];

void udpinit();
void udp();

void init() {
	udpinit();
}

uint64_t kick(uint64_t f) {
	switch(f) {
	case 0x100: udp(); break;
	}
	return 0;
}

void down() {
	printf("down!!!\n");
}

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int s=-1;

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

