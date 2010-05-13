#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"

struct dict dict;
struct atom atoms[1024];

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
	case '\\': return 54;
	case '#': return 55;
	case '<': return 56;
	case '>': return 57;
	case '+': return 58;
	case '-': return 59;
	case '$': return 60;
	case '!': return 61;
	case '@': return 62;
	case '%': return 63;
	case ' ':
	case '\n': return 0;
	default:
		printf("\n!!! unknown symbol %u (%c)\n",c,c);
		abort();
	}
}

char fromintr(uint8_t c) {
	return " abcdefghijklmnopqrstuvwxyz0123456789ABCDEF.,\"^~?=_{}:\\#<>+-$!@%"[c];
}

int atom2idx(int16_t a, struct dict *d) {
	uint16_t **p=d->def;
	for(;*p;p++) {
		if((*p)[2]==a) return p-d->def;
	}
	return 0x10000+(p-d->def);
}

int makeatom(uint64_t pre,uint64_t nm) {
	struct atom *p=atoms;
	for(;p->name[1]||p->name[0];p++) {
		if(p->name[0]==pre && p->name[1]==nm) {
			return p-atoms;
		}
	}
	p->name[0]=pre; p->name[1]=nm;
#ifdef DEBUG
	printf("NEW ATOM: ");
	print_atom(p-atoms);
	printf("(%lu)",p-atoms);
	printf("[%lu][%lu]",p->name[0],p->name[1]);
	printf("\n");
#endif
	return p-atoms;
}

uint64_t unhex(uint8_t x) {
        return x-27;
}

uint64_t make_num(uint16_t a) {
        struct atom *h=atoms+a;
        uint64_t nm=h->name[0];
        uint64_t w=0;

	while(nm) { w<<=4; w|=unhex(nm>>56); nm<<=8; }

        nm=h->name[1];
	if(nm) {
		while(!(nm>>56)) { nm<<=8; }
		while(nm) { w<<=4; w|=unhex(nm>>56); nm<<=8; }
	}

        return w;
}


static void pc(char s) { if(s) putchar(s?fromintr(s):'_'); }

void print_nm(uint64_t nm) {
        char *v=((char*)&nm)+7; pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);
}

void print_atom(uint16_t a) {
	print_nm(atoms[a].name[0]);
	print_nm(atoms[a].name[1]);
}

void hexdump(uint8_t *a, int l) {
        while(l--) {
                printf(" %02x", *a++);
        }
}



