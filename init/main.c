/*
 *  linux/init/main.c
 *
 *  (C) 1991  Linus Torvalds
 */

#define __LIBRARY__
#include <unistd.h> // .h 头文件的默认目录为 include/
#include <time.h>

/*
 * we need this inline - forking from kernel space will result
 * in NO COPY ON WRITE (!!!), until an execve is executed. This
 * is no problem, but for the stack. This is handled by not letting
 * main() use the stack at all after fork(). Thus, no function
 * calls - which means inline code for fork too, as otherwise we
 * would use the stack upon exit from 'fork()'.
 *
 * Actually only pause and fork are needed inline, so that there
 * won't be any messing with the stack from main(), but we define
 * some others too.
 */
 // inline 有两个作用：（1）减少函数调用开销 （2）在创建任务 1 之前保持任务 0 的用户栈干净，因为任务 1 在内核空间执行，没有写时复制
static inline _syscall0(int,fork)
static inline _syscall0(int,pause)
static inline _syscall1(int,setup,void *,BIOS)
static inline _syscall0(int,sync)

#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <asm/system.h>
#include <asm/io.h>

#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <linux/fs.h>

static char printbuf[1024];

extern int vsprintf();
extern void init(void);
extern void blk_dev_init(void);
extern void chr_dev_init(void);
extern void hd_init(void);
extern void floppy_init(void);
extern void mem_init(long start, long end);
extern long rd_init(long mem_start, int length);
extern long kernel_mktime(struct tm * tm);
extern long startup_time;

/*
 * This is set up by the setup-routine at boot-time
 */
#define EXT_MEM_K (*(unsigned short *)0x90002)
#define DRIVE_INFO (*(struct drive_info *)0x90080)
#define ORIG_ROOT_DEV (*(unsigned short *)0x901FC)

/*
 * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
 * and this seems to work. I anybody has more info on the real-time
 * clock I'd be interested. Most of this was trial and error, and some
 * bios-listing reading. Urghh.
 */

#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static void time_init(void)
{
	struct tm time;

	do {
		time.tm_sec = CMOS_READ(0);
		time.tm_min = CMOS_READ(2);
		time.tm_hour = CMOS_READ(4);
		time.tm_mday = CMOS_READ(7);
		time.tm_mon = CMOS_READ(8);
		time.tm_year = CMOS_READ(9);
	} while (time.tm_sec != CMOS_READ(0));
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	time.tm_mon--;
	startup_time = kernel_mktime(&time);
}

static long memory_end = 0; // 物理内存总字节数
static long buffer_memory_end = 0; // 高速缓存末端地址
static long main_memory_start = 0; // 主内存开始地址

struct drive_info { char dummy[32]; } drive_info;

// head.s 的 iret 使得 CPU 的执行跳转到这里
void main(void)		/* This really IS void, no error here. */
{			/* The startup routine assumes (well, ...) this */
/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
 	ROOT_DEV = ORIG_ROOT_DEV;
 	drive_info = DRIVE_INFO;
	memory_end = (1<<20) + (EXT_MEM_K<<10); // 内存大小为 1MB + 扩展内存（固定的？）
	memory_end &= 0xfffff000; // 对 4KB 向下取整
	if (memory_end > 16*1024*1024)
		memory_end = 16*1024*1024; // 看来 Linux0.11 的物理内存的典型值是 16Mb
	if (memory_end > 12*1024*1024) 
		buffer_memory_end = 4*1024*1024;
	else if (memory_end > 6*1024*1024)
		buffer_memory_end = 2*1024*1024;
	else
		buffer_memory_end = 1*1024*1024;
	main_memory_start = buffer_memory_end; // 主内存开始地址等于高速缓冲区末端
#ifdef RAMDISK // 如果定义了虚拟盘，主内存区就从虚拟盘末端地址开始，否则从高速缓冲区末端开始
	main_memory_start += rd_init(main_memory_start, RAMDISK*1024);
#endif
	mem_init(main_memory_start,memory_end);
	trap_init(); // 中断向量初始化
	blk_dev_init(); // 块设备初始化
	chr_dev_init(); // 字符设备初始化（保留接口，未实现）
	tty_init(); // 终端初始化
	time_init(); // 计时初始化
	sched_init(); // 调度程序初始化
	buffer_init(buffer_memory_end); // 缓冲区初始化
	hd_init(); // 硬盘初始化
	floppy_init(); // 软盘初始化
	sti(); // 开中断（在此之前中断一直是关闭的）
	move_to_user_mode();  // 切换到用户模式，之后就是用户态的任务 0 了。它访问内核也要经过系统调用
	if (!fork()) {		/* we count on this going ok */ // 用户态的进程 0 立刻 fork 出 1 号进程（init 进程）
		init(); /* 如果是进程 1， fork() 返回 0，故会调用 init() */
	}
/*
 *   NOTE!!   For any other task 'pause()' would mean we have to get a
 * signal to awaken, but task0 is the sole exception (see 'schedule()')
 * as task 0 gets activated at every idle moment (when no other tasks
 * can run). For task0 'pause()' just means we go check if some other
 * task can run, and if not we return here.
 */
 // 0 号进程一直死循环, pause 是个系统调用的 API（因为任务 0 是用户态进程，访问内核需要系统调用）
 // pause API 在 main.c 中的 static inline _syscall0(int,pause) 宏生成
	for(;;) pause();
}

