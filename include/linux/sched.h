#ifndef _SCHED_H
#define _SCHED_H

#define NR_TASKS 64 // Linux 0.11 设定最大的任务数为 NR_TASKS(64)
#define HZ 100

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

#include <linux/head.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <signal.h>

#if (NR_OPEN > 32)
#error "Currently the close-on-exec-flags are in one word, max 32 files/proc"
#endif

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4

#ifndef NULL
#define NULL ((void *) 0)
#endif

extern int copy_page_tables(unsigned long from, unsigned long to, long size);
extern int free_page_tables(unsigned long from, unsigned long size);

extern void sched_init(void);
extern void schedule(void);
extern void trap_init(void);
#ifndef PANIC
volatile void panic(const char * str);
#endif
extern int tty_write(unsigned minor,char * buf,int count);

typedef int (*fn_ptr)();

struct i387_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
};

struct tss_struct {
	long	back_link;	/* 16 high bits zero */
	long	esp0;
	long	ss0;		/* 16 high bits zero */
	long	esp1;
	long	ss1;		/* 16 high bits zero */
	long	esp2;
	long	ss2;		/* 16 high bits zero */
	long	cr3;
	long	eip;
	long	eflags;
	long	eax,ecx,edx,ebx;
	long	esp;
	long	ebp;
	long	esi;
	long	edi;
	long	es;		/* 16 high bits zero */
	long	cs;		/* 16 high bits zero */
	long	ss;		/* 16 high bits zero */
	long	ds;		/* 16 high bits zero */
	long	fs;		/* 16 high bits zero */
	long	gs;		/* 16 high bits zero */
	long	ldt;		/* 16 high bits zero */
	long	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
	struct i387_struct i387;
};

struct task_struct {
/* these are hardcoded - don't touch */ // 由于 system_call.s 中会使用以下字段的字节偏移量，所以它们的顺序不能随意调整
	long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	long counter;
	long priority;
	long signal;
	struct sigaction sigaction[32];
	long blocked;	/* bitmap of masked signals */
/* various fields */
	int exit_code;
	// start_code 是进程的代码段/数据段的起始地址，会被赋值为任务标号 * 64MB（每个进程在内存的虚拟空间均为 64MB），start_stack 是栈的起始地址
	// end_code 和 end_data 分别是代码段的长度和数据段的长度（单位是字节）。brk 是总的数据长度，它指出了进程代码和数据（包括动态分配的数据空间）在虚拟空间的末端位置
	// 这里的地址都是指在 64MB 虚拟空间内的偏移地址，由于虚拟地址都是 32 位，因此它们都用了长整型
	unsigned long start_code,end_code,end_data,brk,start_stack;
	// pid 是进程的唯一标识，father 是父进程的 PID，这样就形成了“家谱图”
	// pgrp 是组的标识，该值相同的进程属于同一个组
	// session 用于标识不同来访的用户进程，由这些进程产生的子进程和相应的父进程的 session 值相同
	// 来访的进程应该有一个“头”，leader 用于标识这个头
	long pid,father,pgrp,session,leader;
	unsigned short uid,euid,suid;
	unsigned short gid,egid,sgid;
	long alarm; // 进程睡眠时间，为 0 表示没有睡眠
	long utime,stime,cutime,cstime,start_time;
	unsigned short used_math;
/* file system info */
	int tty;		/* -1 if no tty, so it must be signed */
	unsigned short umask;
	struct m_inode * pwd;
	struct m_inode * root;
	struct m_inode * executable;
	unsigned long close_on_exec;
	struct file * filp[NR_OPEN];
/* ldt for this task 0 - zero 1 - cs 2 - ds&ss */
    // LDT 表本身是一个段，但 GDT 不是
    // LDT 表中存放的东西和 GDT 一样，都是段描述符
	struct desc_struct ldt[3]; // 每个任务的整张 LDT 表（通过有三个段描述符，第一个为空）也是放在 task_struct 里的！
/* tss for this task */
	struct tss_struct tss; // tss 中有 ldt 选择符（在 GDT 中的索引）
};

