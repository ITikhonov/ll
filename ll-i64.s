.globl	llsp
.globl	llstack
.globl	llcall

.data

llstack:
	.quad	0,0,0,0,0,0,0,0
	.quad	0,0,0,0,0,0,0,329

llsp:
	.quad	llsp

.text
llcall:
	pushq	%rbx
	movq	llsp,%rsi
	lodsq
	call	*%rdi
	leaq	-8(%rsi),%rsi
	movq	%rax,(%rsi)
	movq	%rsi,llsp
	popq	%rbx
	ret

