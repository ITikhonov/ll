#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

extern uint64_t *llsp;
uint64_t llkick(uint64_t f) { return 0; }

static uint8_t comp[65535];
static struct caddr {uint16_t dict,word; uint8_t *c; int len; } caddrs[1024];
static struct back {uint16_t c; int32_t *o; } backs[1024];

static uint8_t dup_code[7]={0x48, 0x8d, 0x76, 0xf8, 0x48, 0x89, 0x06};

static uint8_t *compile_kick(uint8_t *p, uint64_t n) {
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

static void backref(struct caddr *c, int32_t *o) {
	struct back *p=backs,*e=backs+1024;
	for(;p<e;p++) {
		if(!p->o) { p->o=o; p->c=c-caddrs; return; }
	}
	abort();
}

static void backrefs(struct caddr *c) {
	int i=c-caddrs;
	struct back *p=backs,*e=backs+1024;
	for(;p<e;p++) {
		if(p->c==i&&p->o) {
			uint8_t *pp=(uint8_t *)(p->o+1);
			*(p->o)=(uint32_t)(caddrs[p->c].c-pp);
			p->o=0;
		}
	}
}

struct caddr *atom2caddr(uint16_t d, uint16_t a) {
	struct caddr *p=caddrs;
	for(;p->c;p++) {
		if(p->dict==d&&p->word==a) return p;
	}
	p->dict=d; p->word=a;
	return p;
}

static uint8_t *compile_forth(uint8_t *pc,uint16_t *p, uint16_t dn) {
	struct caddr *c=atom2caddr(dn,*p);
	*pc++=0xe8;
	int32_t *o=(int32_t*)pc;
	pc+=4;
	if(c->c) { *o=c->c-pc; }
	else { backref(c,o); }
	return pc;
}

static uint8_t *compile_number(uint8_t *pc, uint64_t n) {
	memcpy(pc,dup_code,7); pc+=7;
	*pc++=0x48; *pc++=0xb8;
	*(uint64_t*)pc=n; pc+=8;
	return pc;
}

static uint8_t *compile_def(uint8_t *pc, uint16_t *def, struct dict *d, uint16_t dn) {
	struct caddr *c=atom2caddr(dn,def[2]);
	c->c=pc;
	backrefs(c);
	int32_t *lbacks[16], **lback=lbacks;

	uint16_t *p=def+4;
	int l=def[0]/sizeof(uint16_t);
	for(;l--;p++) {
		struct atom *a=atoms+*p;

		switch(a->name[1]&0xff) {
		case 27 ... 42: { // number
			uint64_t n=make_num(*p);
			if(l&&p[1]==makeatom(0,0x37)) { // #
				pc=compile_kick(pc,n); p++; l--;
			} else {
				pc=compile_number(pc,n);
			}
		} break;
		default: {
			uint16_t *sdef;
			if(l>1&&p[1]==makeatom(0,0x36)) { // @
				int idx;
				idx=atom2idx(p[2],&dict);
				if(0x10000&idx) {
					printf("unknown dict");
					print_atom(*p);
					printf("\n");
					abort();
				}
				struct dict *d1=(struct dict *)(dict.def[idx]+4);
				idx=atom2idx(*p,d1);
				if(idx&0x10000) {
					printf("unknown word ");
					print_atom(*p);
					printf(" @ ");
					print_atom(p[2]);
					printf("\n");
					abort();
				}
				sdef=d1->def[idx];
				p+=2; l-=2;
			} else if(*p==makeatom(0,0x33)) { // {
				*lback++=(int32_t*)pc;
				pc+=4;
			} else if(*p==makeatom(0,0x34)) { // }
				lback--;
				**lback=pc-(((uint8_t*)(*lback))+4);
			} else {
				int idx=atom2idx(*p,d);
				if(0x10000&idx) {
					idx=atom2idx(*p,&dict);
					if(0x10000&idx) {
						printf("unknown word");
						print_nm(atoms[*p].name[0]);
						print_nm(atoms[*p].name[1]);
						printf("\n");
						abort();
					}
					sdef=dict.def[idx];
				} else {
					sdef=d->def[idx];
				}
			}
			if(sdef[1]==makeatom(0,0x060f121408LL)) { // forth
				pc=compile_forth(pc,p,dn);
			}
		} break;
		}
	}
	c->len=pc-c->c;
	hexdump(c->c,c->len);
	return pc;
}

uint8_t *compile1(struct dict *d, uint16_t dn, uint8_t *pc) {
        uint16_t **p=d->def;
        for(;*p;p++) {
		uint16_t *def=*p;
                print_nm(atoms[def[2]].name[0]);
                print_nm(atoms[def[2]].name[1]);
                putchar('\n');
                putchar(' ');
                putchar(' ');
                putchar(' ');

		if(def[1]==makeatom(0,0x060f121408LL)) { // forth
			pc=compile_def(pc,def,d,dn);
		} else if(def[1]==makeatom(0,0x04090314LL)) { // dict
			uint16_t *def=*p;
			pc=compile1((struct dict *)(def+4),def[2],pc);
		}
                putchar('\n');
        }
	return pc;
}

void compile() {
	memset(comp,0,sizeof(comp));
	memset(caddrs,0,sizeof(caddrs));
	compile1(&dict,makeatom(0,0x030f1205),comp);
}
