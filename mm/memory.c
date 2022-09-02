/*
 *  linux/mm/memory.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * demand-loading started 01.12.91 - seems it is high on the list of
 * things wanted, and it should be easy to implement. - Linus
 */

/*
 * Ok, demand-loading was easy, shared pages a little bit tricker. Shared
 * pages started 02.12.91, seems to work. - Linus.
 *
 * Tested sharing by executing about 30 /bin/sh: under the old kernel it
 * would have taken more than the 6M I have free, but it worked well as
 * far as I could see.
 *
 * Also corrected some "invalidate()"s - I wasn't doing enough of them.
 */

#include <signal.h>

#include <asm/system.h>

#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>

volatile void do_exit(long code);

static inline volatile void oom(void)
{
	printk("out of memory\n\r");
	do_exit(SIGSEGV);
}

#define invalidate() \
__asm__("movl %%eax,%%cr3"::"a" (0))

/* these are not to be changed without changing head.s etc */
#define LOW_MEM 0x100000
#define PAGING_MEMORY (15*1024*1024) // 参与分页的内存为 15MB
#define PAGING_PAGES (PAGING_MEMORY>>12) // 除以 4096，得到分页数
#define MAP_NR(addr) (((addr)-LOW_MEM)>>12) // 物理地址映射为页号
#define USED 100

#define CODE_SPACE(addr) ((((addr)+4095)&~4095) < \
current->start_code + current->end_code)

static long HIGH_MEMORY = 0; // 全局变量，存放实际物理内存的最高端地址

#define copy_page(from,to) \
__asm__("cld ; rep ; movsl"::"S" (from),"D" (to),"c" (1024))

// 内存映射位图，一个字节映射一物理页，值为对应物理页的占用次数（可能会出现同一个进程占用了两次相同的物理页吧？）
static unsigned char mem_map [ PAGING_PAGES ] = {0,};

/*
 * Get physical address of first (actually last :-) free page, and mark it
 * used. If no free pages left, return 0.
 * 反方向寻找 mem_map[0] ~ mem_map[PAGING_PAGES - 1] 中的空闲项，即 mem_map[i] == 0 的项。
 * 如果找到，就返回物理地址，找不到返回0
 */
unsigned long get_free_page(void)
{
register unsigned long __res asm("ax");

__asm__("std ; repne ; scasb\n\t"
	"jne 1f\n\t"
	"movb $1,1(%%edi)\n\t"
	"sall $12,%%ecx\n\t"
	"addl %2,%%ecx\n\t"
	"movl %%ecx,%%edx\n\t"
	"movl $1024,%%ecx\n\t"
	"leal 4092(%%edx),%%edi\n\t"
	"rep ; stosl\n\t"
	"movl %%edx,%%eax\n"
	"1:"
	:"=a" (__res)
	:"0" (0),"i" (LOW_MEM),"c" (PAGING_PAGES),
	"D" (mem_map+PAGING_PAGES-1)
	);
return __res;
}

/*
 * Free a page of memory at physical address 'addr'. Used by
 * 'free_page_tables()'
 */
void free_page(unsigned long addr)
{
	if (addr < LOW_MEM) return;
	if (addr >= HIGH_MEMORY)
		panic("trying to free nonexistent page");
	addr -= LOW_MEM;
	addr >>= 12; // 换算为内存映射数组 mem_map 数组的下标
	if (mem_map[addr]--) return;
	mem_map[addr]=0;
	panic("trying to free free page");
}

/*
 * This function frees a continuos block of page tables, as needed
 * by 'exit()'. As does copy_page_tables(), this handles only 4Mb blocks.
 */
// 进程退出时释放物理内存，至少释放一个页表映射的 4M 空间，故起始地址对齐 4MB
int free_page_tables(unsigned long from,unsigned long size)
{
	unsigned long *pg_table;
	unsigned long * dir, nr;

	if (from & 0x3fffff)
		panic("free_page_tables called with wrong alignment");
	if (!from) // 释放的线性空间不能是低 4MB 的空间，应该改成 16MB ?
		panic("Trying to free up swapper memory space");
	size = (size + 0x3fffff) >> 22; // 要释放的空间占用的页表数
	dir = (unsigned long *) ((from>>20) & 0xffc); /* _pg_dir = 0 */ // 第一个要释放的页表对应的页目录项（的物理地址）
	for ( ; size-->0 ; dir++) { // 遍历页表
		if (!(1 & *dir)) // 该页目录项是否有效
			continue;
		pg_table = (unsigned long *) (0xfffff000 & *dir); // 页表起始地址
		for (nr=0 ; nr<1024 ; nr++) { // 遍历页表项
			if (1 & *pg_table) // 如果该页表项有效，则释放对应页
				free_page(0xfffff000 & *pg_table); // 释放页表项对应页
			*pg_table = 0; // 页表项清零，空闲
			pg_table++;
		}
		free_page(0xfffff000 & *dir); // 释放页目录项对应页
		*dir = 0;
	}
	invalidate(); // 刷新页变换高速缓冲
	return 0;
}

