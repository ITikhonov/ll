key F space word fetch put dup word store 0d char display
put F or shl8
display F dup rol8 char shl8 ? drop 20 char ; : .display
word D 00 00 00 00 00 00 00 00
space F 20 cmp ? drop drop names word fetch find show 0 word store up ; : drop
end F 0 cmp drop ? drop drop 0 1 sub up ; :
find F over fetch end cmp drop ? drop names sub ; : swap 8 add swap .find
notfound F 0 1 sub cmp drop ? up drop ; :
show F notfound  addrs over add fetch  swap lens add fetch over add swap showa 0a char
showa F cmp ? drop drop ; : dup fetch print 8 add .showa
print F sword 27 qword 2e qword kick number spec 3f char 3f char 3f char dot
sword F 0 cmp8 drop ? shr8 3 shl names add fetch display up ; :
qword F cmp8 ? char shr8 3 shl names add fetch display up ; : drop
number F 24 cmp8 drop ? shr8 dot up ; :
kick F 5e cmp8 ? char shr8 dot up ; : drop
spec F 3f cmp drop ? char 20 char up ; : 3a cmp drop ? char 20 char up ; :
