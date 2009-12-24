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

	unsigned char pkt[65535]={argv[1][0]}, *p=pkt+3;

	memset(p,0,8);
	strncpy(p,argv[2],8);
	p+=8;

	int n;
	for(n=3;n<argc;n++) {
		char *v=argv[n];
		if(v[1]=='\0') { *p++=v[0]; continue; }
		*p++=(H(v[0])<<4)|H(v[1]);
	}

	n-=3;
	pkt[1]=n&0xff;
	pkt[2]=n>>8;

	send(s,pkt,p-pkt,0);
}
