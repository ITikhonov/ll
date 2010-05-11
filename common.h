#include <stdint.h>

extern struct dict { uint16_t *def[512]; } dict;
extern struct atom { uint64_t name[2]; } atoms[1024];


int fromascii(char c);
char fromintr(uint8_t c);

uint64_t unhex(uint8_t x);
uint64_t make_num(uint16_t a);

void print_nm(uint64_t nm);
void hexdump(uint8_t *a, int l);
void print_atom(uint16_t);

int atom2idx(int16_t a, struct dict *d);
int makeatom(uint64_t pre,uint64_t nm);


