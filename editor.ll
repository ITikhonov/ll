key F space word fetch put dup word store display 0d char
put F or shl8
display F dup rol8 char shl8 ? drop 20 char ; : .display
word D 00 00 00 00 00 00 00 00
space F 20 cmp ? drop drop 5b char names word fetch find dot 5d char 0a char 0 word store up ; : drop
end F 0 cmp drop ? drop drop 0 1 sub up ; :
find F over fetch end cmp drop ? drop names sub ; : swap 8 add swap .find
