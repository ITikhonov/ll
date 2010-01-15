init F 'key ^101 drop $0 word store ;
main F ^100 drop ;
key F word fetch or $8 ror dup dot word store ;
drop A 48 ad c3
word A 48 8d 76 f8 48 89 06 48 8d 05 01 00 00 00 c3 23 01 00 00 00 00 00 00
store A 48 8b 0e 48 89 08 48 ad 48 ad c3
; A 48 83 c4 10 c3
dot F ^102 drop ;
fetch A 48 8b 00 c3
dup A 48 8d 76 f8 48 89 06 c3
save F ^0 drop ;
load F ^1 drop ;
dump F ^2 drop ;
reload F ^3 drop ;
ror A 88 c1 48 ad 48 d3 c8 c3
or A 48 0b 06 48 8d 76 04 c3