static int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

static char * argv_rc[] = { "/bin/sh", NULL };
static char * envp_rc[] = { "HOME=/", NULL };

static char * argv[] = { "-/bin/sh",NULL };
static char * envp[] = { "HOME=/usr/root", NULL };

void init(void)
{
	int pid,i;

	setup((void *) &drive_info); // setup 为系统调用，读取磁盘参数，加载虚拟盘（如果有的话）和安装根文件系统
	// 以读写方式打开终端设备 /dev/tty0（对应终端控制台）。第一次打开文件会产生文件描述符（也称句柄号）0 号（stdin）。并将其与 /dev/tty0 关联
	// (void) 表示强制函数无需返回值
	(void) open("/dev/tty0",O_RDWR,0);
	(void) dup(0); // 复制文件描述符 0，产生文件描述符 1 号（stdout），它也和 /dev/tty0 关联
	(void) dup(0); // 复制文件描述符 0，产生文件描述符 2 号（stderr），它也和 /dev/tty0 关联

	// 利用上述描述符在终端显示系统信息，即打印缓冲区块数，总字节数和主内存区空闲内存字节数
	printf("%d buffers = %d bytes buffer space\n\r",NR_BUFFERS,
		NR_BUFFERS*BLOCK_SIZE);
	printf("Free mem: %d bytes\n\r",memory_end-main_memory_start);

	if (!(pid=fork())) { // 创建进程 2
		close(0); // 关闭文件描述符 0
		if (open("/etc/rc",O_RDONLY,0)) // 打开文件 /etc/rc，并将标准输入 stdin 重定向到 /etc/rc 文件，从而会执行 /etc/rc 中的命令
			_exit(1);
		// 将自身替换 /bin/sh 程序（即 shell），从而执行 /etc/rc 文件中的命令
		// sh 的运行方式是非交互式的，因此执行完命令后立刻退出，进程 2 结束
		execve("/bin/sh",argv_rc,envp_rc);
		_exit(2); // execve 执行失败则进程 2 退出
	}
	if (pid>0)
		while (pid != wait(&i)) // 进程 0 等待进程 2 结束，i 存放返回状态信息
			/* nothing */;
	while (1) {
		if ((pid=fork())<0) {
			printf("Fork failed in init\r\n");
			continue;
		}
		if (!pid) {
			close(0);close(1);close(2); // 关闭之前遗留的全部句柄
			// 创建新的会话期
			// 会话期是一个或多个进程组的集合；进程组是一个或多个进程的集合
			// 通常情况下，用户登录后执行的所有程序都属于一个会话期，其登录 shell 是会话期的首进程，它使用的终端是会话期的控制终端
			// 在退出登录（logout）时，所有属于该会话期的进程都会被终止
			setsid();
			(void) open("/dev/tty0",O_RDWR,0); // 重新打开 /dev/tty0 作为 stdin
			(void) dup(0); // 复制产生 stdout
			(void) dup(0); // 复制产生 stderr
			_exit(execve("/bin/sh",argv,envp)); // 以登录 shell 方式再次执行程序 /bin/sh，以创建用户交互 shell 环境。用户就能正常使用 Linux 命令行环境了
		}
		while (1)
			if (pid == wait(&i)) // 等待子进程结束，如果用户在命令行执行了 exit 或 logout 命令，那么会再次进入无限循环中
				break;
		printf("\n\rchild %d died with code %04x\n\r",pid,i);
		sync(); // 同步操作，刷新缓冲区
	}
	_exit(0);	/* NOTE! _exit, not exit() */
}
