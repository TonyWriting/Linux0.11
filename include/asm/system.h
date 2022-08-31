#define move_to_user_mode() \
__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x17\n\t" \ // 0x17 是指 LDT 表中的数据段选择符
	"pushl %%eax\n\t" \ // esp 依然是内核程序使用的 user_stack 原本的位置值
	"pushfl\n\t" \ // 将 EFLAGS 寄存器的全部内容压入堆栈
	"pushl $0x0f\n\t" \ // 0x0f 是指 LDT 表中的代码段选择符，因为所有进程都是 fork 进程 0 的，因为所有进程的代码段选择子 CS 都为 0x0f
	"pushl $1f\n\t" \ // 将 iret 的下一条指令压栈
	"iret\n" \ // 会将上面五个弹出，继续向下执行代码，但特权级从 0 变成 3，变成了用户程序（任务 0）
	"1:\tmovl $0x17,%%eax\n\t" \
	"movw %%ax,%%ds\n\t" \
	"movw %%ax,%%es\n\t" \
	"movw %%ax,%%fs\n\t" \
	"movw %%ax,%%gs" \
	:::"ax")

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)
#define nop() __asm__ ("nop"::)

#define iret() __asm__ ("iret"::)

#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define _set_seg_desc(gate_addr,type,dpl,base,limit) {\
	*(gate_addr) = ((base) & 0xff000000) | \
		(((base) & 0x00ff0000)>>16) | \
		((limit) & 0xf0000) | \
		((dpl)<<13) | \
		(0x00408000) | \
		((type)<<8); \
	*((gate_addr)+1) = (((base) & 0x0000ffff)<<16) | \
		((limit) & 0x0ffff); }

#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \ // 任务段和局部描述符表段的段限长都是 104 个字节
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)

// n 是段描述符的指针，addr 是段描述符中的段基址
#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x89") // 0x89 是任务段的类型
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x82") // 0x82 是局部描述符表段的类型

