# 哈工大操作系统课程实验

## 前言

**TODO**

## 参考资料

**TODO**

## 环境搭建

如果对如何编译、运行 Linux 0.11 以及如何与 Linux 0.11 的文件系统进行文件交换有疑问，请参考实验楼的[《熟悉实验环境》](https://www.lanqiao.cn/courses/115)。

如果不想用实验楼的现成环境，想要在 Ubuntu 16.04/18.04 环境自行搭建 Linux 0.11 的实验环境，请参考 Wangzhike 仓库的[《环境准备》](https://github.com/Wangzhike/HIT-Linux-0.11/blob/master/0-prepEnv/%E5%87%86%E5%A4%87%E5%AE%89%E8%A3%85%E7%8E%AF%E5%A2%83.md)。如果要进行内核的 C 语言级别调试，除了按照上述的《环境准备》搭建环境外，之后还需要执行：

```bash
apt-get install libncurses5:i386
apt-get install libexpat1-dev:i386
```

- 强烈建议**不用** WSL/WSL2 ！

  我曾尝试用 WSL/WSL2，解决了一个个问题，最终还是发现 Windows 和 Linux 0.11 的文件系统无法/难以兼容，从而无法与 Linux 0.11 交换文件。最终转向了[阿里云](https://www.aliyun.com/activity/ambassador/share-gift/goods?taskCode=xfyh2107&recordId=757672&userCode=i5mn5r7m)，选择了其中最便宜的云服务器，算上新用户优惠券一年不到 90 元。其他公司的云（例如微软）当然也可以，价格应该也差不多。

## 代码注释

代码注释在 [Annotation 分支](https://gitee.com/na-chern/hit-os-learning/tree/Annotation/)。

## 实验一 操作系统引导

**TODO**

## 实验二 系统调用

### 实验内容

此次实验的基本内容是：在 Linux 0.11 上添加两个系统调用，并编写两个简单的应用程序测试它们。

（1）`iam()`

第一个系统调用是 `iam()`，其原型为：

```c
int iam(const char * name);
```

完成的功能是将字符串参数 `name` 的内容拷贝到内核中保存下来。要求 `name` 的长度不能超过 23 个字符。返回值是拷贝的字符数。如果 `name` 的字符个数超过了 23，则返回 “-1”，并置 `errno` 为 `EINVAL`。

在 `kernal/who.c` 中实现此系统调用。

（2）`whoami()`

第二个系统调用是 `whoami()`，其原型为：

```c
int whoami(char* name, unsigned int size);
```

它将内核中由 `iam()` 保存的名字拷贝到 `name` 指向的用户地址空间中，同时确保不会对 `name` 越界访存（`name` 的大小由 `size` 说明）。返回值是拷贝的字符数。如果 `size` 小于需要的空间，则返回“-1”，并置 `errno` 为 `EINVAL`。

也是在 `kernal/who.c` 中实现。

（3）测试程序

将 `iam.c` 和 `whoami.c`，`testlab2.c` 和 `testlab2.sh` 拷贝到 Linux 0.11 中，并在其环境下编译 `iam.c`, `whoami.c` 和 `testlab2.c`，最后执行 `testlab2.sh` 和编译后的 `testlab2` ，分数为两者之和。

上述 4 个源文件在 [test/](https://gitee.com/na-chern/hit-os-learning/tree/Experiment2_system_call/test) 目录下。

```bash
gcc -o iam iam.c
gcc -o whoami whoami.c
gcc -o testlab2 testlab2.c
./testlab2 # 满分 50 分
./testlab2.sh # 满分 30 分
```

本实验原本要求用户态程序 `iam.c`, `whoami.c`  也需要自行编写，但是我当时没理解题意，认为是给的（测试代码），因此直接拿了（仅增加了一个宏定义）[Wangzhike 仓库](https://github.com/Wangzhike/HIT-Linux-0.11)的[文件](https://github.com/Wangzhike/HIT-Linux-0.11/tree/master/2-syscall/linux-0.11)。这不涉及内核态。

### 原理分析

系统调用的本质是中断，因为调用内核段的函数不能像用户段那样简单直接的函数调用，否则会有安全问题（比如某个用户程序读取修改了 root 的密码），即使它们都处于你买的同一个内存条中。

以大家熟悉的 C 语言库函数 `printf` 为例，系统调用的路径依次为：

- `printf`
- `int 0x80`
- `system_call`
- 查表 `sys_call_table`
- `sys_write`

下面依次介绍。

（1）`printf` -> `int 0x80`

`printf` 是 C 运行库提供的 API。这是因为如果让用户程序直接调用 `int 0x80`，那么不同平台的可移植性不好（Windows 系统调用的中断向量号是 `0x2e` 而不是 `0x80`），同时也比较复杂，于是运行库充当了中间层。从而不同硬件平台都可以通过 `printf` 进行打印，对用户程序屏蔽了硬件差异。

C 语言的 API 可以通过宏展开或者手写嵌入汇编来调用 `int 0x80`。比如 `printf` 会调用宏 `_syscall3`：

```c
#define _syscall3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" ((long)(a)),"c" ((long)(b)),"d" ((long)(c))); \
if (__res>=0) \
	return (type) __res; \
errno=-__res; \
return -1; \
}
```

关于 C 语言中嵌入汇编的语法可以参考《Linux 内核完全注释》的 3.3.2 节。这里不对上面每行做具体解释，仅介绍其功能。但如果想要理解操作系统，上面每行代码的含义都应该完全掌握。

`_syscall3` 有 3 个入参，它们分别通过 EBX，ECX 和 EDX 寄存器传递进入 `int 0x80` 中。同时还会将 `__NR__##name` 通过 EAX 寄存器传入内核中。在 `printf` 中，`__NR__##name` 会拼接为 `__NR__write`。`__NR__write` 是系统调用号，代表着内核态函数 `sys_write` 在函数指针数组 `sys_call_table` 的偏移。处理完中断返回后，会通过 EAX 寄存器将结果返回到用户态变量 `__res` 中。从代码可以看出，`__res` 代表中断处理是否成功，如果它大于等于 `0` 代表成功；否则失败，此时会将它的相反数赋给全局变量 `errno` 中，最后返回 `-1`。

（2）`int 0x80` -> `system_call`

这里是系统调用的最关键之处，也是用户态和内核态发生改变的边界。在 CPU 执行 `int 0x80` 之前还是处于用户态，执行完跳转到 `system_call` 后就变成内核态了，在之后就是普通的函数调用了。

那么这一步发生了什么呢？我们知道，CPU 的工作就是取址执行，当它看到 `int 0x80` 指令，就会根据中断向量 0x80 ，在中断向量描述符表 IDT 中找到相应的门描述符，特权级检查通过后，就会跳转到相应的中断处理程序的入口地址。

注意在 32 位模式中，寻址是通过段选择子指定的段描述符中的段基址 + 段内偏移的机制（如果对此不了解请看《Linux 内核完全注释》的 4.3 节）。因此在 IDT 表中，存放的每一个表项（也称为门描述符）必须包括两个部分：（中断处理函数所在的）段选择子和段内偏移。除此之外，还会将 EFLAGS，CS 和 EIP 寄存器压栈，如果特权级发生了改变还会涉及栈切换。

该步骤的主要流程是：

- 中断向量号 -> 门描述符 -> （段选择子 + 段内偏移）
- 段选择子 -> 段描述符 -> 段基址
- （段基址 + 段内偏移 + 特权级检查通过） -> 栈切换 -> 寄存器压栈 -> 装载 CS 和 EIP 寄存器，跳转

另一个值得注意的地方是权限检查。需要同时满足两个条件才能跳转：

- 当前特权级 CPL（存放在 CS 寄存器后两位）的值小于等于 IDT 表的门描述符中的 DPL
- 当前特权级 CPL（存放在 CS 寄存器后两位）的值大于等于 GDT 表的段描述符（由门描述符中的段选择子指定）中的 DPL

由于值越小优先级越大，这会保证系统调用前后程序的特权级不会降低。

这里还有一个问题，就是 IDT 表中的门描述符是哪里设置好的呢？是在 main 函数中的初始化相关函数做的。函数调用路径为：

```c
main() -> sched_init() -> set_system_gate() -> _set_gate()
```

代码细节可以看李治军老师的 [L5 节课程](https://www.bilibili.com/video/BV1d4411v7u7?p=5)。本实验的作业不需要对初始化部分进行改动。系统调用的门描述符会被初始化为：

```
63                               48 47 46  44 43     40 39 37 36        32
+----------------------------------+--+----+--+--------+-+-+-+----------+
|                                  |  |    |  |        |     |          |
|       &system_call[31:16]        |P |DPL |S |  TYPE  |0 0 0| Reserved |
|                                  |1 | 00 |0 | 1|1|1|1|     |          |
+-------------+--+--+--+--+--------+--+----+--+--------+-+-+-+----------+
31                               17 16                                  0
+----------------------------------+------------------------------------+
|                                  |                                    |
|         Segment Selector         |           &system_call[15:0]       |
|              0x0008              |                                    |
+----------------------------------+------------------------------------+
```

可见，系统调用的 CS 段选择符指向了内核代码段描述符，段内偏移是 `system_call` 的地址。

（3）`system_call` -> `sys_call_table`

中断向量号的个数往往很有限，但是需要的中断服务处理有很多种。解决方式是，将所有的系统调用都用同一个中断向量号 `int 0x80` 来表示，然后在 `system_call` 内部，根据系统调用号 `__NR__##name` 来确定具体的函数是什么。`sys_call_table` 就是存放这些函数的函数指针的数组，`__NR__##name` 是数组偏移。

（4）查表 `sys_call_table` -> `sys_write`

`sys_call_table[__NR__write * 4] == sys_write ` 。乘以 4 是因为在 32 位模式中，函数的入口地址为 32 位，`__NR__write * 4 `是入口的地址字节。 

### 作业参考

本实验的满分作业可参考[这里](https://gitee.com/na-chern/hit-os-learning/commit/5c2e7e77118fa76cd621d360d0968adbff28405c)。

### 易错点

#### 内核态函数的异常返回值

实际上，我们实现的是内核态函数 `sys_iam` 和 `sys_whoami`。但是在用户态不能直接调用它们，因此在 `iam.c` 和 `whoami.c` 借助了宏`_syscall1` 和 `_syscall2`，生成了用户态函数 `iam()` 和 `whoami()` 被调用。在 `_syscallx` 这些宏中，它们的返回值是这样的（以 `_syscall1` 为例）：

```
#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" ((long)(a))); \
if (__res >= 0) \
	return (type) __res; \
errno = -__res; \
return -1; \
}
```

可以看出，这些宏会讲内核态函数的返回值作进一步的处理。当内核态函数的返回值大于等于零时视为正常，当小于零视为异常，并且会讲全局变量 `errno` 赋值 `-1`，并最终返回 `-1`。因此，在我们实现的内核态函数中，如果发生了异常，根据题意返回 `-(EINVAL)` 即可。

#### `0x80` 和 `__NR__##name`

这两个都是整型，并且都是在数组中的偏移。区别在于，`0x80` 是中断向量，它是中断描述符表 IDT 的偏移，代表着这个中断是系统调用 ；而 `__NR__##name` 则是 `sys_call_table` 数组的偏移，被称为系统调用号，放在 EAX 寄存器中作为入参传递给内核。`sys_call_table` 里面的元素是函数指针，由 `__NR__##name` 确定调用哪个内核态的函数。IDT 和 `sys_call_table` 都是全局变量。

## 实验四 基于内核栈切换的进程切换

进程是操作系统的核心，进程的切换与创建是进程的核心。可以说本实验涉及到了操作系统最核心的地方，值得投入大量时间精力，力求弄懂涉及到的每一行代码。

### 实验内容

原生 Linux 0.11 借助 TSS 的硬件机制（后面会有详细论述），一条指令就能完成任务切换。它虽然简单，但这条指令的执行时间却很长，在实现任务切换时大概需要 200 多个时钟周期。

通过内核栈实现任务切换的时间更短，并且它还能使用指令流水的并行优化技术进一步优化时间。所以无论是 Linux 还是 Windows，进程/线程的切换都没有使用 Intel 提供的这种 TSS 切换手段，而都是通过内核栈实现的。

本次实践项目就是将原生 Linux 0.11 中采用的 TSS 切换部分去掉，取而代之的是基于内核栈的切换程序。

### 原理分析

#### 基于 TSS 切换的进程切换

这是 Linux 0.11 原生的方式，即进程切换会将原进程的硬件上下文保存到属于本进程的 TSS 段中，然后再将新进程的 TSS 段中之前保存的硬件上下文赋值给物理寄存器，从而实现进程切换。

在该方式中，每个进程都会有自己单独的 TSS 段。TSS 是一个**段**（LDT 表也是一个段；但是 GDT 表不是，它只是一个结构体），在内核代码中用一个结构体 `tss_struct` 描述，保存在进程的 PCB，即 `task_struct` 结构体中。因此，进程的 TSS 段是其 PCB 结构体的一部分。

整个流程是什么样的呢？

（1）`int 0x80`

首先当然是从上一个进程的用户态通过中断（系统调用）进入内核。在实验二中已经描述过 `int 0x80` 会发生什么，但是现在看来还不够，缺少了对栈切换的描述。执行 `int 0x80` 指令后，堆栈会从用户栈切换到内核栈，并将此时的寄存器 SS、ESP（用户栈的状态），状态寄存器 EFLAGS 和用户态的下一条指令地址 CS、EIP 压栈。这里有几个问题：

- CPU 是怎么找到内核栈的？或者问，从内存的什么位置找到值赋给物理寄存器 SS、ESP？

  它们保存在进程的 TSS 段。想一想也是合理的，因为 TSS 就是用来保存进程的硬件上下文的，它不仅会保存进程的用户栈状态，还会保存内核栈初始状态。那是怎么找到当前进程的 TSS 段呢？这就需要引出 TR 寄存器了，它存放着指向当前进程 TSS 段的段选择子（进程切换时，会确保 TR 寄存器跟着切换）。

  串起来讲：系统调用发生时，CPU 会自动读取 TR 寄存器里的 TSS 段选择子，找到当前进程的 TSS 段描述符，TSS  描述符里面的段基址会指向当前进程的 TSS 段，最终能找到 TSS 段保存的内核栈初始值的 SS0 和 ESP0。那为什么 TSS 描述符的段基址会指向当前进程的 TSS 段呢？这是内核在 `fork` 出该进程时，在 `copy_process` 函数中通过 `set_ldt_desc` 宏做的。

- 会跳转到内核的什么位置？

  实验二已分析过，系统调用后跳转的 CS 为 `0x0008`，是内核代码段的段选择子；EIP 为 `$system_call`，是系统调用函数的入口地址。这意味着所有进程从用户态通过系统调用进入内核时，它们用着一样的、共用的内核代码段（当然内核数据段也是一样的，它在 `system_call` 中被设置为 `0x0010`），跑着相同的代码。

- 系统调用发生后，内核栈的 SS 和 ESP 初值是什么？

  SS 始终是 `0x0010`，即内核数据段的段选择子，而 ESP 也始终是固定的，为 `PAGE_SIZE + (long) p`，物理含义是，与当前进程的 PCB 在同一物理页，并且在该页的页顶（4KB）。它们源于 `fork` 该进程时的 `copy_process` 函数，截取相关片段如下：

  ```C
  int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
  		long ebx,long ecx,long edx,
  		long fs,long es,long ds,
  		long eip,long cs,long eflags,long esp,long ss) {
      	...
  		p = (struct task_struct *) get_free_page(); // 给当前 PCB 分配一个物理页（大小为 4KB）
      	...
          p->tss.esp0 = PAGE_SIZE + (long) p; // 内核栈的 ESP 初始值（PAGE_SIZE == 4096）
      	p->tss.ss0 = 0x10; // 内核栈的 SS 初始值
      	...
  		}
  ```

  上面代码可以说明，进程被 `fork` 出来后，第一次进入内核中的内核栈确实如此，但是之后再次进入内核还会是如此吗？
  
  答案是肯定的。在下面的 `switch_to` 宏中的 `ljmp *%0` 中，它只会将当前的内核栈压入 `tss.ss` 和 `tss.esp` 中，而不会改变 `tss.esp0` 和 `tss.ss0`，即它们是只读的，专门用于从低特权级转换到 0 特权级时给栈赋初值。也就是说，进程刚从用户态进入其内核态时（注意区别于从一个进程的内核态切换到另一个进程的内核态），它的内核栈总是被赋相同的初值，即总是空的。这也是合理的，进程的内核栈总大小只有 `4096 - sizeof(task_struct)` 个字节（用户栈通常为几 MB，堆则可以到几 GB），如果每发生一次系统调用，内核栈的空间会减少，那么内核栈很快就被用光了。

(2) 进入 `switch_to` 之前

在进入  `switch_to` 之前，程序流依次为：

`int 0x80` -> `system_call` -> `reschedule` -> `schedule` -> `switch_to`。

`reschedule` 只是简单地将 `&ret_from_sys_call` 压栈然后跳转到 `schedule` 函数。

`schedule` 函数中有调度算法，即找到下一个需要被调度/切换进程，然后调用 `switch_to` 宏函数。

`switch_to` 是实现进程切换的地方。它的前半部分指令在切换前进程执行，后半部分指令已经切换到下一个进程了。也就是说，当执行到 `switch_to` 的后半部分指令时，已经是下一个进程在执行了。

下表给出了在进入`switch_to` 之前的内核栈状态（为了对齐表格用了英文注释）：

```shell
+----------------------------------------------------------+
| # push by hardware                                       |
| SS                                                       |
| ESP                                                      |
| EFLAGS                                                   |
| CS                                                       |
| EIP                                                      |
+----------------------------------------------------------+
| # push in `system_call`                                  |
| DS                                                       |
| ES                                                       |
| FS                                                       |
| EDX                                                      |
| ECX                                                      |
| EBX # EDX, ECX and EBX as parameters to `system_call`    |
| EAX # return value of `sys_call_table(,%eax,4)`          |
+----------------------------------------------------------+
| # push in `reschedule`                                   |
| &ret_from_sys_call                                       |
+----------------------------------------------------------+
| # push in `schedule`                                     |
| EBP                                                      |
+----------------------------------------------------------+
```

注意，`switch_to` 是一个宏，因此`schedule`调用 `switch_to` 时，不会进行压栈操作。

现在我们先不深入`switch_to` ，先考虑五段论的最后一段，即切换到下一个进程后，从它的内核态切换到用户态的过程是怎么样的。

如果下一个进程不是第一次被调度，那么切换到下一个进程时，它的内核栈保存了什么呢？

显然就和上图一样，这时因为，如果它不是第一次被调度，那么它处于阻塞前同样会通过 `switch_to` 切换到别的进程中（内核代码对于多个进程是共用的）。如果是进程第一次被调度，那么就需要看 `fork` 的时候做了什么，下一小节会分析。

同时，切换后下一个进程执行的指令位于`switch_to` 中，最终会走到 C 语言函数 `schedule`的右括号。C 语言函数被编译后，最后三行的汇编代码为：

```assembly
mov %ebp, %esp
popl %ebp # 弹出 EBP
ret # 跳转到 ret_from_sys_call 执行指令
```

EBP 是[栈帧](https://segmentfault.com/a/1190000007977460)结构。总之，内核栈会先将 EBP 弹出， 然后跳转到 `ret_from_sys_call` 执行那里的指令。

（3）`ret_from_sys_call`

执行完该函数，进程就会从内核态切换到用户态。显然它会将此时内核栈的寄存器弹出，然后靠一条 `IRET` 指令中断返回。 `IRET` 指令会将 EIP，CS，EFLAGS，ESP，SS 依次弹出到相应寄存器上。最终内核栈**清零**，进程返回用户态。

```assembly
ret_from_sys_call:
	# 上面省略了信号相关的代码
	# 由下可见 ret_from_sys_call 会弹出一系列寄存器并且最后 IRET，清空内核栈并且返回进程的用户态
3:	popl %eax
	popl %ebx
	popl %ecx
	popl %edx
	pop %fs
	pop %es
	pop %ds
	iret
```

（4）`switch_to` 

再回到 `switch_to`。它是进程实际发生切换的地方，也比较难理解。代码虽然短，但是硬件自动完成了很多事情。

```c
#define switch_to(n) {\
struct {long a,b;} __tmp; \
__asm__("cmpl %%ecx,current\n\t" \
	"je 1f\n\t" \
	"movw %%dx,%1\n\t" \
	"xchgl %%ecx,current\n\t" \
	"ljmp *%0\n\t" \
	"cmpl %%ecx,last_task_used_math\n\t" \
	"jne 1f\n\t" \
	"clts\n" \
	"1:" \
	::"m" (*&__tmp.a),"m" (*&__tmp.b), \
	"d" (_TSS(n)),"c" ((long) task[n])); \
}
```

`switch_to` 是一段嵌入汇编的 C 语言宏函数。这里不介绍其语法，详细可先参考《Linux 内核完全注释》的第 3.3.2 节。

`switch_to` 有四个输入：`*&__tmp.a`，`*&__tmp.b`，`_TSS(n)` 和 `(long) task[n]`。

 `(long) task[n]` 是下一个任务的 PCB 结构体指针，执行汇编指令前它被保存在 ECX 寄存器中；

`_TSS(n)` 是下一个任务的 TSS 段选择符，执行汇编指令前它被保存在 EDX 寄存器中；

`*&__tmp.b` 用于存放要下一个任务的 TSS 段选择符。执行汇编指令前它的值是随机的，在汇编指令中同步由 `movw %%dx,%1\n\t` 被赋值为要切换进程的 TSS 段选择符（ `%1` 代表 `*&__tmp.b`）。

`*&__tmp.a` 用于存放 32 位段内偏移地址，但它是无用的，它的值不重要。需要它只是因为 `ljmp` 需要段选择符 + 段内偏移的形式。

因此，`switch_to` 代码的步骤为：

（1）判断要切换的进程是否为当前进程，如果是则直接调到标号 1 处直接结束，否则继续执行下一条指令。

（2）将 EDX 赋值给 `*&__tmp.a`，即 `__tmp.a`。不清楚为什么要先取 `&` 再取 `*`。

（3）交换 ECX 和 `current` 的值。`current` 是个全局变量，它是当前进程 PCB 结构体的指针。此后，操作系统会将下一个进程视为当前进程。

（4）执行 `ljmp *%0\n\t`。虽然这只是一条汇编指令，但是硬件会做许多事情，执行时间长达 200+ 个时钟周期，这也是要使用内核栈切换的原因。它要做的事情是：将当前所有的物理寄存器（EAX/EBX/ECX/EDX/ESI/EDI/EBP/ESP/EIP/EFLAGS/CR3/CS/DS/ES/FS/GS/SS/LDTR）的快照保存到切换前进程的 TSS 段中（通过 TR 寄存器中的 TSS 段选择符找到的），然后将下一个进程的 TSS 段中保存的寄存器快照赋值给物理寄存器，并将 TR 寄存器设置为下一个进程的 TSS 段选择子。

注意，当执行完 `ljmp *%0\n\t` 后，包括 CS，EIP，ESP，SS 在内的全部寄存器都变成下一个进程的了。也就是说，它的下一条指令`cmpl %%ecx,last_task_used_math\n\t` 是下一个进程在执行。如果下一个进程是第一次被调度，那么它的 CS 和 EIP 是指向进程用户态的代码（因为在 `fork` 中 CS 和 EIP 被赋值为用户态的代码，下一小节有分析），即下一条指令会执行下一个进程的用户态代码；如果不是，下一条指令还是 `cmpl %%ecx,last_task_used_math`，因为下一个进程在之前某个时间切换出去时也是执行了内核代码流程，执行了相同的`switch_to`，但应牢记此时已经是切换后进程了。

（5）最后是处理协处理器相关事情，和进程切换的关系不大，可忽略。

#### 基于 TSS 切换的进程创建

正如李治军老师反复强调，进程创建，就是去做出它被第一次切换时候的样子。因而进程切换是进程创建的基础。

创建进程会设置什么呢？当然是进程相关的信息，包括设置进程的 PID，LDT 表（设置其中的用户代码段和数据段描述符），页目录项和页表项，LDT 和 TSS 段描述符等。除此之外，少不了进程的硬件上下文。在基于 TSS 切换的进程创建中，硬件上下文是保存在 TSS 段的，因此进程创建时也需要设置好进程的 TSS 段。

整个流程是什么样的呢？

（1）`fork` API

这个 `fork` 是用户态的，对于用户程序而言，它是 C 语言库提供的 API。在 Linux 0.11 中，也提供了一个用户态的 `fork` API。提供它是因为，在内核通过 `move_to_user_mode` 切换到任务 0，任务 0 是用户态的，之后也需要借助用户态的 `fork` API 创建任务 1。

实现用户态的 `fork` API 十分简单，只需要利用 `_syscall0` 宏函数就能生成。这是实验二系统调用的相关知识。宏展开后，`fork` API 会变成：

```C
int fork(void)
{
    long __res;
    __asm__ volatile ("int $0x80"
        : "=a" (__res)
        : "0" (__NR_fork));
    if (__res >= 0)
        return (int) __res;
    errno = -__res;
    return -1;
}
```

可以看出，正常情况下 `fork` API 的返回值为 `__res`，而且是通过寄存器 EAX 赋值给 `__res` 的。

fork 的一个使用例子为：

```C
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){

    pid_t PID = fork();

    switch(PID) { 
        // PID == -1 代表 fork 出錯
        case -1:
            perror("fork()");
            exit(-1);
        // PID == 0 代表是子进程
        case 0:
            printf("I'm Child process\n");
            printf("Child's PID is %d\n", getpid());
            break;
        // PID > 0 代表是父进程
        default:
            printf("I'm Parent process\n");
            printf("Parent's PID is %d\n", getpid());
    }
    return 0;
}
```

可见在用户程序中，是通过 `fork` API 的返回值来区分父进程和子进程的。因此可以推断，在 `fork` API 的宏展开中，对于子进程，它的 EAX 应该被赋 0，而父进程的 EAX 应该被赋子进程的 PID 号。

（2） `sys_fork`

`fork` 也是系统调用，进入 `sys_fork` 之前，流程为：`int 0x80` -> `system_call` -> `sys_call_table` -> `sys_fork`。这和实验二系统调用一致。

在 `sys_fork` 首先会调用 `find_empty_process` C 语言函数，它会为新进程分配不重复的 PID 号，保存在全局变量 `last_pid` 中，同时在 PCB 数组中分配一个空闲项，并返回其下标号。如果为负则分配失败，直接退出；否则经过将一系列的物理寄存器压栈后，调用 C 语言函数 `copy_process`。在执行 `copy_process` 前，内核栈的内容为：

```shell
+----------------------------------------------------------+
| # push by hardware                                       |
| SS                                                       |
| ESP                                                      |
| EFLAGS                                                   |
| CS                                                       |
| EIP                                                      |
+----------------------------------------------------------+
| # push in `system_call`                                  |
| DS                                                       |
| ES                                                       |
| FS                                                       |
| EDX                                                      |
| ECX                                                      |
| EBX # EDX, ECX and EBX as parameters to `system_call`    |
| &(pushl %eax) # push by `call sys_call_table(,%eax,4)`   |
+----------------------------------------------------------+
| # push in `sys_fork`                                     |
| GS                                                       |
| ESI                                                      |
| EDI                                                      |
| EBP                                                      |
| EAX # return value of `find_empty_process`               |
| &(addl $20,%esp) # push by `copy_process`                |
+----------------------------------------------------------+
```

（3）`copy_process`

`copy_process` 是完成新进程初始化的主体。先看其函数头：

```C
int copy_process(
	int nr,long ebp,long edi,long esi,long gs,long none,
	long ebx,long ecx,long edx,long fs,long es,long ds,
	long eip,long cs,long eflags,long esp,long ss)
```

在 C 语言函数的调用中，函数入参靠栈传递，而且是逆序的（[这里](https://segmentfault.com/a/1190000007977460)有对 C 语言函数栈帧的分析）。即入参在越左边，它越后入栈。这么规定估计是为了方便定位到入参，例如在被调函数的栈帧中， `8(%ebp)` 是第一个入参，`12(%ebp)` 是第二个入参，以此类推。

总之，`copy_process` 的函数头能够和内核栈完全对应起来，例如函数最左边的入参 `nr` 对应内核栈倒数第二的 `EAX`。

在 `copy_process` 函数中完成了进程的初始化，包括设置进程的 PID，LDT 表，页目录项和页表项，LDT 和 TSS 段描述符等。但在本实验中，我们只需要关注它的硬件上下文的初始化。原生的 Linux 0.11 将硬件上下文保存在 TSS 段，因此是对该进程的 TSS 段进行初始化。

```C
int copy_process(
	int nr,long ebp,long edi,long esi,long gs,long none,
	long ebx,long ecx,long edx,long fs,long es,long ds,
	long eip,long cs,long eflags,long esp,long ss) {

		p = (struct task_struct *) get_free_page(); // 申请一页物理页存放子进程的 PCB 结构体
		// 中间省略了与初始化 TSS 无关的代码
		p->tss.back_link = 0;
        p->tss.esp0 = PAGE_SIZE + (long) p; // 内核栈（初始值）放在与 PCB 相同物理页的页面顶端
        p->tss.ss0 = 0x10; // 内核数据段选择符
        p->tss.eip = eip; // 拷贝了父进程的用户态的指令偏移
        p->tss.eflags = eflags;
        p->tss.eax = 0; // 作为用户态的 fork API 的返回值
        p->tss.ecx = ecx;
        p->tss.edx = edx;
        p->tss.ebx = ebx;
        p->tss.esp = esp; // 拷贝了父进程的用户栈（但是内核栈是独立的）
        p->tss.ebp = ebp;
        p->tss.esi = esi;
        p->tss.edi = edi;
        p->tss.es = es & 0xffff;
        p->tss.cs = cs & 0xffff; // 拷贝了父进程的用户态的段选择符
        p->tss.ss = ss & 0xffff;
        p->tss.ds = ds & 0xffff;
        p->tss.fs = fs & 0xffff;
        p->tss.gs = gs & 0xffff;
        p->tss.ldt = _LDT(nr); // LDT 段选择符
        p->tss.trace_bitmap = 0x80000000;
        // 中间省略了与初始化 TSS 无关的代码
		return last_pid; // 返回子进程的 PID
	}
```

可见，子进程在创建的时候几乎全部寄存器都拷贝了父进程的，除了内核栈与 EAX。当 `copy_process` 执行完毕后子进程就创建好了，但还没有开始调度。只有等到父进程回到 `system_call`，进入了 `schedule`，其调度算法选择了子进程。这样在 `switch_to` 函数中才会切换到子进程。如果子进程是第一次被调度，那么它的 TSS 段就是 `copy_process` 时赋的值，EAX 为 0，`fork` API 的返回值也就是 0，并且会直接返回用户态。而对于父进程而言，`copy_process` 的返回值为 `last_pid` ，编译后通过 EAX 返回，因此 `fork` API 的返回值也就是 `last_pid`。

#### 基于内核栈切换的进程切换

虽然基于 TSS 切换的进程切换的内核代码写起来简单，但是它的执行时间长，长达 200 多个时钟周期，所以为了减少进程切换时间开销，本实验将其改成基于内核栈切换的进程切换。进程切换改了，进程创建当然也要跟着改。现在的 Linux 用的都是内核栈切换这种方法了。

实验手册对该部分的讲解很清晰。下面是实验手册没有深入讨论的几个问题：

- 是否还需要 TSS？

  需要。因为虽然现在不是靠 TSS 做进程切换了，但是当进程进入内核时，还是要依赖 TSS 去找到内核栈，这是硬件提供的方法，也是要求。但是与之前不同，现在只需要一个 TSS 段，TR 寄存器永远指向该 TSS 段，进程切换不会改变 TR 寄存器，而是会改变该 TSS 段的 ESP0 的内容，将 ESP0 赋值为该进程 `task_struct` 相同的物理页的页顶。SS0 不用改是因为所有进程的 SS0 都是 `0x10`，即指向内核数据段。

- TSS 是否还需要是 `task_struct` 中的成员？

  我认为不需要了。因为之前每个进程都是有自己的 TSS，但现在是共用的，完全可以将它抽取出来，作为独立的全局变量，而不是放在 `task_struct` 里面。但是实验指导还是将 TSS 放在 `task_struct` 里面，虽然这么做会浪费空间但不影响功能，猜测是因为代码改动量会比较少，所以就没有将它抽取出来。

- 为什么要将 `switch_to` 从宏函数改成（汇编写的）函数？

  我认为这是和李治军老师讲的用户级线程一节中的 `yield` 函数保持一致。`yield` 函数是普通函数，最后有 `RET` 指令，就是要靠 `RET` 指令去弹出另一个线程的栈顶，继续执行另一个线程在切换前的地方。但如果 `switch_to` 还用宏函数是否可以呢？我认为是可以的，因为内核代码对于所有进程是共用的，另一个进程切换前的地方就是当前进程切换前的地方。

- 为什么要修改 `switch_to` 的入参？

  我认为这能简化代码。`switch_to` 的入参可以保持为 `next`，但这样需要在汇编代码中计算 `pnext` 和 `LDT(next)`。显然将它们传递给汇编代码更简单。

#### 基于内核栈切换的进程创建

创建子进程就是要创建出第一次切换到子进程的样子。

在基于 TSS 切换的进程创建中，在 `copy_process` 函数内将父进程的用户态寄存器赋值到了子进程的 TSS 段。而现在需要将它们赋值给子进程的内核栈作为初始值，靠着内核栈弹出赋值给子进程的寄存器。

想象第一次切换到子进程时，程序指令流会怎么走呢？

与基于 TSS 切换的进程创建不同，此时并不会切换 CS 和 EIP。指令流会在 `switch_to` 中继续往下走，因此我们是**根据指令流去推导内核栈的初始值**。切换为子进程后，`switch_to` 中与栈相关的指令为：

```assembly
1:  popl %eax
    popl %ebx
    popl %ecx
    popl %ebp
ret
```

因此内核栈末尾（后进先出）应该要压入 EAX，EBX，ECX，EBP 这四个寄存器。注意 EAX 应该赋值为 0 ，它作为子进程系统调用的返回值。即在 `copy_process` 函数中，内核栈末尾应该这样赋值：

```C
	// `switch_to` will pop them out
	*(--krnstack) = ebp;
	*(--krnstack) = ecx;
	*(--krnstack) = ebx;
	*(--krnstack) = 0; // child process's EAX(return value) is 0
```

最后一条指令为 `RET`，它的作用是将内核栈顶弹出并且跳转该地址执行代码。由于此时内核栈的内容是我们设置的，因此可以新建一个函数，让指令流跳转到新建的函数中。新建函数名为 `first_return_from_kernel`。在 `copy_process` 函数中，内核栈应该赋值 `first_return_from_kernel` 函数地址：

```c
	*(--krnstack) = (long) first_return_from_kernel;
```

在新建的函数应该做些什么呢？我们希望**当子进程回到用户态时，它的所有寄存器都拷贝父进程的**。所以需要将父进程的寄存器赋给子进程的内核栈初始值，那么切换到子进程时就能靠内核栈弹出赋给物理寄存器。前面我们已经在内核栈末尾弹出了 EAX/EBX/ECX/EBP 这 4 个寄存器，经过和 `copy_process` 的函数原型比较后，还需要将 EDI/ESI/GS/EDX/FS/GS/DS/SS/ESP/EFLAGS/CS/EIP  共 11 个寄存器弹出。

因此在 copy_process 函数中需要在内核栈中赋值这 11 个寄存器：

```c
	// `iret` will pop them out
	*(--krnstack) = ss & 0xffff;
	*(--krnstack) = esp;
	*(--krnstack) = eflags;
	*(--krnstack) = cs & 0xffff;
	*(--krnstack) = eip;

	// `first_return_from_kernel` will pop them out
	*(--krnstack) = ds & 0xffff;;
	*(--krnstack) = es & 0xffff;;
	*(--krnstack) = fs & 0xffff;;
	*(--krnstack) = gs & 0xffff;;
	*(--krnstack) = esi;
	*(--krnstack) = edi;
	*(--krnstack) = edx;
```

 `first_return_from_kernel` 函数需要将 7 个寄存器弹出，最后执行 IRET 中断返回弹出 SS/ESP/EFLAGS/CS/EIP：

```assembly
first_return_from_kernel:
    popl %edx
    popl %edi
    popl %esi
    pop %gs
    pop %fs
    pop %es
    pop %ds
    iret
```

可见，基于 TSS 切换的进程创建中，子进程如果第一次被调度，那么子进程会直接执行用户态的代码（因为这时硬件会自动将保存在 TSS 段的用户态的 CS 和 EIP 弹出到相应寄存器中）；但在本节的方式中，子进程如果第一次被调度，那么它还是会先在内核中跑，将内核栈保存的用户态寄存器信息弹出后再回到用户态。

### 作业参考

本实验的作业可参考[这里](https://gitee.com/na-chern/hit-os-learning/commit/fd8fa5f875051721ae7ccda3b403945c36fe891e)。

### 实验报告

回答下面几个问题。

#### 问题 1

针对下面的代码片段：

```assembly
movl tss,%ecx
addl $4096,%ebx
movl %ebx,ESP0(%ecx)
```

回答问题：

- 为什么要加 4096？

因为此时 `%ebx` 为下一个进程的 `task_struct` 指针。而内核栈栈顶初值需要设置为与该进程 `task_struct` 相同物理页的页顶。一页为 4KB，即 4096。

- 为什么没有设置 `tss` 中的 SS0 ？

这是因为所有进程的 SS0 均为 `0x10`，即内核数据段选择符。切换前后它保持不变，不用进行设置。

#### 问题 2

针对代码片段：

```assembly
*(--krnstack) = ebp;
*(--krnstack) = ecx;
*(--krnstack) = ebx;
*(--krnstack) = 0;
```

回答问题：

- 子进程第一次执行时，`eax = ?` 为什么要等于这个数？哪里的工作让 `eax` 等于这样一个数？

代码片段的最后一行使得，子进程第一次执行时，`eax` 为 0。这是因为在用户态的 `fork` API 需要靠返回值区分父进程和子进程。正常情况下父进程的 `fork` API 返回的是子进程的 PID，而子进程的 `fork` API 返回的为 0。

- 这段代码中的 ebx 和 ecx 来自哪里，是什么含义，为什么要通过这些代码将其写到子进程的内核栈中？

它们来自父进程的用户态的 `fork` API。在系统调用时，靠 `system_call` 将它们压入内核栈中。含义是 `system_call` 函数的入参。因为本实验是靠内核栈维护进程的硬件上下文，因为相关寄存器都要入栈。它们的顺序无法修改，需要和 `switch_to` 中 `pop` 的顺序一致。

- 这段代码中的 `ebp` 来自哪里，是什么含义，为什么要做这样的设置？可以不设置吗？为什么？

`ebp` 也源于父进程的用户态 `fork` API。它是父进程用户态 fork 函数栈帧的栈基指针。不能不设置，因为 `switch_to` 中会将它 `pop` 出来，需要匹配。但是如果 `switch_to` 不使用 `ebp`，从而不 `popl %ebp`，那估计是可以的。

#### 问题 3

- 为什么要在切换完 LDT 之后要重新设置 `fs = 0x17`？而且为什么重设操作要出现在切换完 LDT 之后，出现在 LDT 之前又会怎么样？

表面上看，切换前后的 `fs` 均为 `0x17`，重新设置 `fs = 0x17` 是多余的步骤。其实不然，这时因为段寄存器包含两个部分：显式部分和隐式部分。显示部分就是我们能控制，能赋值的 16 位的段选择符，而隐藏部分则拷贝了一份段描述符的信息。这是为了避免每次访问内存都要去查询 GDT 或者 LDT 表，加快程序执行时间。因此，如果在切换完 LDT 表后不重新设置 `fs`，那么由于隐藏部分的存在，CPU 还会直接从隐藏部分去读取段选择符的段基址，这是切换前的了。处理这种方法最简洁的方式是在对段描述符有任何改动之后就立刻重新加载这些段寄存器，CPU 会自动将从新的段描述符的信息加载到隐藏部分中。因此，如果重设操作出现在切换 LDT 之前，那么该重设将会毫无作用，下一个进程还是会用切换前的段描述符，造成程序异常。

