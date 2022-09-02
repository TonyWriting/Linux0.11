/*
 *  linux/kernel/fork.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *  'fork.c' contains the help-routines for the 'fork' system call
 * (see also system_call.s), and some misc functions ('verify_area').
 * Fork is rather simple, once you get the hang of it, but the memory
 * management can be a bitch. See 'mm/mm.c': 'copy_page_tables()'
 */
#include <errno.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <asm/system.h>

extern void write_verify(unsigned long address);

long last_pid=0;

void verify_area(void * addr,int size)
{
	unsigned long start;

	start = (unsigned long) addr;
	size += start & 0xfff;
	start &= 0xfffff000;
	start += get_base(current->ldt[2]);
	while (size>0) {
		size -= 4096;
		write_verify(start);
		start += 4096;
	}
}

int copy_mem(int nr,struct task_struct * p)
{
	unsigned long old_data_base,new_data_base,data_limit;
	unsigned long old_code_base,new_code_base,code_limit;

	code_limit=get_limit(0x0f); // 取出父进程的代码段段限长（在 LDT 中）
	data_limit=get_limit(0x17); // 取出父进程的数据段段限长（在 LDT 中）
	old_code_base = get_base(current->ldt[1]); // 取出父进程的代码段段基址
	old_data_base = get_base(current->ldt[2]); // 取出父进程的数据段段基址
	if (old_data_base != old_code_base)
		panic("We don't support separate I&D");
	if (data_limit < code_limit)
		panic("Bad data_limit");
	new_data_base = new_code_base = nr * 0x4000000; // 设置子进程代码段和数据段在线性空间的基地址为：任务标号（注意区别于 pid）* 64MB，可见所有的段的数据段和代码段的基地址都是一样的
	p->start_code = new_code_base;
	set_base(p->ldt[1],new_code_base);
	set_base(p->ldt[2],new_data_base);
	if (copy_page_tables(old_data_base,new_data_base,data_limit)) { // 设置子进程的页目录表和页表项，即复制父进程的页目录表和页表项
		printk("free_page_tables: from copy_mem\n");
		free_page_tables(new_data_base,data_limit); // 失败则释放
		return -ENOMEM;
	}
	return 0;
}

/*
 *  Ok, this is the main fork-routine. It copies the system process
 * information (task[nr]) and sets up the necessary registers. It
 * also copies the data segment in it's entirety.
 */
