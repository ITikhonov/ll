main:301#,302#,80008cursor< addrs draw@editor
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
addrs:201#
+::48 01 06 48 AD
realloc:204#
compile:4#,
cmpb::3A 06
storeb::8A 0E 88 08 48 AD 48 AD
init:300#,

editor|
color:304#,
cursor:::00 00 00 00 00 00 00 00
drawchar:cursor>~303#1+advance
advance:cursor>+cursor<
draw:">0?,={,,.} "> 1cmpb,={10shr ~drawname cursor>FFFF0000and80+cursor< drawdef, cursor>FFFF0000and100008+cursor< 8+draw.},,  8+draw
drawname:8+"> 1B~cmpb~,_{3advance} dc dc dc dc dc dc dc dc, 8+"> dc dc dc dc dc dc dc dc, 8+
dc:8rol"FFand={,.}drawchar
drawdef:">FFFFand={,,.} 3shl addrs+> drawname, 2+ drawdef


test|
testw:1 2 3

