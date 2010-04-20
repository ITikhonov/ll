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

struct fcode { uint16_t n; } __attribute__((__packed__));
union faddr {
	uint8_t *v;
	struct fcode *f;
};

uint64_t names[512];
union faddr addrs[512];
uint64_t lens[512];
char types[512];

uint64_t llkick(uint64_t f);

void pc(char s) { if(s) putchar(s?s:'_'); }

void print_nm(uint64_t nm) {
	char *v=((char*)&nm)+7; pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);
}

void print_name(int n) {
	uint64_t nm=names[n];
	print_nm(nm);
	if(nm>>56) {
		nm=names[n+1];
		print_nm(nm);
	}
}


int find(uint64_t w, uint64_t pre) {
	int n;
	printf("==== ");
	print_nm(pre);
	print_nm(w);
	printf("\n");
	for(n=0;n<512-(pre?1:0);n++) {
		if(!names[n]) {
			if(pre) { names[n]=pre; names[n+1]=w; types[n+1]='.'; }
			else { names[n]=w; }
			types[n]='U'; addrs[n].v=0;
			break;
		}


		if(pre) { if(names[n]==pre&&names[n+1]==w) break; }
		else { if(names[n]==w) break; }

		if((names[n]>>56)) n++;
	}
	return n;
}


void fdump(FILE *f, uint8_t *a, int l) {
	while(l--) {
		fprintf(f," %02x", *a++);
	}
}

static void append(int cw, uint16_t n) {
	int nlen=lens[cw]+sizeof(struct fcode);
	addrs[cw].f=realloc(addrs[cw].f,nlen);
	union faddr c;
	c.v=addrs[cw].v+lens[cw];
	c.f->n=n;
	lens[cw]=nlen;
}

static void append8(int cw, uint8_t v) {
	int nlen=lens[cw]+1;
	addrs[cw].v=realloc(addrs[cw].v,nlen);
	*(addrs[cw].v+lens[cw])=v;
	lens[cw]=nlen;
}

uint64_t unhex(char x) {
	if(x<'A') {return x-'0';}
	else {return (x-'A')+10;}
}

void dump();



void load() {
#define C1 {char *v=((char*)&nm)+7; pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--); }
	int f=open(savename,O_RDONLY);
	if(f<0) return;
	uint64_t nm=' ',pre=0;
	int tp=' ',tc=0,cw=-1;
	for(;;) {
		char buf[10240], *p=buf, *e;
		int n=read(f,p,sizeof(buf));
		if(n<=0) break;
		e=p+n;
		for(;p<e;p++) {
			if(nm=='$') {
				append(cw,find((nm<<8)|*p,0)); nm=' '; tp=' ';
				continue;
			}

			if(*p==':') {
				if(tp!=':') {
					cw=find(nm,pre); types[cw]='F'; nm=' '; pre=0; tp=':';
				} else {
					switch(types[cw]) {
					case 'F': types[cw]='I'; break;
					case 'I': types[cw]='D'; break;
					case 'D': types[cw]='A'; break;
					}
				}
				continue;
			}


			if(*p>='a'&&*p<='z') {tc='L';}
			else if((*p>='0'&&*p<='9')||(*p>='A'&&*p<='F')) {tc='N';}
			else if(*p==' '||*p=='\n') {tc=' ';}
			else { tc='O'; }

			if(tp=='O'||tc!=tp) {
				if(tp!=' '&&tp!=':') {
					if(types[cw]=='F') { append(cw,find(nm,pre)); }
					else { append8(cw,unhex(nm)|(unhex(nm>>8)<<4)); }
				}
				nm=*p; pre=0;
			} else {
				nm=(nm<<8)|*p;
				if(!pre&&(nm>>56)&0xff) { pre=nm; nm=0; }
			}
			tp=tc;
		}
		
	}
	close(f);
	dump();
	return;
#undef C1
}


int namecmp(const void *a, const void *b) {
	return memcmp(((char *)(names+*(int*)a))+1,((char *)(names+*(int *)b))+1,7);
}

uint8_t comp[65535];
uint8_t *caddrs[512];


uint64_t make_num(int n) {
	uint64_t nm=names[n];
	uint64_t w=0;
	int i;

	for(i=0;i<32&&nm;i+=4) { w|=unhex(nm&0xff)<<i; nm>>=8; }
	if(i==32) {
		uint64_t pre=w; w=0; nm=names[n+1];
		for(i=0;i<32&&nm;i+=4) { w|=unhex(nm&0xff)<<i; nm>>=8; }
		w|=pre<<i;
	}
	return w;
}


#if 1
static uint8_t dup_code[7]={0x48, 0x8d, 0x76, 0xf8, 0x48, 0x89, 0x06};

