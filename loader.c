#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "common.h"

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

