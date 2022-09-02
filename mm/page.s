/*
 *  linux/mm/page.s
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * page.s contains the low-level page-exception code.
 * the real work is done in mm.c
 */

.globl page_fault

page_fault:
	xchgl %eax,(%esp) // 交换 eax 和 esp 指向空间的内容，目的是将出错码放到 eax，同时将 eax 压栈
	pushl %ecx // 保存现场
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%edx // 系统数据段段选择子
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	movl %cr2,%edx // 取出错的线性地址
	pushl %edx // 将出错地址和出错码压栈，作为之后的 C 函数的入参
	pushl %eax
	testl $1,%eax // 根据出错码的最低位，决定调用的 C 函数
	jne 1f
	call do_no_page // 为 0，调用缺页处理函数
	jmp 2f
1:	call do_wp_page // 为 1，掉员工写保护出错处理函数
2:	addl $8,%esp // 丢弃输入参数（出错地址和出错码）
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret
