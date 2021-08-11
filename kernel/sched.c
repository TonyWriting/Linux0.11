/*
 *  linux/kernel/sched.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * 'sched.c' is the main kernel file. It contains scheduling primitives
 * (sleep_on, wakeup, schedule etc) as well as a number of simple system
 * call functions (type getpid(), which just extracts a field from
 * current-task
 */
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/sys.h>
#include <linux/fdreg.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>

#include <signal.h>

#define _S(nr) (1<<((nr)-1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

void show_task(int nr,struct task_struct * p)
{
	int i,j = 4096-sizeof(struct task_struct);

	printk("%d: pid=%d, state=%d, ",nr,p->pid,p->state);
	i=0;
	while (i<j && !((char *)(p+1))[i])
		i++;
	printk("%d (of %d) chars free in kernel stack\n\r",i,j);
}

void show_stat(void)
{
	int i;

	for (i=0;i<NR_TASKS;i++)
		if (task[i])
			show_task(i,task[i]);
}

#define LATCH (1193180/HZ)

extern void mem_use(void);

extern int timer_interrupt(void);
extern int system_call(void);

union task_union {
	struct task_struct task;
	char stack[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK,}; // 存放进程 0 的 PCB 及其内核栈，内核栈应该没有被赋值

long volatile jiffies=0; // 记录了从开机以来经过了多少个滴答/时钟中断（10 ms)
long startup_time=0;
struct task_struct *current = &(init_task.task);
struct task_struct *last_task_used_math = NULL;

struct task_struct * task[NR_TASKS] = {&(init_task.task), };

long user_stack [ PAGE_SIZE>>2 ] ; // 内核程序的栈（也是任务 0 的用户栈），PAGE_SIZE == 4096，PAGE_SIZE >> == 1024；又 long 为 4B，因此栈大小为 4KB

struct { // 在 boot/head.s 的开始执行时便通过 lss stack_start, %esp 设置了内核程序使用的栈
	long * a;
	short b;
	} stack_start = { & user_stack [PAGE_SIZE>>2] , 0x10 }; // 0x10 是指向内核数据段的段选择符，即内核程序和任务 0 的用户栈的 ss 都指向着内核数据段
/*
 *  'math_state_restore()' saves the current math information in the
 * old math state array, and gets the new ones from the current task
 */
void math_state_restore()
{
	if (last_task_used_math == current)
		return;
	__asm__("fwait");
	if (last_task_used_math) {
		__asm__("fnsave %0"::"m" (last_task_used_math->tss.i387));
	}
	last_task_used_math=current;
	if (current->used_math) {
		__asm__("frstor %0"::"m" (current->tss.i387));
	} else {
		__asm__("fninit"::);
		current->used_math=1;
	}
}

/*
 *  'schedule()' is the scheduler function. This is GOOD CODE! There
 * probably won't be any reason to change this, as it should work well
 * in all circumstances (ie gives IO-bound processes good response etc).
 * The one thing you might take a look at is the signal-handler code here.
 *
 *   NOTE!!  Task 0 is the 'idle' task, which gets called when no other
 * tasks can run. It can not be killed, and it cannot sleep. The 'state'
 * information in task[0] is never used.
 */
void schedule(void)
{
	int i,next,c;
	struct task_struct ** p;

/* check alarm, wake up any interruptible tasks that have got a signal */

	for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
		if (*p) {
			if ((*p)->alarm && (*p)->alarm < jiffies) {
					(*p)->signal |= (1<<(SIGALRM-1));
					(*p)->alarm = 0;
				}
			if (((*p)->signal & ~(_BLOCKABLE & (*p)->blocked)) &&
			(*p)->state==TASK_INTERRUPTIBLE)
				(*p)->state=TASK_RUNNING;
		}

/* this is the scheduler proper: */

	while (1) {
		c = -1;
		next = 0; // 初始值为 0，如果没有可调度程序，会调度任务 0（虽然此时的任务 0 可能是阻塞态）。任务 0 只会执行系统调用 pause，又会进入这里
		i = NR_TASKS; // 从后往前遍历 PCB
		p = &task[NR_TASKS]; // p 类型为 task_struct **，即指针的地址
		while (--i) {
			if (!*--p)
				continue; // 跳过不含任务的任务槽
			if ((*p)->state == TASK_RUNNING && (*p)->counter > c) // 两个条件：其一就绪，其二 counter 最大
				c = (*p)->counter, next = i; // 取最大的 counter（运行时间片最短）
		}
		if (c) break; // 如果存在某一个进程的 counter 不为 0（代表时间片未用完），或者没有可以运行的任务（c == -1）则跳出循环
		for(p = &LAST_TASK ; p > &FIRST_TASK ; --p) // 指针是可以比较地址值大小的
			if (*p)
                // 有一点奇怪，运行到这里时，应该有全部进程的 counter 为 0。那么 (*p)->counter >> 1 是不是就多余了？不是，因为处于阻塞态任务的 counter 可能不为 0
				(*p)->counter = ((*p)->counter >> 1) + (*p)->priority; // 更新优先级（时间片）
	}
    // 切换到另一个任务后，switch_to 同时实现指令流和 TSS 的切换，即其后半部分代码不会执行
	switch_to(next);
} // 该括号不会执行，只有在原任务被切换回来才会继续执行

int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

// 将当前任务置为不可中断的等待状态，并让睡眠队列头指针指向当前任务
// Linux 0.11 未采用真正链表的形式，而是通过内核栈中的 tmp 隐式串联起来
// wake_up 总是唤醒队列头指针指向的任务，而在 tmp 里面保存了后一个任务的地址
// 对于不可中断睡眠 (TASK_UNINTERRUPTIBLE) 只能由 wake_up 函数显式地从隐式等待队列头部唤醒队列头进程
// 再由这个队列头部进程执行 tmp->state = 0 依次唤醒等待队列
void sleep_on(struct task_struct **p) // *p 为等待队列头指针，它总是指向最前面的等待任务
{
	struct task_struct *tmp; // 临时指针，存放在当前任务的内核栈中

	if (!p)
		return;
	if (current == &(init_task.task)) // current 为当前任务的指针
		panic("task[0] trying to sleep");
	tmp = *p; // tmp 指向队列中队列头指针指向的原等待任务
	*p = current; // 队列头指针指向新加入的等待任务，即调用本函数的任务
	current->state = TASK_UNINTERRUPTIBLE; // 将当前任务设置为不可中断等待状态
	schedule(); // 本任务睡眠，切换到其他任务去了

    // 当调用本函数的任务被唤醒并重新执行时（注意任务被唤醒后不一定会立刻执行它），将比它早进入等待队列中的那个任务唤醒进入就绪状态
    // 对于不可中断的睡眠，一定是严格按照“队列”的头部进行唤醒
	if (tmp)
		tmp->state=0;
}

// 除了 wake_up 之外，可以用信号（存放在进程 PCB 中一个向量的某个位置）唤醒
// 与不可中断睡眠不同，它会遇到一个问题，即可能唤醒等待队列中间的某个进程，此时需要适当调整队列。这是两个函数的主要区别
void interruptible_sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task))
		panic("task[0] trying to sleep");
	tmp=*p;
	*p=current;
