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

	cmpb	$0,%dl
	je	.llvm.forth
	cmpb	$'$',%dl
	je	.llvm.num
	cmpb	$'@',%dl
	je	.llvm.q
	cmpb	$'^',%dl
	je	.llvm.kick

.llvm.forth:

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

.llvm.q:
	leaq	-8(%rsi),%rsi
	movq	%rax,(%rsi)
	movq	(%rsp),%rax
	shrq	$8,%rdx
	shlq	$3,%rdx
	addq	%rdx,(%rsp)
	jmp	.llvm.end

.llvm.kick:
	leaq	-8(%rsi),%rsi
	movq	%rax,(%rsi)
	movq	%rsi,llsp
	shrq	$8,%rdx
	movq	%rdx,%rdi
	call	kick
	movq	llsp,%rsi
	jmp	.llvm.end

.llvm.end:
	addq	$8,(%rsp)
	jmp	llvm

.lf:	.byte	0xa

