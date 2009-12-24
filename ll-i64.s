.globl	llsp
.globl	llstack
.globl	llcall
.globl	llvm

.data

llstack:
	.quad	0,0,0,0,0,0,0,0
	.quad	0,0,0,0,0,0,0,329

llsp:
	.quad	.

.text
llcall:
	movq	llsp,%rsi
	lodsq
	call	*%rdi
	leaq	-8(%rsi),%rsi
	movq	%rax,(%rsi)
	movq	%rsi,llsp
	ret

llvm:
	movq	(%rsp),%rdi
	movq	(%rdi),%rdi
	movq	addrs(,%rdi,8),%rdi
	call	*%rdi
	addl	$8,(%rsp)
	jmp	llvm