repeat:	current->state = TASK_INTERRUPTIBLE; // 将当前任务设置为可中断等待状态
	schedule();
    // 当调用本函数的任务被唤醒并重新执行时，判断自己是否为队列头。如果不是（信号唤醒），则将队列头唤醒，并将自己睡眠，重新调度
    // 为什么后来的先运行？
	if (*p && *p != current) {
		(**p).state=0;
		goto repeat;
	}
	*p=NULL;
	if (tmp)
		tmp->state=0;
}

// 唤醒 *p 指向的任务（等待队列头指针）
// 由于新等待任务时插入在等待队列头部的，因此唤醒的是最后进入等待队列的任务
void wake_up(struct task_struct **p)
{
	if (p && *p) {
		(**p).state=0;
		*p=NULL;
	}
}

/*
 * OK, here are some floppy things that shouldn't be in the kernel
 * proper. They are here because the floppy needs a timer, and this
 * was the easiest way of doing it.
 */
static struct task_struct * wait_motor[4] = {NULL,NULL,NULL,NULL};
static int  mon_timer[4]={0,0,0,0};
static int moff_timer[4]={0,0,0,0};
unsigned char current_DOR = 0x0C;

int ticks_to_floppy_on(unsigned int nr)
{
	extern unsigned char selected;
	unsigned char mask = 0x10 << nr;

	if (nr>3)
		panic("floppy_on: nr>3");
	moff_timer[nr]=10000;		/* 100 s = very big :-) */
	cli();				/* use floppy_off to turn it off */
	mask |= current_DOR;
	if (!selected) {
		mask &= 0xFC;
		mask |= nr;
	}
	if (mask != current_DOR) {
		outb(mask,FD_DOR);
		if ((mask ^ current_DOR) & 0xf0)
			mon_timer[nr] = HZ/2;
		else if (mon_timer[nr] < 2)
			mon_timer[nr] = 2;
		current_DOR = mask;
	}
	sti();
	return mon_timer[nr];
}

void floppy_on(unsigned int nr)
{
	cli();
	while (ticks_to_floppy_on(nr))
		sleep_on(nr+wait_motor);
	sti();
}

void floppy_off(unsigned int nr)
{
	moff_timer[nr]=3*HZ;
}

