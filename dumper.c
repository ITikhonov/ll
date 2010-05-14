#include <stdio.h>

#include "common.h"

static void dump1(struct dict *d) {
        uint16_t **p=d->def;
        for(;*p;p++) {
		uint16_t *def=*p;
		printf("%2ld: ",p-d->def);
		print_atom(def[2]);
		printf("(%hu)",def[2]);
		putchar(' ');
		putchar('[');
		print_atom(def[1]);
		putchar(']');
		printf("%p*",def);
		if(def[1]==makeatom(0,0x060f121408LL)) {
			uint16_t *p=def+4;
			uint16_t *e=p+def[0]/2;
			for(;p<e;p++) {
				print_atom(*p);
				printf("(%hu)",*p);
				putchar(' ');
			}
		} else if(def[1]==makeatom(0,0x04090314)) {
			printf(" DICT\n");
			uint16_t *def=*p;
			if(d!=(struct dict *)(def+4)) {
				dump1((struct dict *)(def+4));
			}
		} else {
			hexdump((uint8_t*)(def+4),def[0]);
		}
		putchar('\n');
        }
}

void dump() {
	dump1(dict);
}
