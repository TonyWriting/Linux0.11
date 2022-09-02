/*
 *  linux/kernel/system_call.s
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *  system_call.s  contains the system-call low-level handling routines.
 * This also contains the timer-interrupt handler, as some of the code is
 * the same. The hd- and flopppy-interrupts are also here.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call. Ordinary interrupts
 * don't handle signal-recognition, as that would clutter them up totally
 * unnecessarily.
 *
 * Stack layout in 'ret_from_system_call':
 *
 *	 0(%esp) - %eax
 *	 4(%esp) - %ebx
 *	 8(%esp) - %ecx
 *	 C(%esp) - %edx
 *	10(%esp) - %fs
 *	14(%esp) - %es
 *	18(%esp) - %ds
 *	1C(%esp) - %eip
 *	20(%esp) - %cs
 *	24(%esp) - %eflags
 *	28(%esp) - %oldesp
 *	2C(%esp) - %oldss
 */

SIG_CHLD	= 17

EAX		= 0x00
EBX		= 0x04
ECX		= 0x08
EDX		= 0x0C
FS		= 0x10
ES		= 0x14
DS		= 0x18
EIP		= 0x1C
CS		= 0x20
EFLAGS		= 0x24
OLDESP		= 0x28
OLDSS		= 0x2C

state	= 0		# these are offsets into the task-struct.
counter	= 4
priority = 8
signal	= 12
sigaction = 16		# MUST be 16 (=len of sigaction)
blocked = (33*16)

# offsets within sigaction
sa_handler = 0
sa_mask = 4
sa_flags = 8
sa_restorer = 12

nr_system_calls = 72

/*
 * Ok, I get parallel printer interrupts while using the floppy for some
 * strange reason. Urgel. Now I just ignore them.
 */
.globl system_call,sys_fork,timer_interrupt,sys_execve
.globl hd_interrupt,floppy_interrupt,parallel_interrupt
.globl device_not_available, coprocessor_error

.align 2
bad_sys_call:
	movl $-1,%eax
	iret
.align 2
reschedule:
    // 注意这里将 ret_from_sys_call 压栈，是五段论的最后一步
    // schedule 是 C 函数，当它切换回来后，会继续执行 schedule 直到它的右括号
    // 然后 RET 返回，就会将 ret_from_sys_call 弹栈，跳转到此处执行
	pushl $ret_from_sys_call
	jmp schedule
.align 2
system_call:
	cmpl $nr_system_calls-1,%eax
	ja bad_sys_call
	push %ds // 已经是内核栈了
	push %es
	push %fs
	pushl %edx
	pushl %ecx		# push %ebx,%ecx,%edx as parameters
	pushl %ebx		# to the system call
	movl $0x10,%edx		# set up ds,es to kernel space
	mov %dx,%ds
	mov %dx,%es
	movl $0x17,%edx		# fs points to local data space
	mov %dx,%fs
	call sys_call_table(,%eax,4) // 该函数会修改 eax，作为返回值。为什么会将 eax 作为返回值呢？这是因为该函数为 C 函数（即使不是，它里面也会调用 C 函数，比如 sys_fork），编译器会生成相应汇编代码，即自动帮你做了
	pushl %eax // 将 sys_xxx 内核函数的返回值压栈
	movl current,%eax // current 是指向当前任务 task_struct 的指针，全局变量，将它赋给 eax
	cmpl $0,state(%eax)		# state，非 0 代表进程为阻塞，则会调度
	jne reschedule
	cmpl $0,counter(%eax)		# counter，0 代表进程的时间片用光，也会引起调度
	je reschedule # 不满足条件（该行的条件是当前任务的 counter 为 0)，会顺序往下执行
ret_from_sys_call: // 除了 system_call 可能会走到这里，其他中断处理函数
	movl current,%eax		# task[0] cannot have signals
	cmpl task,%eax
	je 3f
	cmpw $0x0f,CS(%esp)		# was old code segment supervisor ?
	jne 3f
	cmpw $0x17,OLDSS(%esp)		# was stack segment = 0x17 ?
	jne 3f
	movl signal(%eax),%ebx # 取信号位图到 ebx，每一位代表一种信号，共 32 个信号
	movl blocked(%eax),%ecx # 取屏蔽位图到 ecx
	notl %ecx # 按位取反
	andl %ebx,%ecx # 对 ebx 和 ecx 进行与运算
	bsfl %ecx,%ecx # 从低位开始扫描，看是否有 1 的位。如果有则 ecx 保留该位的偏移量（0 - 31)。即只处理最低位的那个信号类型
	je 3f
	btrl %ecx,%ebx # 将 ebx 中偏移量为 ecx 的位清零（因为接下来会处理该信号，即将 ecx 信号类型作为参数传递给 do_signal）
	movl %ebx,signal(%eax)
	incl %ecx # 由于信号类型从 1 开始，因此要将 ecx 中的偏移量加 1
	pushl %ecx # 将 ecx 作为入参传递给 do_signal
	call do_signal // 在内核态返回用户态前，会查找进程的信号队列中是否有信号没有处理，如果有会调用 do_signal 处理信号
	popl %eax // 为什么要弹出 eax？因为前面 push 了，这是返回值
