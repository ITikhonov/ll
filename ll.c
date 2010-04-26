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

struct hdr {
	uint16_t type;
	uint32_t len;
	uint16_t reserved;
	uint64_t name[2];
	uint8_t code[];
}  __attribute__((packed));

#define PREFIX (24)
struct hdr *addrs[512];

uint64_t llkick(uint64_t f);

enum wordtype {UNDEF,FORTH,INLINE,DATA,ASM};


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
	uint64_t nm=addrs[n]->name[0];
	print_nm(nm);
	nm=addrs[n]->name[1];
	if(nm) { print_nm(nm); }
}

int find(uint64_t w, uint64_t pre) {
	int n;
	printf("==== ");
	print_nm(pre);
	print_nm(w);
	printf("\n");
	for(n=0;n<512-(pre?1:0);n++) {
		struct hdr *h=addrs[n];
		if(!h) {
			h=addrs[n]=malloc(8);
			if(pre) { h->name[0]=pre; h->name[1]=w; }
			else { h->name[0]=w; h->name[1]=0; }
			h->len=0;
			h->type=UNDEF;
			break;
		}


		if(pre) { if(h->name[0]==pre&&h->name[1]==w) break; }
		else { if(h->name[0]==w) break; }
	}
	return n;
}


void fdump(FILE *f, uint8_t *a, int l) {
	while(l--) {
		fprintf(f," %02x", *a++);
	}
}

static void append(int cw, uint16_t n) {
	struct hdr *h=addrs[cw];
	uint32_t nlen=h->len+2;
	printf("nlen %u [%u]\n",nlen,n);
	nlen=(nlen&0xfffffff8)+8;
	h=addrs[cw]=realloc(h,nlen+sizeof(struct hdr));
	uint16_t *a=(uint16_t *) (h->len + (uint8_t*)(h->code));
	a[0]=n;
	a[1]=0;
	h->len+=2;
}

static void append8(int cw, uint8_t n) {
	struct hdr *h=addrs[cw];
	uint32_t nlen=h->len+1;
	nlen=(nlen&0xfffffff8)+8;
	h=addrs[cw]=realloc(h,nlen+sizeof(struct hdr));
	uint8_t *a=(h->len + (uint8_t*)(h->code));
	*a=n;
	h->len+=1;
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
					cw=find(nm,pre); addrs[cw]->type=FORTH; nm=' '; pre=0; tp=':';
				} else {
					addrs[cw]->type++;
				}
				continue;
			}


			if(*p>='a'&&*p<='z') {tc='L';}
			else if((*p>='0'&&*p<='9')||(*p>='A'&&*p<='F')) {tc='N';}
			else if(*p==' '||*p=='\n') {tc=' ';}
			else { tc='O'; }

			if(tp=='O'||tc!=tp) {
				if(tp!=' '&&tp!=':') {
					if(addrs[cw]->type==FORTH) { append(cw,find(nm,pre)); }
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


uint8_t comp[65535];
uint8_t *caddrs[512];


uint64_t make_num(int n) {
	uint64_t nm=addrs[n]->name[0];
	uint64_t w=0;
	int i;

	for(i=0;i<32&&nm;i+=4) { w|=unhex(nm&0xff)<<i; nm>>=8; }
	if(i==32) {
		uint64_t pre=w; w=0; nm=addrs[n]->name[1];
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
		struct hdr *h=addrs[i];
		if(!h) continue;
		
		uint8_t *backs[16], **backp=&backs[16];
		printf("COMPILE:"); print_name(i); printf("\n");
		switch(h->type) {
		case UNDEF:
			undef++; break;
		case ASM:
			caddrs[i]=p; memcpy(p,h->code,h->len); p+=h->len;
			fdump(stdout,caddrs[i],h->len);
			break;
		case FORTH:
			caddrs[i]=p;
			{
				uint16_t *a=(uint16_t *)(h->code);
				int l=h->len/sizeof(uint16_t);
				while(l--) {
					uint8_t *st=p;
					uint64_t v=*a;
					struct hdr *hv=addrs[v];

					print_name(v);
					uint64_t nm=hv->name[0];
					uint64_t next=l?addrs[*(a+1)]->name[0]:0;
					char c=nm&0xff;
					switch(c) {
					case 27 ... 42:
						v=make_num(v);
						if(next==55) { // '#'
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
						if(next==55) { // '#'
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
						} else if(hv->type==INLINE) { 
							printf("INLI");
							memcpy(p,hv->code,hv->len); p+=hv->len;
						} else if(hv->type==DATA||hv->type=='T') { 
							printf("\ndata word!\n");
							v=(uint64_t)(hv->code);
							memcpy(p,dup_code,7); p+=7;
							*p++=0x48; *p++=0xb8;
							*(uint64_t*)p=v; p+=8;
						} else {
							if(l==0||next==43) {
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
	}
	printf("CODESIZE: %lu\n", p-comp);
	if(undef) printf("UNDEFS!!: %u\n", undef);
}
#endif

void dump() {
	printf("\n=== dump ===\n");
	int i;
	for(i=0;i<512;i++) {
		struct hdr *h=addrs[i];
		if(!h) continue;
		uint64_t nm=h->name[0];
		print_name(i);
		printf(":(%c%x) ","UFIDAE"[h->type],i);
		switch(h->type) {
		case FORTH: {
			uint16_t *p=(uint16_t *)(h->code);
			uint16_t *e=(uint16_t *)((uint8_t*)p+h->len);
			for(;p<e;p++) {
				print_name(*p);
				printf("[%x] ",*p);
			}
		} break;
		case INLINE:
		case DATA:
		case ASM: fdump(stdout,h->code,h->len); break;
		default:;
		}
		printf("\n");
		if(nm>>56) i++;
	}
	printf("%ld\n",*llsp);
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
		}
		llcall(caddrs[2]);
	}
	sodown();
	return 0;
}