/*
 *  INIT_TASK is used to set up the first task table, touch at
 * your own risk!. Base=0, limit=0x9ffff (=640kB)
 */
// 下面定义了第一个任务（任务 0）的 PCB（内核的 GDT 是在 head.s 中设置的，内核进程（内核并没有进程）的 PCB 放在哪里呢？但内核进程有没有 PCB 呢？好像没有。。。任务 0 是用户程序）
// 应注意，当任务 0 通过 move_to_user_mode 开始运行时，TSS 段中保存的寄存器和这里的有些不一样
#define INIT_TASK \
/* state etc */	{ 0,15,15, \
/* signals */	0,{{},},0, \
/* ec,brk... */	0,0,0,0,0,0, \
/* pid etc.. */	0,-1,0,0,0, \ // 任务 0 的 pid 为 0
/* uid etc */	0,0,0,0,0,0, \
/* alarm */	0,0,0,0,0,0, \
/* math */	0, \
/* fs info */	-1,0022,NULL,NULL,NULL,0, \
/* filp */	{NULL,}, \
	{ \
		{0,0}, \ // ldt 表中的第一个段描述符为空
/* ldt */	{0x9f,0xc0fa00}, \ // 任务 0 的代码段描述符，基址为 0，段限长为 640KB
		{0x9f,0xc0f200}, \ // 任务 0 的数据段描述符，基址为 0，段限长为 640KB
	}, \
/*tss*/	{0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)&pg_dir,\
	 0,0,0,0,0,0,0,0, \
	 0,0,0x17,0x17,0x17,0x17,0x17,0x17, \ // 0x17 == 000010111，即（任务 0 的）LDT 表中的数据段选择子，但为什么 CS 也指向了数据段呢？（虽然 move_to_user_mode 后 CS 指向的是代码段）
	 _LDT(0),0x80000000, \ // 任务 0 的 LDT 选择符为 0b00101000，即索引为 0b101 == 5，确实是从第五个开始（前面有两个用户内核代码/数据段描述符，两个不用）
		{} \
	}, \
}

extern struct task_struct *task[NR_TASKS];
extern struct task_struct *last_task_used_math;
extern struct task_struct *current;
extern long volatile jiffies;
extern long startup_time;

#define CURRENT_TIME (startup_time+jiffies/HZ)

extern void add_timer(long jiffies, void (*fn)(void));
extern void sleep_on(struct task_struct ** p);
extern void interruptible_sleep_on(struct task_struct ** p);
extern void wake_up(struct task_struct ** p);

/*
 * Entry into gdt where to find first TSS. 0-nul, 1-cs, 2-ds, 3-syscall
 * 4-TSS0, 5-LDT0, 6-TSS1 etc ...
 */
#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
 // _TSS(n) 应该为第 n 个 TSS 段的段选择符的内存表示
 // 段选择符 == 索引 + TI + RPL。对于 TSS 和 LDT 段选择子，其后三位（即 TI + RPL）全为 0，因为它们在 GDT 表中，并且特权级为 0
 // 这就是为什么 FIRST_TSS_ENTRY 要左移 3 位
 // 又因为相邻任务之间的 TSS 段选择符的索引差距为 2，因此 n 要左移 4 位
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))

#define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3)) // LDT 的位置查找方式和 TSS 的类似，它也是 LDT 段选择符
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))
#define str(n) \
__asm__("str %%ax\n\t" \
	"subl %2,%%eax\n\t" \
	"shrl $4,%%eax" \
	:"=a" (n) \
	:"a" (0),"i" (FIRST_TSS_ENTRY<<3))
/*
 *	switch_to(n) should switch tasks to task nr n, first
 * checking that n isn't the current task, in which case it does nothing.
 * This also clears the TS-flag if the task we switched to has used
 * tha math co-processor latest.
 */
