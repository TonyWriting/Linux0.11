/*
 *  linux/kernel/exit.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <asm/segment.h>

int sys_pause(void);
int sys_close(int fd);

void release(struct task_struct * p)
{
	int i;

	if (!p)
		return;
	for (i=1 ; i<NR_TASKS ; i++)
		if (task[i]==p) {
			task[i]=NULL;
			free_page((long)p);
			schedule();
			return;
		}
	panic("trying to release non-existent task");
}

static inline int send_sig(long sig,struct task_struct * p,int priv)
{
	if (!p || sig<1 || sig>32)
		return -EINVAL;
	if (priv || (current->euid==p->euid) || suser())
		p->signal |= (1<<(sig-1));
	else
		return -EPERM;
	return 0;
}

static void kill_session(void)
{
	struct task_struct **p = NR_TASKS + task;
	
	while (--p > &FIRST_TASK) {
		if (*p && (*p)->session == current->session)
			(*p)->signal |= 1<<(SIGHUP-1);
	}
}

/*
 * XXX need to check permissions needed to send signals to process
 * groups, etc. etc.  kill() permissions semantics are tricky!
 */
int sys_kill(int pid,int sig)
{
	struct task_struct **p = NR_TASKS + task;
	int err, retval = 0;

	if (!pid) while (--p > &FIRST_TASK) {
		if (*p && (*p)->pgrp == current->pid) 
			if ((err=send_sig(sig,*p,1)))
				retval = err;
	} else if (pid>0) while (--p > &FIRST_TASK) {
		if (*p && (*p)->pid == pid) 
			if ((err=send_sig(sig,*p,0)))
				retval = err;
	} else if (pid == -1) while (--p > &FIRST_TASK) {
		if ((err = send_sig(sig,*p,0)))
			retval = err;
	} else while (--p > &FIRST_TASK)
		if (*p && (*p)->pgrp == -pid)
			if ((err = send_sig(sig,*p,0)))
				retval = err;
	return retval;
}

static void tell_father(int pid)
{
	int i;

	if (pid)
		for (i=0;i<NR_TASKS;i++) {
			if (!task[i])
				continue;
			if (task[i]->pid != pid)
				continue;
			task[i]->signal |= (1<<(SIGCHLD-1));
			return;
		}
/* if we don't find any fathers, we just release ourselves */
/* This is not really OK. Must change it to make father 1 */
	printk("BAD BAD - no father found\n\r");
	release(current);
}

int do_exit(long code)
{
	int i;
	// 首先释放当前进程代码段和数据段所占的内存页
    // 当前任务的 task_struct 所在内存页不在该函数中释放，而是通过父进程调用 wait()，最终在 release() 中释放
	free_page_tables(get_base(current->ldt[1]),get_limit(0x0f));
	free_page_tables(get_base(current->ldt[2]),get_limit(0x17));
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i] && task[i]->father == current->pid) {
			task[i]->father = 1; // 如果当前进程有子进程，就将子进程的 father 变为 init 进程
			if (task[i]->state == TASK_ZOMBIE)
				/* assumption task[1] is always init */
				// 如果该子进程处于僵死 TASK_ZOMBIE 状态，则向 init 进程发送子进程终止信号 SIGCHLD
				(void) send_sig(SIGCHLD, task[1], 1);
		}
	// 关闭当前进程打开的全部文件
	for (i=0 ; i<NR_OPEN ; i++)
		if (current->filp[i])
			sys_close(i);
	// 对当前进程的当前工作目录，根目录和运行程序的 i 节点进行同步操作，放回各个 i 节点并置空
	iput(current->pwd);
	current->pwd=NULL;
	iput(current->root);
	current->root=NULL;
	iput(current->executable);
	current->executable=NULL;
	if (current->leader && current->tty >= 0) // 如果进程是一个会话头进程并有控制终端，则释放该终端
		tty_table[current->tty].pgrp = 0;
	if (last_task_used_math == current)
		last_task_used_math = NULL;
	if (current->leader) // 如果进程是一个 leader 进程，则终止该会话的所有相关进程
		kill_session();
	current->state = TASK_ZOMBIE; // 当前进程 current 从运行态（R）切换成退出（E）
#if VERBOSE
	fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", current->pid, 'E', jiffies, "do_exit", current->father);
#else
    fprintk(3, "%ld\t%c\t%ld\n", current->pid, 'E', jiffies);
#endif
	current->exit_code = code;
	tell_father(current->father); // 通知父进程，即向父进程发送信号 SIGCHLD 告知当前进程将终止
	schedule();
	return (-1);	/* just to suppress warnings */
}

int sys_exit(int error_code)
{
	return do_exit((error_code&0xff)<<8);
}

// 挂起当前进程，直到 pid 执行的子进程退出（终止）或者收到终止该进程的信号
// 如果 pid 所指的子进程已经僵死（TASK_ZOMBIE），则本调用立即返回
// 详细分析见《Linux 内核完全注释》
int sys_waitpid(pid_t pid,unsigned long * stat_addr, int options)
{
	int flag, code;
	struct task_struct ** p;

	verify_area(stat_addr,4);
repeat:
	flag=0;
	for(p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!*p || *p == current)
			continue;
		if ((*p)->father != current->pid)
			continue;
		if (pid>0) {
			if ((*p)->pid != pid)
				continue;
		} else if (!pid) {
			if ((*p)->pgrp != current->pgrp)
				continue;
		} else if (pid != -1) {
			if ((*p)->pgrp != -pid)
				continue;
		}
        // 到达这里时必有找到了要等待的 pid（pid > 0） 或者 pid == -1（代表等待任意一个子进程）
		switch ((*p)->state) {
			case TASK_STOPPED:
				if (!(options & WUNTRACED))
					continue;
				put_fs_long(0x7f,stat_addr);
				return (*p)->pid;
			case TASK_ZOMBIE:
				current->cutime += (*p)->utime;
				current->cstime += (*p)->stime;
				flag = (*p)->pid;
				code = (*p)->exit_code;
				release(*p);
				// 将退出码 exit_code 保存到 FS 段指定的 stat_addr 地址处
				// 当用户程序通过系统调用开始执行内核代码时，内核代码会在段寄存器 DS,ES 加载 GDT 的内核数据段描述符（段选择符为 0x10），即 DS,ES 用于访问内核数据段
				// FS 则加载 LDT 的数据段描述符（段选择符为 0x17），即将 FS 用于访问用户数据段
				// stat_addr 是用户态程序传递过来的整形变量地址
				put_fs_long(code,stat_addr);
				return flag;
			default:
				flag=1;
				continue;
		}
	}
	if (flag) {
		if (options & WNOHANG)
			return 0;
		current->state=TASK_INTERRUPTIBLE; // 当前进程从就绪态（J）变成阻塞态（W）
#if VERBOSE
		fprintk(3, "%ld\t%c\t%ld\t%s\t%ld\n", current->pid, 'W', jiffies, "sys_waitpid", current->father);
#else
        fprintk(3, "%ld\t%c\t%ld\n", current->pid, 'W', jiffies);
#endif
		schedule();
		// 重新调度本任务后，如果没有收到除了 SIGCHLD 以外的信号，还是重复处理
		if (!(current->signal &= ~(1<<(SIGCHLD-1))))
			goto repeat;
		else
			return -EINTR;
	}
	return -ECHILD;
}


