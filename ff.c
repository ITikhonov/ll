#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define H(x) (x>='a'?(10+x-'a'):(x-'0'))


void d(uint8_t *x) {
	printf("%02x ",x[-8]);
	printf("%02x ",x[-7]);
	printf("%02x ",x[-6]);
	printf("%02x ",x[-5]);
	printf("%02x ",x[-4]);
	printf("%02x ",x[-3]);
	printf("%02x ",x[-2]);
	printf("%02x  ",x[-1]);
}

void d1(uint8_t *x) {
	printf("%02x ",*x);
}

int main(int argc,char *argv[]) {
	int s=socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons(1233),.sin_addr={htonl(0x7f000001)}};
	connect(s,(struct sockaddr*)&a,sizeof(a));

	unsigned char pkt[65535]={'F'}, *p=pkt+3, t=0;

	int n=0,sp=0,last=-1; char *v=argv[1];
	for(;;v++){
restart:	switch(*v){
		case ' ':
		case '\0':
			if(!sp) {
				sp=1;
				int pad=7-((p-(pkt+3))&7);
				if(pad) { memset(p,0,pad); p+=pad; *p++=t; t=0; }; n++;
				d(p);
			}
			break;
		default:
			if(sp==1) {
				sp=0;
				switch(*v) {
 				case '$':
					{ int num=0; for(v++;;v++){
						int x=H(*v);
						if(x<0||x>15) { sp=1; *(uint64_t*)p=num|((uint64_t)('$')<<56); p+=8; n++; break; }
						num<<=4; num|=x;
					}}
					goto restart;
				case '[':
					*(uint16_t*)p=last; last=(p-(pkt+3))>>3;
					p+=8; sp=1; n++;
					printf("\n[[[ %d\n",last);
					continue;
				case ']':
					{
						uint64_t *pp=(uint64_t*)(pkt+3+(last<<3));
						int q=last;
						last=*(int16_t*)pp;
						*pp=(n-q-1)|((uint64_t)('@')<<56);
						printf("\n]]] %d\n",last);
						sp=1;
					}
					continue;
				default:;
				}
			}
			sp=0;
			*p++=*v;
		}
		if(!*v) break;
	}

	n--;
	pkt[1]=n&0xff;
	pkt[2]=n>>8;

	printf("\n");
	int i; for(i=0;i<p-pkt;i++) { d1(pkt+i); }
	printf("\n");

	send(s,pkt,p-pkt,0);
}
