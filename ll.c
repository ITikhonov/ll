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

uint64_t llkick(uint64_t f) {
	switch(f){
	case 0x1: load(); return 0;
	case 0x2: dump(&dict); return 0;
	case 0x3: soreload(); return 0;
	case 0x4: need_compile=1; return 0;
	case 0x10: return (uint64_t)(dict->def);
	case 0x11: return (uint64_t)(atoms);
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

