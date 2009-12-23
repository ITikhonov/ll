#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define H(x) (x>='a'?(10+x-'a'):(x-'0'))

int main(int argc,char *argv[]) {
	int s=socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons(1233),.sin_addr={htonl(0x7f000001)}};
	connect(s,(struct sockaddr*)&a,sizeof(a));

	unsigned char pkt[65535]={'F'}, *p=pkt+3;

	int n=0; char *v=argv[1];
	for(;;v++){
		switch(*v){
		case ' ':
		case '\0':
			{ int pad=8-((p-(pkt+1))&7); if(pad) { memset(p,0,pad); p+=pad; }; n++; }
			break;
		default:
			*p++=*v;
		}
		if(!*v) break;
	}

	pkt[1]=n&0xff;
	pkt[2]=n>>8;

	send(s,pkt,p-pkt,0);
}