uint8_t *compile_kick(uint8_t *p, uint64_t n) {
	memcpy(p,dup_code,7); p+=7;
	// mov    %rsi,llsp
	*p++=0x48; *p++=0x89; *p++=0x34; *p++=0x25;
	*(uint32_t*)p=(uint32_t)(uint64_t)(&llsp); p+=4;
	// movq $x,%rdi
	*p++=0x48; *p++=0xc7; *p++=0xc7;
	*(uint32_t*)p=(uint32_t)n; p+=4;
	// call llkick
	*p++=0xe8;
	*(uint32_t*)p=(uint32_t)(uint64_t)((uint8_t*)llkick-(p+4)); p+=4;
	// movq $llsp,%rsi
	*p++=0x48; *p++=0x8b; *p++=0x34; *p++=0x25;
	*(uint32_t*)p=(uint32_t)(uint64_t)(&llsp); p+=4;
	return p;
}

void compile() {
	int undef=0;
	uint8_t *p=comp;
	int i; for(i=0;i<512;i++) {
		if(!names[i]) continue;
		uint8_t *backs[16], **backp=&backs[16];
		printf("COMPILE:"); print_name(i); printf("\n");
		switch(types[i]) {
		case 'U':
			undef++; break;
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
					uint64_t v=a->n;

					print_name(a->n);
					uint64_t nm=names[a->n];
					char c=nm&0xff;
					if((nm>>8)=='$') { c='$'; v=nm&0xff; }
					switch(c) {
					case '0' ... '9':
					case 'A' ... 'F':
						v=make_num(a->n);
						if(l&&names[(a+1)->n]=='#') {
							printf("KICK %lx\n",v);
							p=compile_kick(p,v);
							a++; l--;
							break;
						} else {
							printf("NUMR %lx\n",v);
						}

					case '$':
						memcpy(p,dup_code,7); p+=7;
						*p++=0x48; *p++=0xb8;
						*(uint64_t*)p=v; p+=8;
						break;

					default:
						if(l&&names[(a+1)->n]=='#') {
							printf("QUOT %lx\n",v);
							memcpy(p,dup_code,7); p+=7;
							*p++=0x48; *p++=0xb8;
							*(uint64_t*)p=v; p+=8;
							a++; l--;
							break;
						}

						if(c=='{') {
							*(--backp)=p;
							*p++=0x00; *p++=0x00; *p++=0x00; *p++=0x00; break;
						} else if(c=='}') {
							*(uint32_t*)(*backp)=p-(*backp+4); backp++; break;
						} else if(types[v]=='I') { 
							printf("INLI");
							memcpy(p,addrs[v].v,lens[v]); p+=lens[v];
						} else if(types[v]=='D'||types[v]=='T') { 
							printf("\ndata word!\n");
							v=(uint64_t)(addrs[v].v);
							memcpy(p,dup_code,7); p+=7;
							*p++=0x48; *p++=0xb8;
							*(uint64_t*)p=v; p+=8;
						} else {
							if(l==0 || (l>0&&a[1].n==4)) {
								printf("JUMP %lx\n",v);
								*p++=0xff; *p++=0x24; *p++=0x25;
							} else {
								printf("CALL %lx\n",v);
								*p++=0xff; *p++=0x14; *p++=0x25;
							}
							*(uint32_t*)p=(uint32_t)(uint64_t)(caddrs+v); p+=4;
						}
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
		printf("\n\n");
	}
	printf("CODESIZE: %lu\n", p-comp);
	if(undef) printf("UNDEFS!!: %u\n", undef);
}
#endif

void dump() {
#define C1 {char *v=((char*)&nm)+7; pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--); }
	printf("\n=== dump ===\n");
	int i;
	for(i=0;i<512;i++) {
		if(!names[i]) continue;
		uint64_t nm=names[i];
		print_name(i);
		printf(":(%c%x) ",types[i],i);
		switch(types[i]) {
		case 'F': {
			struct fcode *p=addrs[i].f;
			struct fcode *e=(struct fcode*)(addrs[i].v+lens[i]);
			for(;p<e;p++) {
				print_name(p->n);
				printf("[%x] ",p->n);
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
		if(nm>>56) i++;
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

int need_compile=0;

uint64_t llkick(uint64_t f) {
	switch(f){
	//case 0: save(); return 0;
	case 1: load(); return 0;
	case 2: dump(); return 0;
	case 3: soreload(); return 0;
	case 4: need_compile=1; return 0;
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

	find(*(uint64_t*)"tini\0\0\0",0);
	find(*(uint64_t*)"niam\0\0\0",0);
	types[find(0x7b,0)]='S';
	types[find(0x7d,0)]='S';
	find(0x2e,0);
	find('e',0);
	if(argc>1) { savename=argv[1]; }
	load(); dump();
	soreload();
	compile();

	llsp--;

	llcall(caddrs[0]);
	for(;;) {
		if(need_compile) {
			compile(); dump(); need_compile=0;
			if(types[5]=='F') {
				llcall(caddrs[5]);
				types[5]='U';
			}
			
		}
		llcall(caddrs[1]);
	}
	sodown();
	return 0;
}

