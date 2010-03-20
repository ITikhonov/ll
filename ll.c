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

struct fcode { uint8_t t; uint64_t v; } __attribute__((__packed__));
union faddr {
	uint8_t *v;
	struct fcode *f;
};

uint64_t names[512];
union faddr addrs[512];
uint64_t lens[512];
char types[512];

uint64_t llkick(uint64_t f);

int find(uint64_t w) {
	//printf("f:====\n");
	int n;
	for(n=0;n<512;n++) {
		//printf("f:%d(%p) %08lx %08lx\n",n,addrs[n].v,w,names[n]);
		if(!names[n]) {
			//printf("f:alloc %lx %d\n",w,n);
			names[n]=w;
			types[n]='A'; addrs[n].v=malloc(lens[n]=1);
			*addrs[n].v=0xc3;
			break;
		}
		if(names[n]==w) break;
	}

	//printf("f:ret %d\n",n);
	return n;
}


void fdump(FILE *f, uint8_t *a, int l) {
	while(l--) {
		fprintf(f," %02x", *a++);
	}
}

static void append(int cw, uint8_t t, uint64_t v) {
	int nlen=lens[cw]+sizeof(struct fcode);
	addrs[cw].f=realloc(addrs[cw].f,nlen);
	union faddr c;
	c.v=addrs[cw].v+lens[cw];
	c.f->t=t;
	c.f->v=v;
	lens[cw]=nlen;
}

static void append8(int cw, uint8_t v) {
	int nlen=lens[cw]+1;
	addrs[cw].v=realloc(addrs[cw].v,nlen);
	*(addrs[cw].v+lens[cw])=v;
	lens[cw]=nlen;
}


void load() {
#define C1 {nm<<=8;if(nm>>56)putchar(nm>>56);}
	int f=open("test.ll",O_RDONLY);
	if(f<0) return;
	uint64_t nm=0;
	int tp=' ',tc=0,cw=-1,prefix=0;
	for(;;) {
		char buf[10240], *p=buf, *e;
		int n=read(f,p,sizeof(buf));
		if(n<=0) break;
		e=p+n;
		while(p<e) {
			if(*p>='a'&&*p<='z') {tc='L';}
			else if((*p>='0'&&*p<='9')||(*p>='A'&&*p<='F')) {tc='N';}
			else {tc=*p;}

			if(tp!=tc){
				switch(tp){
				case 'N':
					printf("N:%08lx\n",nm);
					if(types[cw]!='F') {
						append8(cw,nm);
					} else {
						if(prefix=='@') { append(cw,'^',nm); prefix=0; }
						else { append(cw,'$',nm); }
					}
					nm=0; break;
				case ' ': case '\r': case '\n': case '\t': nm=0; break;
				case ':': nm=0; break;
				case '@': prefix='@'; nm=0; break;
				case '|': prefix='|'; nm=0; break;
				case '$': printf(" char %c %02x\n",*p,*p); append(cw,'$',*p); tc=' '; break;
				default :
					if(tc==':') {
						if(tp!=':') {
							cw=find(nm);
							printf("def : %d %lx ",cw,nm); C1 C1 C1 C1 C1 C1 C1 C1; putchar('\n');
							types[cw]='F'; lens[cw]=0; addrs[cw].f=realloc(addrs[cw].f,lens[cw]);
						}
					} else {
						printf("prefix: %c %02x\n",prefix?prefix:' ',prefix);
						if(prefix=='@') {
							append(cw,'@',find(nm));
						} else if(prefix=='|') {
							append(cw,'|',find(nm));
						} else {
							append(cw,0,find(nm));
						}
						printf(" word: %c",prefix?prefix:' '); C1 C1 C1 C1 C1 C1 C1 C1; putchar('\n');
						prefix=0;
					}
				}
			}

			switch(tc) {
			case 'L': nm<<=8; nm|=*p; break;
			case 'N': nm<<=4; if(*p>='A') { nm|=*p-'A'+10; } else { nm|=*p-'0'; } break;
			case ':':
				if(tp==':') switch(types[cw]) {
				case 'F': types[cw]='I'; break;
				case 'I': types[cw]='D'; break;
				case 'D': types[cw]='A'; break;
				} break;
			default : nm=tc<<8;
			}
			p++; tp=tc;
		}
		
	}
	close(f);
	//exit(0);
	return;
#undef C1
}


int namecmp(const void *a, const void *b) {
	return memcmp(((char *)(names+*(int*)a))+1,((char *)(names+*(int *)b))+1,7);
}

uint8_t comp[65535];
uint8_t *caddrs[512];