int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
	struct task_struct *p;
	int i;
	struct file *f;

    // get_free_page 获得一页(4KB)内存（内存管理中会讲，不能用 malloc 因为它是用户态的库代码，内核中不能用）
    // 找到 mem_map 为 0（空闲）的那一页，将其地址返回。并且进行类型强制转换，即将该页内存作为 task_stuct(PCB)
    // 这一页 4KB 专门用来存放 task_struct 和内核栈
	p = (struct task_struct *) get_free_page();
	if (!p)
		return -EAGAIN;
	task[nr] = p; // 前面的 find_empty_process 已经保证了 task[nr] 是空闲的
	// 下面这行已经完成大部分的拷贝了，包括信号相关的 sigaction 以及文件相关的 filp 字段
	*p = *current;	/* NOTE! this doesn't copy the supervisor stack */
	// 之后需要对子进程 task_struct 的某些字段进行修改
	p->state = TASK_UNINTERRUPTIBLE;
	p->pid = last_pid;
	p->father = current->pid;
	p->counter = p->priority; // 复位时间片计数器，一般为 15 个滴答
	p->signal = 0; // 信号向量表 sigaction 和阻塞位 blocked 都是拷贝过来的，但是信号位图 signal 清零表示目前子进程还没有接收到任何信号
	p->alarm = 0;
	p->leader = 0;		/* process leadership doesn't inherit */
	p->utime = p->stime = 0;
	p->cutime = p->cstime = 0;
	p->start_time = jiffies;
	// 整个 TSS 都需要修改。如果不修改，前面的 *p = *current 只能保证子进程的 TSS 和父进程（被创建时）的 TSS 相同，但是随着父进程的执行，物理寄存器肯定发生了变化
	p->tss.back_link = 0;
    // PAGE_SIZE 为 4KB，因此 tss.esp0 在该任务的 task_struct 所在页的页顶位置。下面两句话创建了内核栈
	// 因此，内核栈的最大长度为 4KB - sizeof(task_struct) ≈ 3KB。这和用户栈所在的 64MB（linux0.11 规定每个普通任务的虚拟空间为 64MB)差距很大
	p->tss.esp0 = PAGE_SIZE + (long) p;
	p->tss.ss0 = 0x10; // ss0 段选择符指向内核数据段描述符（看来所有任务的内核栈的段选择子都指向内核数据段描述符，包括任务 0）

	// 这里的 eip 是父进程在进入中断前 CPU 自动压栈的，即它指向用户态的代码！它具体指向着 fork API 中 int 0x80 下一条的用户态代码
	// 因此，当子进程第一次被调度时，它是从用户态的 fork API 开始执行的（但是如果之后再被调度，就从内核态的 switch_to 中开始执行了）
	p->tss.eip = eip;
	p->tss.eflags = eflags;
	p->tss.eax = 0; // 注意子进程的 eax（即返回值）为 0，因此如果 fork 的返回值为 0 则为子进程。用户程序就靠它区分父子进程
	p->tss.ecx = ecx;
	p->tss.edx = edx;
	p->tss.ebx = ebx;
	p->tss.esp = esp; // 用户栈用的就是父进程的（执行 int 0x80 时候父进程用的）用户栈；但是当然内核栈是不一样的
	p->tss.ebp = ebp;
	p->tss.esi = esi;
	p->tss.edi = edi;
	p->tss.es = es & 0xffff; // 为什么要进行与运算呢？因为入参 es 是 32 位，而段寄存器是 16 位？
	p->tss.cs = cs & 0xffff;
	p->tss.ss = ss & 0xffff;
	p->tss.ds = ds & 0xffff;
	p->tss.fs = fs & 0xffff;
	p->tss.gs = gs & 0xffff;
    // 设置子进程独有的 LDT 段选择符，保存在该任务的 TSS 段中；当 CPU 执行任务切换时
    // 会自动从当前任务的 TSS 段（TR 寄存器存储其选择符）中的 LDT 选择符加载到 LDTR 寄存器
    // 当然还包括一系列的其他寄存器，特别是 cs, eip
	p->tss.ldt = _LDT(nr);
	p->tss.trace_bitmap = 0x80000000;
	if (last_task_used_math == current) // 如果使用数学协处理器则保存上下文
		__asm__("clts ; fnsave %0"::"m" (p->tss.i387));
    // 复制进程页表。即线性地址空间中设置该任务的代码段和数据段的描述符中的段基址和段限长，并复制页表
	if (copy_mem(nr,p)) {
		task[nr] = NULL; // 出错则释放资源
		free_page((long) p);
		return -EAGAIN;
	}
	for (i=0; i<NR_OPEN;i++) // 如果父进程有文件是打开的，则相应的文件打开次数加 1，因为子进程会共享它们
		if ((f=p->filp[i]))
			f->f_count++;
	if (current->pwd)
		current->pwd->i_count++;
	if (current->root)
		current->root->i_count++;
	if (current->executable)
		current->executable->i_count++;

    // 在 GDT 表中设置该任务的 TSS 段描述符，gdt+(nr<<1)+FIRST_TSS_ENTRY 为 TSS 段描述符的起始地址，&(p->tss) 为 TSS 段的段基址
	set_tss_desc(gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
    // 在 GDT 表中设置该任务的 LDT 段描述符，gdt+(nr<<1)+FIRST_TSS_ENTRY 为 LDT 段描述符的起始地址，&(p->ldt) 为 LDT 段的段基址（注意 LDT 本身也是一个段）
	set_ldt_desc(gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
	p->state = TASK_RUNNING;	/* do this last, just in case */
	return last_pid; // 父进程的返回值为：子进程的 pid
}

 // 给新进程分配 pid 值并返回存放 PCB 的进程标号
 // pid 分配逻辑是，在原本数值递增直到找到未被使用过的
int find_empty_process(void)
{
	int i;

	repeat: // repeat 的目的是做什么呢？应该就是去找一个独一无二的 pid 给新的进程吧
		if ((++last_pid)<0) last_pid=1; // last_pid 是最后一个已分配的 pid，它什么时候会小于零呢？应该是防止溢出的
		for(i=0 ; i<NR_TASKS ; i++) // NR_TASKS 宏定义为 64
			if (task[i] && task[i]->pid == last_pid) goto repeat;
	for(i=1 ; i<NR_TASKS ; i++)
		if (!task[i]) // 从前往后找，直到找到一个空进程
			return i; // C 函数的简单类型的返回值会存放在 eax 寄存器中
	return -EAGAIN;
}
