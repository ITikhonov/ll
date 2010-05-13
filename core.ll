main:301#,302#,80008cursor@draw< dict list@draw
char:104#,
cr:Achar
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
dict:10#
atoms:11#
+::48 01 06 48 AD
realloc:204#
compile:4#,
cmpb::3A 06
storeb::8A 0E 88 08 48 AD 48 AD
init:300#,

draw|
color:304#,
cursor:::00 00 00 00 00 00 00 00
char:cursor>~303#1+advance
advance:cursor>+cursor<
atom:4shl atoms+">nm 8+>nm
dc:8rol"FFand={,.}char
nm:dc dc dc dc dc dc dc dc,
name:20shrFFFFand atom 
def:?={,,.} ^>FFFFand atom 3advance ~2+~def
forth:~8+~^+def
word:" >"name 40x "FFFFand ~10shrFFFFand forth$?,={,forth.},,,
line:cursor>100000+FFFF0000and8+cursor<
list:">0?,={,,.}word line 8+list
x:cursor>FFFF0000and+cursor<

