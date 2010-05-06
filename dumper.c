#include <stdio.h>

#include "common.h"

static char fromintr(uint8_t c) {
        return " abcdefghijklmnopqrstuvwxyz0123456789ABCDEF.,\"^~?=_{}:@#<>+-"[c];
}

void pc(char s) { if(s) putchar(s?fromintr(s):'_'); }

void print_nm(uint64_t nm) {
        char *v=((char*)&nm)+7; pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);pc(*v--);
}

void dump(struct dict *d) {
        uint16_t **p=d->def;
        for(;*p;p++) {
		uint16_t *def=*p;
		printf("%2ld: ",p-d->def);
		print_nm(atoms[def[2]].name[0]);
		print_nm(atoms[def[2]].name[1]);
		if(def[1]==makeatom(0,0x060f121408)) {
			uint16_t *p=def+4;
			uint16_t *e=p+def[0]/2;
			for(;p<e;p++) {
				print_nm(atoms[*p].name[0]);
				print_nm(atoms[*p].name[1]);
				putchar(' ');
			}
		} else if(def[1]==makeatom(0,0x04090314)) {
			printf(" DICT\n");
			uint16_t *def=*p;
			dump((struct dict *)(def+4));
		}
		putchar('\n');
        }
}

