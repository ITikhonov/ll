#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

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
	uint64_t nm=0;
	char state='N',bs=0,*pn=((char*)&nm)+1;
	int cw=-1;
	for(;;) {
		char buf[10240], *p=buf, *e;
		int n=fread(p,1,sizeof(buf),f);
		if(n==0) break;
		e=p+n;
		while(p<e) {
			switch(state) {
			case ' ':
				if(*p=='\n') { state='N'; nm=0; pn=((char*)&nm)+1; }
				else if(*p!=' ') { state=bs; continue; }
				break;
			case 'N':
				if(*p==' ') { cw=find(nm); state=' '; bs='T'; break; }
				*pn++=*p; break;
			case 'T':
				switch(*p) {
				case 'F':
					lens[cw]=5;
					addrs[cw]=realloc(addrs[cw],lens[cw]);
					types[cw]='F';
					state=' '; bs='V'; break;
				case 'A':
					lens[cw]=0;
					if(addrs[cw]) addrs[cw]=realloc(addrs[cw],lens[cw]);
					types[cw]='A';
					nm=0; state=' '; bs='H'; break;
				}
				break;
			case 'V':
				nm=0; 
				switch(*p) {
				case '@': case '$': case '^': bs=*p; state='R'; break;
				default: pn=((char*)&nm)+1; state='W'; continue;
				}
				break;
			case 'W':
				if(*p==' '||*p=='\n') {
					lens[cw]+=8;
					addrs[cw]=realloc(addrs[cw],lens[cw]);
					*(uint64_t*)(((uint8_t*)addrs[cw])+lens[cw]-8)=find(nm)<<8;
					makeitforth(addrs[cw]);
					state=' '; bs='V'; continue;
				}
				*pn++=*p;
				break;
			case 'R':
				if(*p==' '||*p=='\n') {
					lens[cw]+=8;
					addrs[cw]=realloc(addrs[cw],lens[cw]);
					*(uint64_t*)(((uint8_t*)addrs[cw])+lens[cw]-8)=(nm<<8)|bs;
					state=' '; bs='V'; continue;
				}
				nm<<=4; if(*p>='a') { nm|=*p-'a'+10; } else { nm|=*p-'0'; }
				break;
			case 'H':
				if(*p==' '||*p=='\n') {
					lens[cw]+=1;
					addrs[cw]=realloc(addrs[cw],lens[cw]);
					*(((uint8_t*)addrs[cw])+lens[cw]-1)=nm;
					nm=0; state=' '; bs='H'; continue;
				}
				nm<<=4; if(*p>='a') { nm|=*p-'a'+10; } else { nm|=*p-'0'; }
			}
			p++;
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
		} break;
		case 'A': fdump(stdout,addrs[i],lens[i]); break;
		default:;
		}
		printf("\n");
	}
	printf("%ld\n",*llsp);
}

void *dl=0;
uint64_t (*kick_so)(uint64_t f)=0;

void soreload() {
	printf("reload!\n");
	if(dl) {
		uint64_t (*down_so)()=dlsym(dl,"down");
		printf("down=%p!\n",down_so);
		if(down_so) { down_so(); }
		dlclose(dl);
	}

	dl=dlopen("./kick.so",RTLD_NOW);
	if(!dl) printf("no kick.so: %s\n", dlerror());
	void (*init_so)()=dlsym(dl,"init");
	if(init_so) init_so(&llsp,names,addrs,lens,types,llcall);
	else printf("no init in so\n");
	kick_so=dlsym(dl,"kick");
	if(!kick_so) printf("no kick in so\n");
}

uint64_t llkick(uint64_t f) {
	switch(f){
	case 0: save(); return 0;
	case 1: load(); return 0;
	case 2: dump(); return 0;
	case 3: soreload(); return 0;
	default:
		if(kick_so) return kick_so(f);
	}
	return 0;
}

int main(int argc,char *argv[]) {
	memset(names,0,sizeof(names));
	memset(addrs,0,sizeof(addrs));
	memset(lens,0,sizeof(lens));
	memset(types,0,sizeof(types));

	find(*(uint64_t*)"\0main\0\0");
	if(argc>1) { savename=argv[1]; }
	load(); dump();
	soreload();

	llsp--;

	for(;;) {
		llcall(addrs[0]);
	}
}

