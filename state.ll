; I c3
dot F ^102 drop ;
drop A 48 ad c3
dump F ^2 drop ;
dup A 48 8d 76 f8 48 89 06 c3
fetch A 48 8b 00 c3
init F 'key ^101 drop $0 word store ;
stack F ^103 drop ;
isp F $3f cmp drop ?1 ; drop stack $3f ;
key F isp word fetch or $8 ror dup dot word store ;
load F ^1 drop ;
main F ^100 drop ;
or A 48 0b 06 48 8d 76 08 c3
reload F ^3 drop ;
ror A 88 c1 48 ad 48 d3 c8 c3
save F ^0 drop ;
store A 48 8b 0e 48 89 08 48 ad 48 ad c3
word A 48 8d 76 f8 48 89 06 48 8d 05 01 00 00 00 c3 00 00 00 00 00 00 00 00
cmp I 48 3b 06
test F $1 dot ?2 $2 dot ;
