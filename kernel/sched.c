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

static union task_union init_task = {INIT_TASK,};

long volatile jiffies=0; // 记录从开机到当前时间的时钟中断发生次数（溢出了怎么办？）
long startup_time=0;
struct task_struct *current = &(init_task.task);
struct task_struct *last_task_used_math = NULL;

struct task_struct * task[NR_TASKS] = {&(init_task.task), };

long user_stack [ PAGE_SIZE>>2 ] ;

struct {
	long * a;
	short b;
	} stack_start = { & user_stack [PAGE_SIZE>>2] , 0x10 };
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

    for (p = &LAST_TASK; p > &FIRST_TASK; --p)
        if (*p)
        {
            if ((*p)->alarm && (*p)->alarm < jiffies)
            {
                (*p)->signal |= (1 << (SIGALRM - 1));
                (*p)->alarm = 0;
            }
            if (((*p)->signal & ~(_BLOCKABLE & (*p)->blocked)) && (*p)->state == TASK_INTERRUPTIBLE)
            {
                (*p)->state = TASK_RUNNING;
                // 如果进程 p 的信号位图中除去被阻塞的信号还有其他信号，那么它从阻塞态（W）切换为就绪态（J）
#if VERBOSE
                fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", (*p)->pid, 'J', jiffies, "schedule", (*p)->father);
#else
                fprintk(3, "%ld\t%c\t%ld\n", (*p)->pid, 'J', jiffies);
#endif
            }
        }

    /* this is the scheduler proper: */
    while (1)
    {
        c = -1;
        // 初始值为 0，如果没有可调度程序，会调度任务 0（虽然此时的任务 0 可能是阻塞态）。任务 0 只会执行系统调用 pause，又会进入这里
        // 因此任务 0 是唯一一个可能从阻塞态到运行态的
        next = 0;
        i = NR_TASKS;
        p = &task[NR_TASKS];
        while (--i)
        {
            if (!*--p)
                continue;                                          // 跳过不含任务的任务槽
            if ((*p)->state == TASK_RUNNING && (*p)->counter > c)  // 两个条件：其一就绪，其二 counter 最大
                c = (*p)->counter, next = i;
        }
        if (c)
            break;  // 如果存在某一个进程的 counter 不为 0（代表时间片未用完），或者没有可以运行的任务（c == -1）则跳出循环
        for (p = &LAST_TASK; p > &FIRST_TASK; --p)
            if (*p)
                (*p)->counter = ((*p)->counter >> 1) + (*p)->priority;
    }
    // 如果 next 不为当前进程 current，那么在 switch_to 会将 current 切换为就绪态（J），next 切换为运行态（R）
    // Linux 0.11 用 TASK_RUNNING 同时表示就绪态（J）和运行态（R），所以源码里不需要改变 current 和 next 的 state
    // 但是按照本实验要求，需要将它们作区分并打印出来
    if (current->pid != task[next]->pid)
    {
        // 判断 current 是否为运行态（R），因为进程阻塞时也有可能会调用 schedule 到达这里
        if (current->state == TASK_RUNNING)
        {
#if VERBOSE
            fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", current->pid, 'J', jiffies, "schedule", current->father);
#else
            fprintk(3, "%ld\t%c\t%ld\n", current->pid, 'J', jiffies);
#endif
        }
#if VERBOSE
        fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", task[next]->pid, 'R', jiffies, "schedule", task[next]->father);
#else
        fprintk(3, "%ld\t%c\t%ld\n", task[next]->pid, 'R', jiffies);
#endif
    }
    switch_to(next);
}

// 系统无事可做的时候，进程 0 会始终循环调用 sys_pause()，以激活调度算法
// 此时它的状态可以是等待态，等待有其它可运行的进程；也可以叫运行态，因为它是唯一一个在 CPU 上运行的进程，只不过运行的效果是等待
// 这里采用第二种方式，因为如果用第一种的方式，那么 /var/process.log 会多出来许多进程 0 的状态切换而冗杂
// 因此，打印的时候需要判断当前任务是否为 0，如果是则不进行打印
int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE;
	if (current->pid != FIRST_TASK->pid)
	{
#if VERBOSE
        fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", current->pid, 'W', jiffies, "sys_pause", current->father);
#else
		fprintk(3, "%ld\t%c\t%ld\n", current->pid, 'W', jiffies);
#endif
	}
	schedule();
	return 0;
}

void sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task))
		panic("task[0] trying to sleep");
	tmp = *p;
	*p = current;
	current->state = TASK_UNINTERRUPTIBLE;
#if VERBOSE
    fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", current->pid, 'W', jiffies, "sleep_on", current->father);
#else
	fprintk(3, "%ld\t%c\t%ld\n", current->pid, 'W', jiffies);
#endif
	schedule();
	if (tmp)
	{
		tmp->state=0;
#if VERBOSE
        fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", tmp->pid, 'J', jiffies, "sleep_on", tmp->father);
#else
		fprintk(3, "%ld\t%c\t%ld\n", tmp->pid, 'J', jiffies);
#endif
	}
}

void interruptible_sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task))
		panic("task[0] trying to sleep");
	tmp=*p;
	*p=current;
repeat:	current->state = TASK_INTERRUPTIBLE;
#if VERBOSE
    fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", current->pid, 'W', jiffies, "interruptible_sleep_on", current->father);
#else
	fprintk(3, "%ld\t%c\t%ld\n", current->pid, 'W', jiffies);
#endif
	schedule();
	if (*p && *p != current) {
		(**p).state=0;
#if VERBOSE
        fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", (**p).pid, 'J', jiffies, "interruptible_sleep_on", (**p).father);
#else
		fprintk(3, "%ld\t%c\t%ld\n", (**p).pid, 'J', jiffies);
#endif
		goto repeat;
	}
	*p=NULL;
	if (tmp)
	{
		tmp->state=0;
#if VERBOSE
        fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", tmp->pid, 'J', jiffies, "interruptible_sleep_on", tmp->father);
#else
		fprintk(3, "%ld\t%c\t%ld\n", tmp->pid, 'J', jiffies);
#endif
	}
}

void wake_up(struct task_struct **p)
{
    if (p && *p)
    {
        (**p).state = 0;
#if VERBOSE
        fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", (**p).pid, 'J', jiffies, "wake_up", (**p).father);
#else
        fprintk(3, "%ld\t%c\t%ld\n", (**p).pid, 'J', jiffies);
#endif
        *p = NULL;
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
	if ((--current->counter)>0) return;
	current->counter=0;
	if (!cpl) return;
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
	set_tss_desc(gdt+FIRST_TSS_ENTRY,&(init_task.task.tss));
	set_ldt_desc(gdt+FIRST_LDT_ENTRY,&(init_task.task.ldt));
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