void compile() {
	static uint8_t dup[7]={0x48, 0x8d, 0x76, 0xf8, 0x48, 0x89, 0x06};
	uint8_t *p=comp;
	int i; for(i=0;i<512;i++) {
		if(!addrs[i].f) continue;
		uint8_t *backs[16], **backp=&backs[16];
		printf("%s",(char*)(names+i)+1); 
		switch(types[i]) {
		case 'A':
			caddrs[i]=p; memcpy(p,addrs[i].v,lens[i]); p+=lens[i];
			fdump(stdout,caddrs[i],lens[i]);
			break;
		case 'F':
			caddrs[i]=p;
			{
				struct fcode *a=addrs[i].f;
				int l=lens[i]/sizeof(struct fcode);
				while(l--) {
					uint8_t *st=p;
					uint64_t v=a->v; uint8_t c=a->t;
					if(c&&c!='@') { fprintf(stdout," %c%lx",c,v); }
					else { fprintf(stdout," %.1s%.7s",(c?((char*)&c):""),((char*)(names+v))); }
					switch(c) {
					case 0:
						if(v==2) { // '{'
							*(--backp)=p;
							*p++=0x00; *p++=0x00; *p++=0x00; *p++=0x00; break;
						} else if(v==3) { // '}'
							*(uint32_t*)(*backp)=p-(*backp+4); backp++; break;
						} else if(types[v]=='I') { 
							memcpy(p,addrs[v].v,lens[v]); p+=lens[v];
						} else if(types[v]=='D'||types[v]=='T') { 
							printf("\ndata word!\n");
							v=(uint64_t)(addrs[v].v);
							memcpy(p,dup,7); p+=7;
							*p++=0x48; *p++=0xb8;
							*(uint64_t*)p=v; p+=8;
						} else {
							*p++=0xff; *p++=0x14; *p++=0x25;
							*(uint32_t*)p=(uint32_t)(uint64_t)(caddrs+v); p+=4;
						}
						break;
					case '|':
						*p++=0xff; *p++=0x24; *p++=0x25;
						*(uint32_t*)p=(uint32_t)(uint64_t)(caddrs+v); p+=4; break;
					case '$':
					case '@':
						memcpy(p,dup,7); p+=7;
						*p++=0x48; *p++=0xb8;
						*(uint64_t*)p=v; p+=8;
						break;
					case '^':
						memcpy(p,dup,7); p+=7;
						// mov    %rsi,llsp
						*p++=0x48; *p++=0x89; *p++=0x34; *p++=0x25;
						*(uint32_t*)p=(uint32_t)(uint64_t)(&llsp); p+=4;
						// movq $x,%rdi
						*p++=0x48; *p++=0xc7; *p++=0xc7;
						*(uint32_t*)p=(uint32_t)v; p+=4;
						// call llkick
						*p++=0xe8;
						*(uint32_t*)p=(uint32_t)(uint64_t)((uint8_t*)llkick-(p+4)); p+=4;
						// movq $llsp,%rsi
						*p++=0x48; *p++=0x8b; *p++=0x34; *p++=0x25;
						*(uint32_t*)p=(uint32_t)(uint64_t)(&llsp); p+=4;
						break;
					}
					fdump(stdout,st,p-st);
					printf("\n");
					a++;
				}
				*p++=0xc3;
			}
			case 'D': break;
		}
		printf("total: ");
		if(caddrs[i]) fdump(stdout,caddrs[i],p-caddrs[i]);
		printf("\n");
	}
	printf("CODESIZE: %lu\n", p-comp);
}

void dump() {
#define C1 {nm<<=8;if(nm>>56)putchar(nm>>56);}
	printf("\n=== dump ===\n");
	int i;
	for(i=0;i<512;i++) {
		if(!addrs[i].v) continue;
		uint64_t nm=names[i];
		C1 C1 C1 C1 C1 C1 C1 C1
		printf(":(%c) ",types[i]);
		switch(types[i]) {
		case 'F': {
			struct fcode *p=addrs[i].f;
			struct fcode *e=(struct fcode*)(addrs[i].v+lens[i]);
			for(;p<e;p++) {
				switch(p->t) {
				case '@':
					printf("@");
				case 0:
					{ uint64_t nm=names[p->v];
					C1 C1 C1 C1 C1 C1 C1 C1
					printf(" ");
					} break;
				default:
					printf("%c%lx ",p->t,p->v);
				}
			}
		} break;
		case 'I':
		case 'D':
		case 'A': fdump(stdout,addrs[i].v,lens[i]); break;
		case 'T':
			printf("[%lx]",lens[i]);
		default:;
		}
		printf("\n");
	}
	printf("%ld\n",*llsp);
#undef C1
}

void *dl=0;
uint64_t (*kick_so)(uint64_t f)=0;

void sodown() {
	if(dl) {
		uint64_t (*down_so)()=dlsym(dl,"down");
		printf("down=%p!\n",down_so);
		if(down_so) { down_so(); }
		dlclose(dl);
	}
}

void soreload() {
	printf("reload!\n");
	sodown();

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
	//case 0: save(); return 0;
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

	find(*(uint64_t*)"tini\0\0\0");
	find(*(uint64_t*)"niam\0\0\0");
	find(0x7b00); find(0x7d00);
	if(argc>1) { savename=argv[1]; }
	load(); dump();
	soreload();
	compile();

	llsp--;

	llcall(caddrs[0]);
	llcall(caddrs[1]);
	sodown();
	return 0;
}