/*
 *  Well, here is one of the most complicated functions in mm. It
 * copies a range of linerar addresses by copying only the pages.
 * Let's hope this is bug-free, 'cause this one I don't want to debug :-)
 *
 * Note! We don't copy just any chunks of memory - addresses have to
 * be divisible by 4Mb (one page-directory entry), as this makes the
 * function easier. It's used only by fork anyway.
 *
 * NOTE 2!! When from==0 we are copying kernel space for the first
 * fork(). Then we DONT want to copy a full page-directory entry, as
 * that would lead to some serious memory waste - we just copy the
 * first 160 pages - 640kB. Even that is more than we need, but it
 * doesn't take any more memory - we don't copy-on-write in the low
 * 1 Mb-range, so the pages can be shared with the kernel. Thus the
 * special case for nr=xxxx.
 */
// 内存管理最复杂的函数之一，作用是设置子进程的页目录项和页表，让它那段虚拟地址区域也能映射到父进程的物理内存中
int copy_page_tables(unsigned long from,unsigned long to,long size)
{
	unsigned long * from_page_table;
	unsigned long * to_page_table;
	unsigned long this_page;
	unsigned long * from_dir, * to_dir;
	unsigned long nr;

    // 0x3fffff = 4M - 1
	if ((from&0x3fffff) || (to&0x3fffff)) // 判断父子进程的段基址是否都在 4MB 的边界位置上
		panic("copy_page_tables called with wrong alignment");
    // from 是父进程的段基址，是 32 位虚拟地址，右移 22 位是页目录号
    // 一个页目录项为 4 字节，所以在乘以 4，即右移 20 位 且将最后两位清零
    // 因此 from_dir 是父进程的页目录表项的基地址（因为页目录表放在物理地址 0 处）
    // 每个页表项对应 4KB 内存，每个页目录项对应 4MB 内存。每个进程用 64MB 虚拟内存，所以最多用 16 个页目录项？
    // 只有一个页目录表，最多有 1024 个页目录项，对应最多有 1024 个页表
	from_dir = (unsigned long *) ((from>>20) & 0xffc); /* _pg_dir = 0 */
	to_dir = (unsigned long *) ((to>>20) & 0xffc);
	size = ((unsigned) (size+0x3fffff)) >> 22; // 需要复制的页表数，即页目录项数，这里一般是 16
    // 子进程对每个页目录项都申请一物理页保存页表，由此也可以看出，一个进程占用的页目录项是连续的
	for( ; size-->0 ; from_dir++,to_dir++) {
		if (1 & *to_dir) // 如果目的页表已存在（P = 1），则出错死机
			panic("copy_page_tables: already exist");
		if (!(1 & *from_dir)) // 如果源页表不存在（P = 0），则跳过
			continue;
		from_page_table = (unsigned long *) (0xfffff000 & *from_dir); // 获得父进程该页目录项对应的页表地址（每个页目录项 32 位，其中高 20 位是页表物理地址）
		if (!(to_page_table = (unsigned long *) get_free_page())) // 为子进程的页表申请一页，若成功页表地址返回给 to_page_table
			return -1;	/* Out of memory, see freeing */
        // 注意，对于子进程的页目录项，它并不是拷贝父进程的，而是要和其页表关联起来
		*to_dir = ((unsigned long) to_page_table) | 7; // 给刚申请的页表设置页目录项，且右三位为 1（用户级，可读写，页存在）
		nr = (from==0)?0xA0:1024; // 计算需要拷贝的页表项的数目，如果是父进程是 0 号进程，则只拷贝前 160 项，否则 1024 项
        // 将父进程的页表内容拷贝给子进程的页表
		for ( ; nr-- > 0 ; from_page_table++,to_page_table++) {
			this_page = *from_page_table;
			if (!(1 & this_page)) //最后一位为 0，该页面未使用，跳过
				continue;
			this_page &= ~2; // 2 是 010，~2 是 101，代表用户级，只读，页存在。写时复制
			*to_page_table = this_page;
			if (this_page > LOW_MEM) { // 1MB 以内的低地址不参与用户的分页管理
				*from_page_table = this_page; // 令源页也只读
				this_page -= LOW_MEM;
				this_page >>= 12;
				mem_map[this_page]++; // 该页已占用
			}
		}
	}
	invalidate();
	return 0;
}

/*
 * This function puts a page in memory at the wanted address.
 * It returns the physical address of the page gotten, 0 if
 * out of memory (either when trying to access page-table or
 * page.)
 */
