#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "common.h"

extern uint64_t llstack[16];
extern uint64_t *llsp;
extern void llvm();

void load();
void dump();
void compile();



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

int imagefd=-1;

char *atom2str(uint16_t a, char s[17]) {
	uint64_t n=atoms[a].name[0];
	char *p=s;
	int i;

	for(i=56;i>=0;i-=8) {
		printf("OPEN %d %lx %x\n",i,(n>>i)&0xff,fromintr((n>>i)&0xff));
		uint8_t c=n>>i;
		if(c) *p++=fromintr(c);
	}
	n=atoms[a].name[1];
	for(i=56;i>=0;i-=8) {
		printf("OPEN %d %lx %x\n",i,(n>>i)&0xff,fromintr((n>>i)&0xff));
		uint8_t c=n>>i;
		if(c) *p++=fromintr(c);
	}
	*p=0;

	printf("OPEN %016lx:%016lx '%s'\n",atoms[a].name[0],atoms[a].name[1], s);
	return s;
}

uint64_t llkick(uint64_t f) {
	char s[17];
	switch(f){
	case 0x1: load(); return 0;
	case 0x2: dump(&dict); return 0;
	case 0x3: soreload(); return 0;
	case 0x4: need_compile=1; return 0;
	case 0x5: dumpatoms(); return 0;
	case 0x10: return (uint64_t)(dict->def);
	case 0x11: return (uint64_t)(atoms);
	case 0x12: { uint64_t a=*llsp++; forthcall(*llsp++,a);
		     return 0; }

	case 0x20: imagefd=open(atom2str(*llsp++,s),O_CREAT|O_TRUNC|O_WRONLY,0644); break;
	case 0x21: close(imagefd); break;
	case 0x22: write(imagefd,llsp,1); llsp++; break;

	case 0x30: imagefd=open(atom2str(*llsp++,s),O_RDONLY,0644); break;
	case 0x31: close(imagefd); break;
	case 0x32: {uint8_t c; return (read(imagefd,&c,1)==1)?c:-1;}

	default:
		if(kick_so) return kick_so(f);
	}
	return 0;
}

int key(int c) {
	llsp--; *llsp=c;
	forthcall(makeatom(0,0x0b0519),makeatom(0,0x030f1205));
}

int main(int argc,char *argv[]) {
	uint16_t *core=realloc(0,8+sizeof(struct dict));
	memset(core,0,sizeof(struct dict)+8);
	dict=(struct dict *)(core+4);
	int idx=atom2idx(makeatom(0,0x030f1205),dict)&0xFFFF;
	dict->def[idx]=core;
	core[0]=sizeof(struct dict);
	core[1]=makeatom(0,0x04090314LL);
	core[2]=makeatom(0,0x030f1205);

	load(); dump();
	dumpatoms();
	soreload();
	compile();

	llsp--;

	forthcall(makeatom(0,0x090e0914),makeatom(0,0x030f1205));
	for(;;) {
		if(need_compile) {
			compile(); dump(); need_compile=0;
		}
		forthcall(makeatom(0,0x0d01090e),makeatom(0,0x030f1205));
	}
	sodown();
	return 0;
}

