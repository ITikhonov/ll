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
	movq	(%rsp),%rdx
	movq	(%rdx),%rdx

	cmpb	$'$',%dl
	je	.llvm.num

	shrq	$8,%rdx
	movq	addrs(,%rdx,8),%rdx
	call	*%rdx
	jmp	.llvm.end

.llvm.num:
	shrq	$8,%rdx
	leaq	-8(%rsi),%rsi
	movq	%rax,(%rsi)
	movq	%rdx,%rax
	jmp	.llvm.end

.llvm.end:
	addl	$8,(%rsp)
	jmp	llvm