// page 是物理页的物理地址（必然是 4KB 对齐），address 是线性地址。本函数的功能是建立 address 和 page 的映射关系，即设置页目录表和页表
unsigned long put_page(unsigned long page,unsigned long address)
{
	unsigned long tmp, *page_table;

/* NOTE !!! This uses the fact that _pg_dir=0 */

	if (page < LOW_MEM || page >= HIGH_MEMORY)
		printk("Trying to put page %p at %p\n",page,address);
	if (mem_map[(page-LOW_MEM)>>12] != 1)
		printk("mem_map disagrees with %p at %p\n",page,address);
    // address 对应的页目录项（的物理地址），从而 *page_table 是页目录项
	page_table = (unsigned long *) ((address>>20) & 0xffc);
	if ((*page_table)&1) // 如果页目录项的最后一位为 1，则代表该页存在，即页表在内存中
		page_table = (unsigned long *) (0xfffff000 & *page_table); // page_table 此时变成了页表的起始地址
	else { // 如果该页不存在，则新分配一个物理页给它
		if (!(tmp=get_free_page()))
			return 0;
		*page_table = tmp|7; // 设置页目录项对应的页表地址，且右三位为 1（用户级，可读写，页存在）
		page_table = (unsigned long *) tmp; // page_table 此时变成了页表的起始地址
	}
    // 最后设置线性地址对应的页表，(address>>12) & 0x3ff 的作用是拿出 address 中间的十位，即页表项在页表中的偏移
	page_table[(address>>12) & 0x3ff] = page | 7;
/* no need for invalidate */
	return page;
}

// 写时复制，创建进程时，子进程和父进程被设置成共享数据段和代码段的物理页，且页全部设置为只读
// 当父/子进程要写数据时，会触发页面写保护异常，经由 do_wp_page 会调用到本函数
// 入参 table_entry 为物理页对应的页表项指针
void un_wp_page(unsigned long * table_entry)
{
	unsigned long old_page,new_page;

	old_page = 0xfffff000 & *table_entry; // 取页表项对应页的物理地址
	if (old_page >= LOW_MEM && mem_map[MAP_NR(old_page)]==1) { // 如果位于主内存区且引用次数为 1（表示该页不被其它进程共享）
		*table_entry |= 2; // 置位 R/W 位，无需申请新页
		invalidate();
		return;
	}
	if (!(new_page=get_free_page())) // 貌似没有内存换出，直接就报错了？？
		oom();
	if (old_page >= LOW_MEM)
		mem_map[MAP_NR(old_page)]--;
	*table_entry = new_page | 7;
	invalidate();
	copy_page(old_page,new_page);
}	

/*
 * This routine handles present pages, when users try to write
 * to a shared page. It is done by copying the page to a new address
 * and decrementing the shared-page counter for the old page.
 *
 * If it's in code space we exit with a segment error.
 */
// 写时复制。如果进程对一个只读的页面进行写操作，会进入下面的页面写保护错误处理函数
void do_wp_page(unsigned long error_code,unsigned long address)
{
#if 0
/* we cannot do this yet: the estdio library writes to code space */
/* stupid, stupid. I really want the libc.a from GNU */
	if (CODE_SPACE(address)) // 一般对代码段的写操作非法，肯定是进程代码有问题，但是 estdio 库支持，所以预留了操作
		do_exit(SIGSEGV);
#endif
	un_wp_page((unsigned long *)
		(((address>>10) & 0xffc) + (0xfffff000 &
		*((unsigned long *) ((address>>20) &0xffc))))); // 入参为指向出错页的页表项指针：页表偏移量 + 页表起始地址

}

void write_verify(unsigned long address)
{
	unsigned long page;

	if (!( (page = *((unsigned long *) ((address>>20) & 0xffc)) )&1))
		return;
	page &= 0xfffff000;
	page += ((address>>10) & 0xffc);
	if ((3 & *(unsigned long *) page) == 1)  /* non-writeable, present */
		un_wp_page((unsigned long *) page);
	return;
}

void get_empty_page(unsigned long address)
{
	unsigned long tmp;

	if (!(tmp=get_free_page()) || !put_page(tmp,address)) {
		free_page(tmp);		/* 0 is ok - ignored */
		oom();
	}
}

/*
 * try_to_share() checks the page at address "address" in the task "p",
 * to see if it exists, and if it is clean. If so, share it with the current
 * task.
 *
 * NOTE! This assumes we have checked that p != current, and that they
 * share the same executable.
 */
