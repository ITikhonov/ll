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
extern void llvm();


uint64_t names[512];
void *addrs[512];
int lens[512];
char types[512];

int find(uint64_t w) {
	int fr=0,n;

	for(n=0;n<512;n++) {
		if(!addrs[n]) { fr=n; break; }
		if(names[n]==w) { return n; }
	}

	for(;n<512;n++) {
		if(names[n]==w) { return n; }
	}

	n=fr; names[n]=w;
	types[n]='A'; addrs[n]=malloc(lens[n]=1);
	*(unsigned char*)addrs[n]=0xc3;
	return n;
}

int main(int argc,char *argv[]) {
	int s=socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons(1233),.sin_addr={htonl(0x7f000001)}};
	bind(s,(struct sockaddr*)&a,sizeof(a));
	fcntl(s, F_SETFL, O_NONBLOCK);
	memset(names,0,sizeof(names));
	memset(addrs,0,sizeof(addrs));
	memset(lens,0,sizeof(lens));
	memset(types,0,sizeof(types));

	{	find(*(uint64_t*)"\0main\0\0");
		int n=find(*(uint64_t*)"\0;\0\0\0\0\0");
		addrs[n]=realloc(addrs[n],5);
		((uint8_t*)addrs[n])[0]=0x48;
		((uint8_t*)addrs[n])[1]=0x83;
		((uint8_t*)addrs[n])[2]=0xc4;
		((uint8_t*)addrs[n])[3]=0x10;
		((uint8_t*)addrs[n])[4]=0xc3;
	}

	llsp--;

	for(;;) {
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
					*(uint8_t*)addrs[n]=0xe8;
					*(int32_t*)(addrs[n]+1)=((uint8_t*)llvm)-((uint8_t*)d);
					int i; for(i=0;i<l;i++) {
						if((char)*p) { *d=*p; } else { *d=find(*p)<<8; }
						d++; p++;
					}
				}
				break;
			case 'L':
				{
					int i; for(i=0;i<512;i++) {
						if(!addrs[i]) continue;
						printf("%.7s[%c] ",((char *)(names+i))+1,types[i]);
						switch(types[i]) {
						case 'F': {
							uint64_t *p=(uint64_t*)(addrs[i]+5);
							uint64_t *e=(uint64_t*)(addrs[i]+lens[i]);
							for(;p<e;p++) {
								if((char)*p) {
									printf("%c%lu ",(char)*p,*p>>8);
								} else {
									printf("%.7s[%d] ",(char*)(names+(*p>>8))+1,(int)*p>>8);
								}
							}
							}
						default:;
						}
						printf("\n");
					}
					printf("%ld\n",*llsp);
					
				}
			case 'E':
				llcall(addrs[n]);
				break;
			default:;
			}
		}
		llcall(addrs[0]);
	}
}

