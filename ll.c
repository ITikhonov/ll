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

char *savename="state.ll";


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


void fdump(FILE *f, uint8_t *a, int l) {
	while(l--) {
		fprintf(f," %02x", *a++);
	}
}

void makeitforth(uint8_t *a) {
	*a=0xe8;
	*(int32_t*)(a+1)=((uint8_t*)llvm)-(a+5);
}

void load() {
	FILE *f=fopen(savename,"r");
	if(!f) return;
	uint64_t nm=0,*w;
	char state='N',bs=0,*pn=((char*)&nm);
	int cw=-1;
	for(;;) {
		char buf[10240], *p=buf, *e;
		int n=fread(p,1,sizeof(buf),f);
		if(n==0) break;
		e=p+n;
restart:	switch(state) {
		case ' ':
			while(p<e) { if((*p)!=' ') { state=bs; bs=' '; goto restart; } p++; } break;
		case 'N':
			while(p<e) {
				if(*p==' ') {
					cw=find(nm<<8);
					lens[cw]=5;
					addrs[cw]=realloc(addrs[cw],lens[cw]);
					types[cw]='F'; makeitforth(addrs[cw]);
					bs='T'; state=' '; p++;
					nm=0; pn=(char*)&nm; goto restart;
				}
				*pn++=*p++;
			} break;
		case 'D':
			while(p<e) {
				if(*p==' '||*p=='\n') {
					lens[cw]+=8;
					addrs[cw]=realloc(addrs[cw],lens[cw]);
					w=(uint64_t*)(((char*)addrs[cw])+lens[cw]-8);
					*w=(nm<<8)|bs;
					bs=*p=='\n'?'N':'W'; state=' '; p++; 
					nm=0; goto restart;
				} else {
					nm<<=4;
					if(*p>='a') { nm|=*p-'a'+10; }
					else { nm|=*p-'0'; }
					p++;
				}
			} break;
		case 'W':
			while(p<e) {
				switch(*p){
				case '@':
				case '$':
					if(bs==' ') {
						bs=*p++;
						state='D';
						goto restart;
					}
					bs='W';
				case '\n':
				case ' ':
					lens[cw]+=8;
					addrs[cw]=realloc(addrs[cw],lens[cw]);
					w=(uint64_t*)(((char*)addrs[cw])+lens[cw]-8);
					*w=find(nm<<8)<<8;
					bs=*p=='\n'?'N':'W'; state=' '; p++; 
					nm=0; pn=(char*)&nm; goto restart;
				}
				*pn++=*p++;
			} break;
		case 'T':
			bs='W'; state=' ';
			goto restart;
		}
		
	}
	fclose(f);
	return;
}

void save() {
	FILE *f=fopen(savename,"w");
	int i; for(i=0;i<512;i++) {
		if(!addrs[i]) continue;
		fprintf(f,"%.7s %c",((char *)(names+i))+1,types[i]);
		switch(types[i]) {
		case 'F':
			{
				uint64_t *a=(uint64_t*)(((uint8_t*)addrs[i])+5);
				int l=(lens[i]-5)/8;
				while(l--) {
					uint64_t v=(*a)>>8; uint8_t c=(*a++)&0xff;
					if(c) { fprintf(f," %c%lx",c,v); }
					else { fprintf(f," %.7s",((char*)(names+v))+1); }
				}
			}
			break;
		case 'D':
			fdump(f,((uint8_t*)addrs[i])+5,lens[i]-5);
			break;
		case 'A':
			fdump(f,addrs[i],lens[i]);
		default:;
		}
		fprintf(f,"\n");
	}
	fclose(f);
	
}

void dump() {
	printf("\n=== dump ===\n");
	int i;
	for(i=0;i<512;i++) {
		if(!addrs[i]) continue;
		printf("%.7s[%c] ",((char *)(names+i))+1,types[i]);
		switch(types[i]) {
		case 'F': {
			uint64_t *p=(uint64_t*)(addrs[i]+5);
			uint64_t *e=(uint64_t*)(addrs[i]+lens[i]);
			for(;p<e;p++) {
				if((char)*p) {
					printf("%c%lx ",(char)*p,*p>>8);
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


uint64_t kick(uint64_t f) {
	switch(f){
	case 0: save(); return 0;
	case 1: load(); return 0;
	case 2: dump(); return 0;
	}
	return 0;
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

	find(*(uint64_t*)"\0main\0\0");
	if(argc>1) { savename=argv[1]; }
	load();

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
		llcall(addrs[0]);
	}
}

