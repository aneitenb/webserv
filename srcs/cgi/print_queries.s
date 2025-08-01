.section .rodata
__query_string__:
	.string	"QUERY_STRING"
__HTTP_HEADERS__:
	.string	"Content-Type: text/html\r\n\r\n"
__start_list__:
	.string	"<html>\n<head>\n<title>\nqList\n</title>\n</head>\n<body>\n<h2>Queries:</h2>\n<ul>\n"
__print_query__:
	.string "<li><strong>%.*s</strong></li>\n"
__end_list__:
	.string	"</ul>\n</body>\n</html>"

.text

	.globl	main

main:
	leaq	__query_string__(%rip), %rdi
	call	getenv
	movq	$0, %r8
	cmpq	%rax, %r8
	je		__main_ret
	movq	%rax, %r12
	call	_print_headers
	call	_print_list_start
	movq	%r12, %rdi
	call	_print_queries
	call	_print_list_end

__main_ret:
	movq	$0, %rax
	ret

_print_headers:
	leaq	__HTTP_HEADERS__(%rip), %rdi
	call	printf
	ret

_print_list_start:
	leaq	__start_list__(%rip), %rdi
	call	printf
	ret

_print_list_end:
	leaq	__end_list__(%rip), %rdi
	call	printf
	ret

_print_query:
	movq	%rdi, %rdx
	leaq	__print_query__(%rip), %rdi
	call	printf
	ret

_print_queries:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$8, %rsp
	andq	$-16, %rsp
	movq	%rdi, -8(%rbp)
	call	__pq_loop
	ret

__pq_loop:
	movq	-8(%rbp), %rdi
	call	_skip_ampers
	addq	%rax, -8(%rbp)
	movq	-8(%rbp), %rdi
	call	_query_length
	movq	$0, %r9
	cmpq	%rax, %r9
	je		__pq_ret
	movq	%rax, %r9
	movq	-8(%rbp), %rdi
	movq	%r9, %rsi
	addq	%r9, -8(%rbp)
	call	_print_query
	jmp		__pq_loop

__pq_ret:
	movq	%rbp, %rsp
	popq	%rbp
	ret

_skip_ampers:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	%rdi, %r8
	movq	$0, %r9
	movb	$0x26, %r10b

__sa_loop:
	movb	(%r8, %r9, 1), %r11b
	cmpb	%r10b, %r11b
	jne		__sa_ret
	incq	%r9
	jmp		__sa_loop

__sa_ret:
	movq	%r9, %rax
	movq	%rbp, %rsp
	popq	%rbp
	ret

_query_length:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	%rdi, %r8
	movq	$0, %r9
	movb	$0, %r10b
	movb	$0x26, %r11b

__ql_loop:
	movb	(%r8, %r9, 1), %dl
	cmpb	%r10b, %dl
	je		__ql_ret
	cmpb	%r11b, %dl
	je		__ql_ret
	incq	%r9
	jmp		__ql_loop

__ql_ret:
	movq	%r9, %rax
	movq	%rbp, %rsp
	popq	%rbp
	ret