static int try_to_share(unsigned long address, struct task_struct * p)
{
	unsigned long from;
	unsigned long to;
	unsigned long from_page;
	unsigned long to_page;
	unsigned long phys_addr;

	from_page = to_page = ((address>>20) & 0xffc);
	from_page += ((p->start_code>>20) & 0xffc);
	to_page += ((current->start_code>>20) & 0xffc);
/* is there a page-directory at from? */
	from = *(unsigned long *) from_page;
	if (!(from & 1))
		return 0;
	from &= 0xfffff000;
	from_page = from + ((address>>10) & 0xffc);
	phys_addr = *(unsigned long *) from_page;
/* is the page clean and present? */
	if ((phys_addr & 0x41) != 0x01)
		return 0;
	phys_addr &= 0xfffff000;
	if (phys_addr >= HIGH_MEMORY || phys_addr < LOW_MEM)
		return 0;
	to = *(unsigned long *) to_page;
	if (!(to & 1)) {
		if ((to = get_free_page()))
			*(unsigned long *) to_page = to | 7;
		else
			oom();
	}
	to &= 0xfffff000;
	to_page = to + ((address>>10) & 0xffc);
	if (1 & *(unsigned long *) to_page)
		panic("try_to_share: to_page already exists");
/* share them: write-protect */
	*(unsigned long *) from_page &= ~2;
	*(unsigned long *) to_page = *(unsigned long *) from_page;
	invalidate();
	phys_addr -= LOW_MEM;
	phys_addr >>= 12;
	mem_map[phys_addr]++;
	return 1;
}

/*
 * share_page() tries to find a process that could share a page with
 * the current one. Address is the address of the wanted page relative
 * to the current data space.
 *
 * We first check if it is at all feasible by checking executable->i_count.
 * It should be >1 if there are other tasks sharing this inode.
 */
// 尝试找到其它进程，与它共享物理内存。因为子进程，孙进程等等对应的可执行文件都是一样的（如果没有执行新的可执行文件）
static int share_page(unsigned long address)
{
	struct task_struct ** p;

	if (!current->executable) // 当前进程是否有相应的可执行文件（进程刚创建出来时是没有的）
		return 0;
	if (current->executable->i_count < 2) // 可执行文件有没有被多个进程使用
		return 0;
	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!*p)
			continue;
		if (current == *p)
			continue;
		if ((*p)->executable != current->executable)
			continue;
		if (try_to_share(address,*p)) // 尝试与进程 p 共享物理页
			return 1;
	}
	return 0;
}

// 缺页中断（内存换入）时会调用的函数，address 为产生缺页异常的线性地址
void do_no_page(unsigned long error_code,unsigned long address)
{
	int nr[4];
	unsigned long tmp;
	unsigned long page;
	int block,i;

	address &= 0xfffff000;
	tmp = address - current->start_code; // 逻辑地址
	if (!current->executable || tmp >= current->end_data) {
		get_empty_page(address);
		return;
	}
	if (share_page(tmp))
		return;
	if (!(page = get_free_page()))
		oom();
/* remember that 1 block is used for header */
	block = 1 + tmp/BLOCK_SIZE;
	for (i=0 ; i<4 ; block++,i++)
		nr[i] = bmap(current->executable,block);
	bread_page(page,current->executable->i_dev,nr);
	i = tmp + 4096 - current->end_data;
	tmp = page + 4096;
	while (i-- > 0) {
		tmp--;
		*(char *)tmp = 0;
	}
	if (put_page(page,address))
		return;
	free_page(page);
	oom();
}

// start_mem 是主内存区起始地址，end_mem 是物理内存总大小（字节数），典型值 16MB
void mem_init(long start_mem, long end_mem)
{
	int i;

	HIGH_MEMORY = end_mem;
	for (i=0 ; i<PAGING_PAGES ; i++)
		mem_map[i] = USED; // 一开始给了一个很大的值，认为所有物理页都用完了
	i = MAP_NR(start_mem); // 主内存区起始地址对应的物理页号
	end_mem -= start_mem; // 主内存区的总大小
	end_mem >>= 12; // 主内存区对应的物理页数
	while (end_mem-->0)
		mem_map[i++]=0; // 将主内存区页面映射数组清零，但是高速缓冲区和虚拟盘（如果有）的 mem_map 一直是占用状态。另外还有不能用的是一开始减去的不参与分页的空间 LOW_MEM
}

void calc_mem(void)
{
	int i,j,k,free=0;
	long * pg_tbl;

	for(i=0 ; i<PAGING_PAGES ; i++)
		if (!mem_map[i]) free++;
	printk("%d pages free (of %d)\n\r",free,PAGING_PAGES);
	for(i=2 ; i<1024 ; i++) {
		if (1&pg_dir[i]) {
			pg_tbl=(long *) (0xfffff000 & pg_dir[i]);
			for(j=k=0 ; j<1024 ; j++)
				if (pg_tbl[j]&1)
					k++;
			printk("Pg-dir[%d] uses %d pages\n",i,k);
		}
	}
}
