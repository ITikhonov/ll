#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "common.h"

static int fromascii(char c) {
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

static char fromintr(uint8_t c) {
	return " abcdefghijklmnopqrstuvwxyz0123456789ABCDEF.,\"^~?=_{}:@#<>+-"[c];
}

struct atom atoms[1024];

struct dict dict;

int atom2idx(int16_t a, struct dict *d) {
	uint16_t **p=d->def;
	for(;*p;p++) {
		if((*p)[2]==a) return p-d->def;
	}
	return 0x10000+(p-d->def);
}

int makeatom(uint64_t pre,uint64_t nm) {
	struct atom *p=atoms;
	for(;p->name[1];p++) {
		if(p->name[0]==pre && p->name[1]==nm) {
			return p-atoms;
		}
	}
	p->name[0]=pre; p->name[1]=nm;
	return p-atoms;
}

static uint64_t unhex(uint8_t x) {
        return x-27;
}




void load() {
	int f=open("state.ll",O_RDONLY);
	if(f<0) return;
	uint64_t nm=' ',pre=0;
	int tp=' ',tc=0;
	struct dict *d=&dict;
	uint16_t *def=0, **pdef=0;
	for(;;) {
		char buf[10240], *p=buf, *e;
		int n=read(f,p,sizeof(buf));
		if(n<=0) break;
		e=p+n;
		for(;p<e;p++) {
			putchar(*p); putchar('\n');
			if(*p==':') {
				if(tp!=':') {
					uint16_t a=makeatom(pre,nm);
					printf("ATOM %d:",a);
					print_nm(pre);
					print_nm(nm);
					printf("\n");
					int idx=atom2idx(a,d)&0xFFFF;
					pdef=d->def+idx;
					def=*pdef=realloc(*pdef,8);
					def[0]=0; def[1]=makeatom(0,0x060f121408); def[2]=a;
					nm=0; pre=0; tp=':';
				} else {
					def[1]++;
				}
				continue;
			} else if(*p=='|') {
				uint16_t a=makeatom(pre,nm);
				int idx=atom2idx(a,&dict)&0xFFFF;
				def=dict.def[idx]=realloc(dict.def[idx],8+sizeof(struct dict));
				memset(def+4,0,sizeof(struct dict));
				def[0]=sizeof(struct dict); def[1]=makeatom(0,0x04090314); def[2]=a; 
				d=(struct dict *)(def+4);
				def=0;
				
				nm=0; pre=0; tp=':';
				printf("new dict!");
				continue;
			}


			if(*p>='a'&&*p<='z') {tc='L';}
			else if((*p>='0'&&*p<='9')||(*p>='A'&&*p<='F')) {tc='N';}
			else if(*p==' '||*p=='\n') {tc=' ';}
			else { tc='O'; }

			if(tp=='O'||tc!=tp) {
				if(tp!=' '&&tp!=':') {
					if(def[1]==makeatom(0,0x060f121408)) {
						uint16_t a=makeatom(pre,nm);
						def=*pdef=realloc(*pdef,8+def[0]+8);
						def[4+def[0]/2]=a;
						def[0]=def[0]+2;

					} else {
						def=*pdef=realloc(*pdef,8+def[0]+8);
						((uint8_t *)(def+3))[def[0]++]=unhex(nm)|(unhex(nm>>8)<<4);
					}
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
	return;
}

int main() {
	load();

	dump(&dict);

	return 0;
}