#define switch_to(n) {\
struct {long a,b;} __tmp; \ // __tmp.a 存放 32 位偏移（无用，忽略），__tmp.b 存放 TSS 段选择符
__asm__("cmpl %%ecx,current\n\t" \ // 判断新任务 n 是否为当前任务
	"je 1f\n\t" \ // 如果是当前任务，则跳转到标号 1 退出
	"movw %%dx,%1\n\t" \ // 将 TSS 段选择符 _TSS(n) 赋值给 __tmp.b。movw 表示赋值两个字节（段选择子为两个字节）
	"xchgl %%ecx,current\n\t" \ // 交换 ecx 和 current 的内容，交换后 ecx 指向当前进程，current 指向将要切换过去的进程
    // ljmp 需要传递给它 64 位的数据，%0 代表 __tmp.a（虽然它没用）
    // 长跳到段选择符会造成任务切换，切换后后面的代码不会执行了，只有等到重新切换回来才会继续执行（怎么找回来下一行代码的地址？ TSS 段里面有cs,eip）
    // 一条 ljmp 硬件自动完成当前物理前寄存器状态压入当前任务的 TSS 段中，并将 _TSS(n) 的寄存器状态赋给物理寄存器
    // 串起来：TSS 段选择符 -> TSS 段描述符（其中存放了相应 TSS 的段基址。非任务 0 的任务在 fork 时设置它，任务 0 在 sched_init 中设置它）
	// -> TSS 段（存放在 task_struct 结构体中，非任务 0 的任务在 fork 时设置它，任务 0 手写在 INIT_TASK 宏中）
	"ljmp *%0\n\t" \
	"cmpl %%ecx,last_task_used_math\n\t" \
	"jne 1f\n\t" \
	"clts\n" \
	"1:" \
	::"m" (*&__tmp.a),"m" (*&__tmp.b), \ // 不知道为什么要先取地址再取值，多次一举？
	"d" (_TSS(n)),"c" ((long) task[n])); \
}

#define PAGE_ALIGN(n) (((n)+0xfff)&0xfffff000)

#define _set_base(addr,base)  \
__asm__ ("push %%edx\n\t" \
	"movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %%dl,%1\n\t" \
	"movb %%dh,%2\n\t" \
	"pop %%edx" \
	::"m" (*((addr)+2)), \
	 "m" (*((addr)+4)), \
	 "m" (*((addr)+7)), \
	 "d" (base) \
	)

#define _set_limit(addr,limit) \
__asm__ ("push %%edx\n\t" \
	"movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %1,%%dh\n\t" \
	"andb $0xf0,%%dh\n\t" \
	"orb %%dh,%%dl\n\t" \
	"movb %%dl,%1\n\t" \
	"pop %%edx" \
	::"m" (*(addr)), \
	 "m" (*((addr)+6)), \
	 "d" (limit) \
	)

#define set_base(ldt,base) _set_base( ((char *)&(ldt)) , (base) )
#define set_limit(ldt,limit) _set_limit( ((char *)&(ldt)) , (limit-1)>>12 )

/**
#define _get_base(addr) ({\
unsigned long __base; \
__asm__("movb %3,%%dh\n\t" \
	"movb %2,%%dl\n\t" \
	"shll $16,%%edx\n\t" \
	"movw %1,%%dx" \
	:"=d" (__base) \
	:"m" (*((addr)+2)), \
	 "m" (*((addr)+4)), \
	 "m" (*((addr)+7)) \
        :"memory"); \
__base;})
**/

static inline unsigned long _get_base(char * addr)
{
         unsigned long __base;
         __asm__("movb %3,%%dh\n\t"
                 "movb %2,%%dl\n\t"
                 "shll $16,%%edx\n\t"
                 "movw %1,%%dx"
                 :"=&d" (__base)
                 :"m" (*((addr)+2)),
                  "m" (*((addr)+4)),
                  "m" (*((addr)+7)));
         return __base;
}

#define get_base(ldt) _get_base( ((char *)&(ldt)) )

// 取出段选择符 segment 指定的段选择符中的段限长值
#define get_limit(segment) ({ \
unsigned long __limit; \
// lsl 是 load segment limit 的缩写，它能够从段描述符中取出分散的比特位拼接处段限长
// 因所得的段限长为实际字节数减1，因此还需要通过 inc 来加一后返回
__asm__("lsll %1,%0\n\tincl %0":"=r" (__limit):"r" (segment)); \
__limit;})

#endif
