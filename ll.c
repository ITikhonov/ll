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

union faddr {
	uint8_t *v;
	uint16_t *f;
};

uint64_t names[512];
#define PREFIX (8)
union faddr addrs[512];

uint64_t llkick(uint64_t f);

enum wordtype {UNDEF,FORTH,INLINE,DATA,ASM,EXTEND};


// """abcdefghijklmnopqrstuvwxyz0123456789ABCDEF.,"^~?=_{}:@"""
int fromascii(char c) {
	switch(c){
	case 'a' ... 'z': return (c-'a')+1;
	case '0' ... '9': return (c-'0')+27;
	case 'A' ... 'F': return (c-'A')+37;
	case '.': return 43;
	case ',': return 44;
	case '"': return 45;
	case '^': return 46;
	case '~': return 47;
	case '?': return 48;
	case '=': return 49;
	case '_': return 50;
	case '{': return 51;
	case '}': return 52;
	case ':': return 53;
	case '@': return 54;
	case '#': return 55;
	case '<': return 56;
	case '>': return 57;
	case '+': return 58;
	case '-': return 59;
	case ' ':
	case '\n': return 0;
	default:
		printf("\n!!! unknown symbol %u (%c)\n",c,c);
		abort();
	}
}

char fromintr(uint8_t c) {
	return " abcdefghijklmnopqrstuvwxyz0123456789ABCDEF.,\"^~?=_{}:@#<>+-"[c];
}

void pc(char s) { if(s) putchar(s?fromintr(s):'_'); }

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
			if(pre) { names[n]=pre; names[n+1]=w; }
			else { names[n]=w; }
			addrs[n].v=malloc(8);
			addrs[n].f[1]=0;
			addrs[n].f[0]=FORTH;
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

int len(int x) {
	if(addrs[x].f) return addrs[x].f[1];
	return 0;
}

uint16_t *type(int x){
	return addrs[x].f;
}

static void append(int cw, uint16_t n) {
	int olen=len(cw)+PREFIX;
	int nlen=olen+sizeof(*addrs[cw].f);
	addrs[cw].f=realloc(addrs[cw].f,(nlen&0xfffffff8)+8);
	union faddr c;
	c.v=addrs[cw].v+olen;
	*c.f=n;
	addrs[cw].f[1]=nlen-PREFIX;
}

static void append8(int cw, uint8_t v) {
	int olen=len(cw)+PREFIX;
	int nlen=olen+1;
	addrs[cw].v=realloc(addrs[cw].v,(nlen&0xfffffff8)+8);
	*(addrs[cw].v+olen)=v;
	addrs[cw].f[1]=nlen-PREFIX;
}

uint64_t unhex(uint8_t x) {
	return x-27;
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
			if(*p==':') {
				if(tp!=':') {
					cw=find(nm,pre); *type(cw)=FORTH; nm=' '; pre=0; tp=':';
				} else {
					(*type(cw))++;
				}
				continue;
			}


			if(*p>='a'&&*p<='z') {tc='L';}
			else if((*p>='0'&&*p<='9')||(*p>='A'&&*p<='F')) {tc='N';}
			else if(*p==' '||*p=='\n') {tc=' ';}
			else { tc='O'; }

			if(tp=='O'||tc!=tp) {
				if(tp!=' '&&tp!=':') {
					if(*type(cw)==FORTH) { append(cw,find(nm,pre)); }
					else { append8(cw,unhex(nm)|(unhex(nm>>8)<<4)); }
				}
				nm=fromascii(*p); pre=0;
			} else {
				nm=(nm<<8)|fromascii(*p);
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
		switch(*type(i)) {
		case UNDEF:
			undef++; break;
		case ASM:
			caddrs[i]=p; memcpy(p,addrs[i].v+PREFIX,len(i)); p+=len(i);
			fdump(stdout,caddrs[i],len(i));
			break;
		case FORTH:
			caddrs[i]=p;
			{
				uint16_t *a=(uint16_t *)(addrs[i].v+PREFIX);
				int l=len(i)/sizeof(*addrs[i].f);
				while(l--) {
					uint8_t *st=p;
					uint64_t v=*a;

					print_name(*a);
					uint64_t nm=names[*a];
					char c=nm&0xff;
					switch(c) {
					case 27 ... 42:
						v=make_num(*a);
						if(l&&names[*(a+1)]==55) { // '#'
							printf("KICK %lx\n",v);
							p=compile_kick(p,v);
							a++; l--;
							break;
						} else {
							printf("NUMR %lx\n",v);
						}

						memcpy(p,dup_code,7); p+=7;
						*p++=0x48; *p++=0xb8;
						*(uint64_t*)p=v; p+=8;
						break;

					default:
						if(l&&names[*(a+1)]==55) { // '#'
							printf("QUOT %lx\n",v);
							memcpy(p,dup_code,7); p+=7;
							*p++=0x48; *p++=0xb8;
							*(uint64_t*)p=v; p+=8;
							a++; l--;
							break;
						}

						if(c==51) { // '{'
							*(--backp)=p;
							*p++=0x00; *p++=0x00; *p++=0x00; *p++=0x00; break;
						} else if(c==52) { // '}'
							*(uint32_t*)(*backp)=p-(*backp+4); backp++; break;
						} else if(*type(v)==INLINE) { 
							printf("INLI");
							memcpy(p,addrs[v].v+PREFIX,len(v)); p+=len(v);
						} else if(*type(v)==DATA||*type(v)=='T') { 
							printf("\ndata word!\n");
							v=(uint64_t)(addrs[v].v+PREFIX);
							memcpy(p,dup_code,7); p+=7;
							*p++=0x48; *p++=0xb8;
							*(uint64_t*)p=v; p+=8;
						} else {
							if(l==0 || (l>0&&names[a[1]]==43)) {
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
			case DATA: break;
		}
		printf("total: ");
		if(caddrs[i]) fdump(stdout,caddrs[i],p-caddrs[i]);
		printf("\n\n");
		if(names[i]>>56) i++;
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
		printf(":(%c%x) ","UFIDAE"[*type(i)],i);
		switch(*type(i)) {
		case FORTH: {
			uint16_t *p=(uint16_t *)(addrs[i].v+PREFIX);
			uint16_t *e=(uint16_t *)(addrs[i].v+len(i));
			for(;p<e;p++) {
				print_name(*p);
				printf("[%x] ",*p);
			}
		} break;
		case INLINE:
		case DATA:
		case ASM: fdump(stdout,addrs[i].v+PREFIX,len(i)); break;
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
	if(init_so) init_so();
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

	find(5,0); //e
	find(0x090e0914,0); //init
	find(0x0d01090e,0); //main
	find(fromascii(0x7b),0);
	find(fromascii(0x7d),0);
	find(fromascii(0x2e),0);
	if(argc>1) { savename=argv[1]; }
	load(); dump();
	soreload();
	compile();

	llsp--;

	llcall(caddrs[1]);
	for(;;) {
		if(need_compile) {
			compile(); dump(); need_compile=0;
			if(*type(0)==FORTH) {
				llcall(caddrs[0]);
				*type(0)=UNDEF;
			}
			
		}
		llcall(caddrs[2]);
	}
	sodown();
	return 0;
}

