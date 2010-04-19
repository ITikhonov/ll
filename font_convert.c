#include <stdio.h>
#include <unistd.h>

char lens[256];

int main() {
	FILE *f=fopen("font.h","w");
	int w;

	fprintf(f,"uint8_t font[]={\n");
	int i=0;
	for(i=0;i<256;i++) { lens[i]=-1; }
	for(;;) {
		int c=getchar();
		if(c==-1) break;
		if(c==1){
			fprintf(f,"\n");
			w=i;
			i=0;
		} else {
			fprintf(f,"0x%02x,",c);

			if(c!=0xff) {
				int off=i%8;
				int no=i/8;
				if(lens[no]<off) { lens[no]=off; }
			}

			i++;
		}
	}
	fprintf(f,"};\n\nunsigned char fontm[]={");
	for(i=0;i<256;i++) {
		if(lens[i]==-1) break;
		fprintf(f,"%u,",lens[i]+1);
	}
	fprintf(f,"};\n\nint fontw=%u;\n",w);

	fclose(f);
	return 0;
}

