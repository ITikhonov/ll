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

int find(uint64_t w) {
	int fr=0,n;

	printf("find %lx\n",w);
	for(n=0;n<512;n++) {
		printf("  search/f %d(%lx)\n",n,names[n]);
		if(!defs[n].data) { fr=n; break; }
		if(names[n]==w) { return n; }
	}

	printf("free %d\n",fr);

	for(;n<512;n++) {
		if(defs[n].data) printf("  search   %d(%lx)\n",n,names[n]);
		if(names[n]==w) { return n; }
	}

	n=fr; names[n]=w;
	printf("  usefree  %d\n",n);
	defs[n].t='A'; defs[n].data=malloc(defs[n].len=1);
	defs[n].data[0]=0xc3;
	return n;
}

int main(int argc,char *argv[]) {
	int s=socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons(1233),.sin_addr={htonl(0x7f000001)}};
	bind(s,(struct sockaddr*)&a,sizeof(a));
	fcntl(s, F_SETFL, O_NONBLOCK);
	memset(names,0,sizeof(names));
	memset(defs,0,sizeof(defs));

	find(*(uint64_t*)"main\0\0\0");

	llsp--;

	for(;;) {
		unsigned char b[1024];
		if(recv(s,b,1024,0)>0) {
			uint64_t sn=*(uint64_t*)(b+3);
			int n=find(sn);
			int l=*(uint16_t*)(b+1);
			printf("pkt %d\n",n);

			switch(b[0]){
			case 'A':
			case 'D':
				defs[n].t=b[0];
				defs[n].data=realloc(defs[n].data,defs[n].len=l);
				memcpy(defs[n].data,b+11,l);
				break;
			case 'F': 
				defs[n].t=b[0];
				defs[n].data=realloc(defs[n].data,defs[n].len=l*8);
				{
					uint64_t *p=(uint64_t*)(b+11);
					uint64_t *d=(uint64_t*)defs[n].data;
					int i; for(i=0;i<l;i++) {
						if(0xff00000000000000ll&*p) { *d=*p; } else { *d=find(*p); }
						d++; p++;
					}
				}
				break;
			case 'L':
				{
					int i; for(i=0;i<512;i++) {
						if(!defs[i].data) continue;
						printf("%.8s[%c] ",(char *)(names+i),defs[i].t);
						switch(defs[i].t) {
						case 'F': {
							uint64_t *p=(uint64_t*)defs[i].data;
							uint64_t *e=(uint64_t*)(defs[i].data+defs[i].len);
							for(;p<e;p++) {
								if(0xff00000000000000ll&*p) {
									printf("%c%lu ",(char)(*p>>56),0xffffffffffffff&*p);
								} else {
									printf("%s ",(char*)(names+*p));
								}
							}
							}
						default:;
						}
						printf("\n");
					}
					printf("%ld\n",*llsp);
					
				}
			default:;
			}
		}
		llcall(defs[0].data);
	}
}