void do_floppy_timer(void)
{
	int i;
	unsigned char mask = 0x10;

	for (i=0 ; i<4 ; i++,mask <<= 1) {
		if (!(mask & current_DOR))
			continue;
		if (mon_timer[i]) {
			if (!--mon_timer[i])
				wake_up(i+wait_motor);
		} else if (!moff_timer[i]) {
			current_DOR &= ~mask;
			outb(current_DOR,FD_DOR);
		} else
			moff_timer[i]--;
	}
}

#define TIME_REQUESTS 64

static struct timer_list {
	long jiffies;
	void (*fn)();
	struct timer_list * next;
} timer_list[TIME_REQUESTS], * next_timer = NULL;

void add_timer(long jiffies, void (*fn)(void))
{
	struct timer_list * p;

	if (!fn)
		return;
	cli();
	if (jiffies <= 0)
		(fn)();
	else {
		for (p = timer_list ; p < timer_list + TIME_REQUESTS ; p++)
			if (!p->fn)
				break;
		if (p >= timer_list + TIME_REQUESTS)
			panic("No more time requests free");
		p->fn = fn;
		p->jiffies = jiffies;
		p->next = next_timer;
		next_timer = p;
		while (p->next && p->next->jiffies < p->jiffies) {
			p->jiffies -= p->next->jiffies;
			fn = p->fn;
			p->fn = p->next->fn;
			p->next->fn = fn;
			jiffies = p->jiffies;
			p->jiffies = p->next->jiffies;
			p->next->jiffies = jiffies;
			p = p->next;
		}
	}
	sti();
}

void do_timer(long cpl)
{
	extern int beepcount;
	extern void sysbeepstop(void);

	if (beepcount)
		if (!--beepcount)
			sysbeepstop();

	if (cpl)
		current->utime++;
	else
		current->stime++;

	if (next_timer) {
		next_timer->jiffies--;
		while (next_timer && next_timer->jiffies <= 0) {
			void (*fn)(void);
			
			fn = next_timer->fn;
			next_timer->fn = NULL;
			next_timer = next_timer->next;
			(fn)();
		}
	}
	if (current_DOR & 0xf0)
		do_floppy_timer();
	if ((--current->counter)>0) return; // 如果时间片没用完，那么直接返回，不进行调度
	current->counter=0;
	if (!cpl) return; // 如果 cpl == 0（内核态），则不会进行调度，否则会调度。因此 linux 0.11 的内核态是不可抢占的
	schedule();
}

int sys_alarm(long seconds)
{
	int old = current->alarm;

	if (old)
		old = (old - jiffies) / HZ;
	current->alarm = (seconds>0)?(jiffies+HZ*seconds):0;
	return (old);
}

int sys_getpid(void)
{
	return current->pid;
}

int sys_getppid(void)
{
	return current->father;
}

int sys_getuid(void)
{
	return current->uid;
}

int sys_geteuid(void)
{
	return current->euid;
}

int sys_getgid(void)
{
	return current->gid;
}

int sys_getegid(void)
{
	return current->egid;
}

int sys_nice(long increment)
{
	if (current->priority-increment>0)
		current->priority -= increment;
	return 0;
}

void sched_init(void)
{
	int i;
	struct desc_struct * p;

	if (sizeof(struct sigaction) != 16)
		panic("Struct sigaction MUST be 16 bytes");
    // 因为任务 0 是第一个进程，不是 fork 出来的，所以需要额外设置
    // 任务 0 严格意义上是用户程序了，虽然它的代码段和数据段和内核的是有重叠的
	set_tss_desc(gdt+FIRST_TSS_ENTRY,&(init_task.task.tss)); // 设置任务 0 的 TSS 段描述符，TSS 段本身在 sched.h 的 INIT_TASK 设置
	set_ldt_desc(gdt+FIRST_LDT_ENTRY,&(init_task.task.ldt)); // 设置任务 0 的 LDT 段描述符，LDT 段本身在 sched.h 的 INIT_TASK 设置
	p = gdt+2+FIRST_TSS_ENTRY;
	for(i=1;i<NR_TASKS;i++) {
		task[i] = NULL;
		p->a=p->b=0;
		p++;
		p->a=p->b=0;
		p++;
	}
/* Clear NT, so that we won't have troubles with that later on */
	__asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
	ltr(0);
	lldt(0);
	outb_p(0x36,0x43);		/* binary, mode 3, LSB/MSB, ch 0 */
	outb_p(LATCH & 0xff , 0x40);	/* LSB */
	outb(LATCH >> 8 , 0x40);	/* MSB */
	set_intr_gate(0x20,&timer_interrupt);
	outb(inb_p(0x21)&~0x01,0x21);
	set_system_gate(0x80,&system_call);
}