3:	popl %eax // 将一堆用户态的寄存器弹出栈，和 system_call 的压栈是相反的过程
	popl %ebx
	popl %ecx
	popl %edx
	pop %fs
	pop %es
	pop %ds
	iret

.align 2
coprocessor_error:
	push %ds
	push %es
	push %fs
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	movl $0x17,%eax
	mov %ax,%fs
	pushl $ret_from_sys_call
	jmp math_error

.align 2
device_not_available:
	push %ds
	push %es
	push %fs
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	movl $0x17,%eax
	mov %ax,%fs
	pushl $ret_from_sys_call
	clts				# clear TS so that we can use math
	movl %cr0,%eax
	testl $0x4,%eax			# EM (math emulation bit)
	je math_state_restore
	pushl %ebp
	pushl %esi
	pushl %edi
	call math_emulate
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 2
timer_interrupt: # 通过一片 8253 进行系统定时，每隔 10ms 发出一个时钟中断
	push %ds		# save ds,es and put kernel data space
	push %es		# into them. %fs is used by _system_call
	push %fs
	pushl %edx		# we save %eax,%ecx,%edx as gcc doesn't
	pushl %ecx		# save those across function calls. %ebx
	pushl %ebx		# is saved as we use that in ret_sys_call
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	movl $0x17,%eax
	mov %ax,%fs
	incl jiffies # 增加全局变量时钟滴答数
	movb $0x20,%al		# EOI to interrupt controller #1
	outb %al,$0x20
	movl CS(%esp),%eax
	andl $3,%eax		# %eax is CPL (0 or 3, 0=supervisor)
	pushl %eax
	call do_timer		# 'do_timer(long CPL)' does everything from
	addl $4,%esp		# task switching to accounting ...
	jmp ret_from_sys_call

.align 2
sys_execve:
	lea EIP(%esp),%eax
	pushl %eax
	call do_execve
	addl $4,%esp
	ret

.align 2
sys_fork: // 汇编非 C 函数
	call find_empty_process // 分配 pid 与找空的 task_struct 存放新进程
	testl %eax,%eax // Test对两个参数(目标，源)执行AND逻辑操作，并根据结果设置标志寄存器，但不会覆盖寄存器原有内容
	js 1f // 为负则跳转到 1 处
	push %gs // 压入 2 字节？不是，虽然 gs 是 16 位的，但是压栈会压入 4 个字节，其高 2 个字节为空
	pushl %esi // pushl 压入双字，即 4 字节 32 位的数据
	pushl %edi
	pushl %ebp
	pushl %eax // 此时 eax 是 C 函数 find_empty_process 的返回值了（不再是 system_call 的系统调用号，而是 tasks 中空闲的 task_sturct 的标号）
    // copy_process 是个 C 函数，它需要一堆入参，从哪里来呢？
    // 显然是从之前往内核栈中 push 的那些父进程的寄存器，注意越后入栈的作为 C 函数越左边的入参。
	// call == （push 下一条指令地址） + （jmp 目标函数地址）；ret == pop %eip（将当前栈内容 esp 赋给 eip）
	// 在 32 位 C 函数调用中，一般会通过栈帧结构（ebp + esp）维护函数调用栈，从而退出函数的时候，函数栈能够恢复成原本未进入被调函数的样子
	call copy_process // 这里会改变 eax（作为 C 函数的返回值）
	addl $20,%esp // 栈顶回退 20 个字节。为什么是 20 个字节呢？gs（16 位段寄存器，但用了 4 个字节存储） + esi + edi + ebp + eax == 20 个字节
1:	ret

hd_interrupt:
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	movl $0x17,%eax
	mov %ax,%fs
	movb $0x20,%al
	outb %al,$0xA0		# EOI to interrupt controller #1
	jmp 1f			# give port chance to breathe
1:	jmp 1f
1:	xorl %edx,%edx
	xchgl do_hd,%edx
	testl %edx,%edx
	jne 1f
	movl $unexpected_hd_interrupt,%edx
1:	outb %al,$0x20
	call *%edx		# "interesting" way of handling intr.
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret

floppy_interrupt:
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	movl $0x17,%eax
	mov %ax,%fs
	movb $0x20,%al
	outb %al,$0x20		# EOI to interrupt controller #1
	xorl %eax,%eax
	xchgl do_floppy,%eax
	testl %eax,%eax
	jne 1f
	movl $unexpected_floppy_interrupt,%eax
1:	call *%eax		# "interesting" way of handling intr.
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret

parallel_interrupt:
	pushl %eax
	movb $0x20,%al
	outb %al,$0x20
	popl %eax
	iret
