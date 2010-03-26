init:@key@101,
key:"char prev> ^type"prev< 0?,={,space.} 1?,={,operator.} 2?,={,digit.} 3?,={,colon.} ,letter
type:$!?,_{,0.} $0?,_{,1.} $:?,_{,2.}={,3.} $A?,_{,1.} $[?,_{,2.} $a?,_{,1.} ${?,_{,4.} ,1
prev:::00 00 00 00 00 00 00 00
buf:::00 00 00 00 00 00 00 00
current:::00 00 00 00 00 00 00 00
space:0?,={.}pop D?,={Achar compile},
letter:4?,={,pushl.} 0?,={,pushl.} pop pushl
digit:2?,={,pushn.} 0?,={,pushn.} pop pushn
operator:0?,={,pushl.} pop pushl
colon:4?,={create.} 3?,={cycle.} syntax
pop:0?,={,.} 3?,={,.} 2?,={,buf>$$append 0buf<.} ,buf>find3shr$Cappend 0buf<
create:,,buf>find ""current< addrs+0^>realloc~< 0~lens+< 0buf<
pushl:buf>shlb or buf<
pushn:hex buf>4shl or buf<
syntax:,,$Echar$Rchar$Rchar
cycle:,,$Tchar
print:rolb"char rolb"char rolb"char rolb"char rolb"char rolb"char rolb"char rolb"char,
hex:$A?,_{$0-.}37-
find:names search
search:^^ > 0?,={,^<names-~,.} ?,,={names-~,.}8+|search
append:^^record current>" addrs+^lens+>9+^> realloc~< " lens+">~^9+~< ~addrs+>+ ~^<1+<
recordc:0cmpb,={,$ }@205,
recordw:rolb"recordc rolb"recordc rolb"recordc rolb"recordc rolb"recordc rolb"recordc rolb"recordc rolb"recordc,
record:$C?,={recordc 3shl names+>recordw.}
