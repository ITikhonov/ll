key F input editor ? select ; : edit
input F qstion return space 0d char put word fetch display up
put F word fetch or shl8 word store
space F 20 cmp drop ? drop word fetch 0 word store up ; :
return F 0d cmp drop ? 0a char reset up up ; :
reset F 0 word store 0 def store
word D 00 00 00 00 00 00 00 00
def D 00 00 00 00 00 00 00 00
pos D 00 00 00 00 00 00 00 00
editor F def fetch 0 cmp8 drop drop
display F dup rol8 char shl8 ? drop 20 char ; : .display
select F names swap find dup def store addrs add fetch pos store def fetch show 0d char
end F 0 cmp drop ? drop over store names sub up ; :
find F over fetch end cmp drop ? drop names sub ; : swap 8 add swap .find
edit F 0 cmp drop ? right refresh ; : replace refresh
refresh F 0d char def fetch names over add fetch display show
replace F names swap find 5 shl pos fetch store right
right F pos fetch 8 add pos store
qstion F 3f cmp drop ? names word fetch find show reset up up ; :
show F addrs over add fetch  swap lens add fetch over add swap showa
showa F cmp ? drop drop ; : cursor dup fetch print 8 add .showa
print F 0 qword 27 qword 2e qword kick number spec 3f char 3f char 3f char dot
qword F cmp8 ? char shr8 3 shl names add fetch display up ; : drop
number F 24 cmp8 drop ? shr8 dot up ; :
kick F 5e cmp8 ? char shr8 dot up ; : drop
spec F 3f cmp drop ? char 20 char up ; : 3a cmp drop ? char 20 char up ; :
test F cmp drop
cursor F pos fetch cmp drop ? word fetch display 7c char :
