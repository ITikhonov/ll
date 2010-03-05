add I 48 01 06 48 ad
addrs F ^201
char F ^104 drop
cmp I 48 3b 06
cmp8 I 3a 06
decm I ff 08 48 ad
dot F ^102 drop
drop I 48 ad
dump F ^2 drop
dup I 48 8d 76 f8 48 89 06
fetch I 48 8b 00
; I c3
init F 'key ^101 drop
lens F ^202
load F ^1 drop
main F ^100 drop
names F ^200
or I 48 0b 06 48 8d 76 08
over I 48 8d 76 f8 48 89 06 48 8b 46 08
realloc F ^204
reload F ^3 drop
rol8 I 48 c1 c0 08
ror I 88 c1 48 ad 48 d3 c8
save F ^0 drop
shl8 I 48 c1 e0 08
shr8 I 48 c1 e8 08
shl I 88 c1 48 d3 26 48 ad
stack F ^103 drop
store I 48 8b 0e 48 89 08 48 ad 48 ad
sub I 48 29 06 48 ad
up I 48 83 c4 08
swap I 48 89 c2 48 8b 06 48 89 16
