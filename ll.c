#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern uint64_t llstack[16];
extern uint64_t *llsp;
extern void llcall(void *p);

struct def { char t; unsigned char *data; int len; };

uint64_t names[512];
struct def defs[512];

int main(int argc,char *argv[]) {
	int s=socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons(1233),.sin_addr={htonl(0x7f000001)}};
	bind(s,(struct sockaddr*)&a,sizeof(a));
	fcntl(s, F_SETFL, O_NONBLOCK);
	memset(names,0,sizeof(names));
	memset(defs,0,sizeof(defs));

	names[0]=*(uint64_t*)("main\0\0\0");
	defs[0].t='A'; defs[0].data=malloc(defs[0].len=1);
	defs[0].data[0]=0xc3;

	llsp--;


	for(;;) {
		unsigned char b[1024];
		if(recv(s,b,1024,0)>0) {
			printf("pkt\n");
			int n,fr,sn=*(uint64_t*)(b+1);
			for(n=0;n<512;n++) {
				if(!defs[n].data) { fr=0; break; }
				if(names[n]==sn) { goto found; }
			}

			for(;n<512;n++) { if(names[n]==sn) { goto found; } }
			n=fr; names[n]=sn;
			

found:			switch(b[0]){
			case 'A':
			case 'D': {
					int l=*(uint16_t*)(b+9);
					defs[n].t=b[0];
					defs[n].data=realloc(defs[n].data,defs[n].len=l);
					memcpy(defs[n].data,b+11,l);
				}
			default:;
			}
		}
		printf("%p %ld\n",defs[0].data,*llsp);
		llcall(defs[0].data);
	}
}

