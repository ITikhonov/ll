; I c3
dot F ^102 drop ;
drop I 48 ad
dump F ^2 drop ;
dup I 48 8d 76 f8 48 89 06
fetch I 48 8b 00
init F 'key ^101 drop $0 word store ;
stack F ^103 drop ;
isp F $3f cmp drop ?1 ; drop stack $3f ;
key F isp word fetch or $8 ror dup dot word store find ;
load F ^1 drop ;
main F ^100 drop ;
or I 48 0b 06 48 8d 76 08
reload F ^3 drop ;
ror I 88 c1 48 ad 48 d3 c8
save F ^0 drop ;
store I 48 8b 0e 48 89 08 48 ad 48 ad
word T 8
cmp I 48 3b 06
test F $1 dot ?2 $2 dot ;
shl8 I 48 c1 e0 08
names F ^200 ;
add I 48 01 06 48 ad
up I 48 83 c4 08
find1 F dup fetch dot $8 add ;
findx F cmp ?3 find1 up findx ; 
find F names $1000 add names findx drop drop ;
