#include <stdio.h>

#include "common.h"

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

