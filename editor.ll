init:@key@101,
key:prev> ^type"prev< 0?,={,space.} 1?,={,operator.} 2?,={,digit.} 3?,={,colon.} ,letter
type:$!?,_{,0.} $0?,_{,1.} $:?,_{,2.}={,3.} $A?,_{,1.} $[?,_{,2.} $a?,_{,1.} ${?,_{,4.} ,1
prev:::00 00 00 00 00 00 00 00
buf:::00 00 00 00 00 00 00 00
space:0?,={.}append
letter:4?,={,pushl.} 0?,={,pushl.} append pushl
digit:2?,={,pushn.} 0?,={,pushn.} append pushn
operator:0?,={,pushl.} append pushl
colon:4?,={create.} 3?,={cycle.} syntax
append:0?,={,.} 3?,={,.} 2?,={,buf>dot0buf<.} $Wchar,buf>print0buf<
create:,,$Cchar
pushl:buf>shlb or buf<
pushn:hex buf>4shl or buf<
syntax:,,$Echar
cycle:,,$Tchar
print:rolb"char rolb"char rolb"char rolb"char rolb"char rolb"char rolb"char rolb"char,
hex:$A?,_{$0-.}37-
