main:301#,302#,500100 1 303#, 80008cursor< names drawnames
char:104#,
dot:102#,
?::48 39 06
.::C3
,::48 AD
=::0F 85
_::0F 89
"::48 8D 76 F8 48 89 06
stack:103#,
>::48 8B 00
<::48 8B 0E 48 89 08 48 AD 48 AD
^::48 8D 76 F8 48 89 06 48 8B 46 08
shlb::48 C1 E0 08
or::48 0B 06 48 8D 76 08
and::48 23 06 48 8D 76 08
~::48 89 C2 48 8B 06 48 89 16
ror::88 C1 48 AD 48 D3 C8
rol::88 C1 48 AD 48 D3 C0
rolb::48 C1 C0 08
shl::88 C1 48 D3 26 48 AD
shr::88 C1 48 D3 2E 48 AD
-::48 29 06 48 AD
names:200#
addrs:201#
lens:202#
types:203#
+::48 01 06 48 AD
realloc:204#
compile:4#,
cmpb::3A 06
storeb::8A 0E 88 08 48 AD 48 AD
init:300#,

cursor:::00 00 00 00 00 00 00 00
drawchar:FFand={,.} cursor>~303#1+cursor>+cursor<
drawcell:8rol"drawchar 8rol"drawchar  8rol"drawchar 8rol"drawchar 8rol"drawchar 8rol"drawchar 8rol"drawchar 8rol drawchar
drawnames:"> 0?,={,,.} drawcell cursor>80000+FFFF0000and8+ cursor< 8+drawnames

