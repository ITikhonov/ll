key     F input display
input   F select edit
select  F e selsp read
e       F def fetch 0 cmp drop drop ? ret : up
selsp   F 20 cmp drop ? drop names word fetch find dup def store addrs add fetch pos store 0 word store up up ret : ret
edit    F 20 cmp drop ? drop right ret : read
read    F word fetch or shl8 word store up up
display F clr def fetch 0 cmp8 drop ? drop word fetch pword ret : names over add fetch pword show
return F 0d cmp drop ? 0a char reset up ret :
qstion F 3f cmp drop ? names word fetch find show reset up ret :
space F 20 cmp drop ? drop word fetch 0 word store up ret :
reset F 0 word store 0 def store
word D 00 00 00 00 00 00 00 00
def D 00 00 00 00 00 00 00 00
pos D 00 00 00 00 00 00 00 00
pword F dup rol8 char shl8 ? drop 20 char ret : .pword
end F 0 cmp drop ? drop over store names sub up ret :
find F over fetch end cmp drop ? drop names sub ret : swap 8 add swap .find
replace F names swap find 5 shl pos fetch store right
right F pos fetch 8 add pos store
show F addrs over add fetch  swap lens add fetch over add swap showa
showa F cmp ? drop drop ret : cursor dup fetch print 8 add .showa
print F 0 qword 27 qword 2e qword kick number spec 3f char 3f char 3f char dot
qword F cmp8 ? char shr8 3 shl names add fetch pword up ret : drop
number F 24 cmp8 drop ? shr8 dot up ret :
kick F 5e cmp8 ? char shr8 dot up ret : drop
spec F 3f cmp drop ? char 20 char up ret : 3a cmp drop ? char 20 char up ret :
test F cmp drop
cursor F pos fetch cmp drop ? word fetch pword 7c char :
refresh F clr def fetch names over add fetch pword show
clr F 0d char 1b char 5b char 30 char 4b char
