main:301#,302#,100008cursor\draw< core$dict\draw draw\editor
char:104#,
cr:Achar
dot:102#,
?::48 39 06
.::C3
,::48 AD
nz::0F 84
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
~::48 89 C1 48 8B 06 48 89 0E
ror::88 C1 48 AD 48 D3 C8
rol::88 C1 48 AD 48 D3 C0
rolb::48 C1 C0 08
shl::88 C1 48 D3 26 48 AD
shr::88 C1 48 D3 2E 48 AD
-::48 29 06 48 AD
dict:10#
atoms:11#
dumpatoms:5#,
+::48 01 06 48 AD
!::48 89 C3 48 AD
@::48 8D 76 F8 48 89 06 48 89 D8
%::48 01 C3 48 AD
realloc:204#
compile:4#,
cmpb::3A 06
storeb::8A 0E 88 08 48 AD 48 AD
init:300#, init\editor
find:@>0?,={,,0.} >20shrFFFFand ?,={,@.} 8%find
fb::8A 00
t::48 85 06
key:key\editor
sw::48 8B 0E 66 89 08 48 AD 48 AD

editor|
init:8et> realloc "et< 8~<
key:ct+fb FFand "check tempatom8+> FF00000000000000t,nz{tempatom<0} 8shl or tempatom8+<
tempatom:::00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
type:1B?,_{,word$.} 2B?,_{,number$.} ,op$
check:tempatom8+>0?,,={,.} type op$?,={,pop.}  tempatom8+>FFand type ?,,={.} pop 
pop:tempatom> tempatom8+> atoms!makeatom append 0tempatom< 0tempatom8+<
makeatom:@8+>0?,={, @8+< @< @atoms-4shr.} ?,={~ @>?,={,,@atoms-4shr.} ~} 10%makeatom
expand:et> ">2+ ~^~realloc ~^< et<
append:expand et>">+2-sw

draw: 80020cursor\draw< et>8+ et>">+ def\draw tempatom\editor datom\draw

ct:::00 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 3D 2D 37 3C 3F FF FF FF FF FF 3A 2C 3B 2B FF 1B 1C 1D 1E 1F 20 21 22 23 24 35 FF 38 31 39 30 3E 25 26 27 28 29 2A FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 36 FF 2E 32 FF 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 33 FF 34 2F FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF

et:::00 00 00 00 00 00 00 00

draw|
color:304#,
cursor:::00 00 00 00 00 00 00 00
char:cursor>~303#1+advance
advance:cursor>+cursor<
atom:4shl atoms+ datom
datom:">nm 8+>nm
dc:8rol"FFand={,.}char
nm:dc dc dc dc dc dc dc dc,
name:20shrFFFFand atom 
def:?={,,.} ^>FFFFand atom 3advance ~2+~def
forth:~8+~^+def
word:" >"name 40x "FFFFand ~10shrFFFFand forth$?,={,forth.},,,
line:cursor>100000+FFFF0000and8+cursor<
list:">0?,={,,.}word line 8+list
x:cursor>FFFF0000and+cursor<
dict:dict\core!find>8+ list\draw

