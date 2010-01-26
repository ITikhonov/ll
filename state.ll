add I 48 01 06 48 ad
clear F $0 word store $38 shift store ;
cmp I 48 3b 06
decm I ff 08 48 ad
dot F ^102 drop ;
drop I 48 ad
dump F ^2 drop ;
dup I 48 8d 76 f8 48 89 06
fetch I 48 8b 00
find1 F dup fetch $0 cmp drop ? ; : word fetch cmp drop ? ; : drop $8 add .find1
find F names find1 drop names sub ;
; I c3
init F 'key ^101 drop clear ;
isp F $20 cmp drop ? drop norm find dot clear up : ;
key F isp word fetch or $8 ror word store shift fetch $8 sub shift store ;
load F ^1 drop ;
main F ^100 drop ;
names F ^200 ;
norm F word fetch shift fetch ror word store ;
or I 48 0b 06 48 8d 76 08
reload F ^3 drop ;
ror I 88 c1 48 ad 48 d3 c8
save F ^0 drop ;
shift T 8
shl8 I 48 c1 e0 08
shl I 88 c1 48 d3 26 48 ad
stack F ^103 drop ;
store I 48 8b 0e 48 89 08 48 ad 48 ad
sub I 48 29 06 48 ad
up I 48 83 c4 08
word T 8
