# 哈工大操作系统课程实验

## 前言

我一直认为：LeetCode、八股文和系统设计只是短期提升面试水平的捷径；而想要走的更远，需要持续地学习名校的经典课程和**实验**，对于非科班的同学更是如此。操作系统涵盖方方面面，许多应用层的设计思想（如多进/线/协程、并发、异步、缓冲）都能在操作系统中找到参照。

[哈工大李老师的操作系统课程](https://www.bilibili.com/video/BV19r4y1b7Aw?p=1)对于进程、内存的教学清晰流畅，并且十分注重代码和实验，并非需要背的文科式的概念堆砌，据说是国内最好的操作系统课程之一（国内另一门[南京大学蒋老师](https://space.bilibili.com/202224425/channel/collectiondetail?sid=192498)的课程也十分不错）。它采用的是实际工业界的 Linux 0.11，约两万行，而 Linux 2.6 代码量在千万行级别，相较之下容易掌握。但 Linux 0.11 和 Bochs 硬件模拟器可能也稍显陈旧，并不是为了教学而设计的，实验过程中也有一些避不开的晦涩难懂的代码，但它们对理解操作系统并没有太大帮助。MIT 的 [6.S081](https://pdos.csail.mit.edu/6.828/2021/schedule.html) 则专门设计了一个教学使用的操作系统 [xv6](https://pdos.csail.mit.edu/6.828/2021/xv6/book-riscv-rev2.pdf)，英语较好的同学可以选择学习它。

对于已经工作的同学，个人建议在学操作系统时要考虑投入产出比。对于核心代码（比如系统调用，进程创建与切换）值得投入大量甚至无限的时间精力，但对于其他部分，不一定要弄懂每一行，只要知道这个函数是在做什么就足够了（有时候还可以猜测它在做什么），战线不宜过长。对于实验也是类似，不必追求从零独立完成。首先只看完视频是根本没法做实验的，至少还需要认真阅读《Linux 内核完全注释》、《Linux 内核设计的艺术》和 Linux 0.11 源码的相应部分。阅读完后可能会有大概的思路，但是到写具体代码时还是难以下手。这时可以看实验参考，底线是理解实验参考的每行代码。

## 学习材料

- [哈工大操作系统视频](https://www.bilibili.com/video/BV19r4y1b7Aw/?p=1&vd_source=683a01bdc1972c35f5b27445f6fa8ccd)
- [《Linux 内核完全注释》](https://book.douban.com/subject/1231236//)，官方教材一，查漏补缺
- [《操作系统原理、实现与实践》](https://book.douban.com/subject/30391722/)，官方教材二，李老师写的配套课程的教材
- [《Linux 内核设计的艺术》](https://book.douban.com/subject/24708145/)，也是权威严谨的书籍，图解丰富，从学生角度出发，读起来连贯不跳跃
- [《品读 Linux 0.11 核心代码》](https://github.com/dibingfa/flash-linux0.11-talk)，浅显易懂，小白都能看懂
- 实验楼的[《操作系统原理与实践》](https://www.lanqiao.cn/courses/115)，官方实验平台
- GitHub [Wangzhike/HIT-Linux-0.11](https://github.com/Wangzhike/HIT-Linux-0.11)，实验记录与讲解

## 环境搭建

### 开发环境

我曾尝试用 [WSL](https://learn.microsoft.com/en-us/windows/wsl/install)，解决了不少环境问题，但最终还是发现 Windows 和 Linux 0.11 的文件系统难以兼容，从而无法与 Linux 0.11 交换文件（现在可能有解决方案，比如 WSL2，但估计也较麻烦）。所以最终转向了[阿里云](https://www.aliyun.com/activity/ambassador/share-gift/goods?taskCode=xfyh2107&recordId=757672&userCode=i5mn5r7m)，选择了其中最便宜的轻量应用服务器，原生 Ubuntu 操作系统，新用户一年仅几十元。

除了做本实验外，拥有一台个人云服务器好处多多，比如用于开发[个人博客网站](https://nachen95.github.io/)，不受地点限制可以随时从某一台个人 PC 机通过 SSH 连接上去。我正是用 VSCode 通过 [SSH](https://code.visualstudio.com/docs/remote/ssh) 连接（可以通过 [SSH keys](https://code.visualstudio.com/docs/remote/troubleshooting#_improving-your-security-with-a-dedicated-key) 免密登录）到云服务器查看、编辑其中的 Linux 0.11 的代码。

在 Ubuntu 上如何编译，运行 Linux 0.11 请参考[《Linux-0.11实验环境准备》](https://github.com/Wangzhike/HIT-Linux-0.11/blob/master/0-prepEnv/%E5%87%86%E5%A4%87%E5%AE%89%E8%A3%85%E7%8E%AF%E5%A2%83.md)。

### 运行环境

上述的云服务器就可以启动硬件模拟器 Bochs 来运行 Linux 0.11。但问题是响应慢，启动了 Linux 0.11 后，在上面敲行命令都要好多秒才能显示出来。猜测原因有可能是图形界面要用比较大的带宽。同时我发现[实验楼](https://www.lanqiao.cn/courses/115)里提供的环境自带桌面系统，可通过浏览器打开。其上的 Linux 0.11 的运行、调试速度快，而且也能 [SSH 直连](https://www.lanqiao.cn/library/shiyanlou-docs/feature/ssh) (缺点是需要开通会员，三个月需几十元）。

因此，整个工作流为：

- 个人 PC 机通过 VSCode SSH 到云服务器上，在 VSCode 上编辑代码，在云服务器上做 Linux 0.11 的编译检查。
- 通过 SCP + `sshpass`  将云服务器的 Linux 0.11 代码打包并拷贝到实验楼的服务器上。
- 在实验楼的服务器解压代码，并编译出 Linux 0.11 内核映像。
- 浏览器打开实验楼服务器对应的桌面 UI，在上面运行、调试 Linux 0.11。

## 代码注释

代码注释在 [Annotation 分支](https://github.com/NaChen95/Linux0.11/tree/Annotation)。

## 实验一 操作系统引导

### 实验内容

此次实验的基本内容是：

1. 阅读《Linux 内核完全注释》的第 6 章，对计算机和 Linux 0.11 的引导过程进行初步的了解；
2. 按照下面的要求改写 0.11 的引导程序 `bootsect.s`
3. 有兴趣同学可以做做进入保护模式前的设置程序 `setup.s`。

改写 `bootsect.s` 主要完成如下功能：

1. `bootsect.s` 能在屏幕上打印一段提示信息“XXX is booting...”，其中 XXX 是你给自己的操作系统起的名字，例如 LZJos、Sunix 等（可以上论坛上秀秀谁的 OS 名字最帅，也可以显示一个特色 logo，以表示自己操作系统的与众不同。）

改写 `setup.s` 主要完成如下功能：

1. `bootsect.s` 能完成 `setup.s` 的载入，并跳转到 `setup.s` 开始地址执行。而 `setup.s` 向屏幕输出一行"Now we are in SETUP"。
2. `setup.s` 能获取至少一个基本的硬件参数（如内存参数、显卡参数、硬盘参数等），将其存放在内存的特定地址，并输出到屏幕上。
3. setup.s 不再加载 Linux 内核，保持上述信息显示在屏幕上即可。

### 原理分析

刚开机上电时，内存 RAM 里空空如也。在磁盘中虽然有操作系统程序，但 CPU 只能从内存里取指令执行，因此操作系统需要先将自己从磁盘搬运到内存中（即自举），并做好相关的初始化。搬运是由操作系统里的 `bootsect.s` 做的。

刚开机上电时，80x86 CPU 从硬件层面将 CS 置为 0xf000，IP 置为 0xfff0，因此 CS:IP 指向的物理地址为（16位实模式）0xffff0，这是 BIOS 程序的起始地址。BIOS 程序是固化在 ROM 里，掉电不会丢失，即这部分地址是映射到 ROM 里的。它的作用是对硬件做检查，并设置中断向量和中断服务程序供后面使用，最后硬件触发由 BIOS 设置好的 int 0x19 中断，它将磁盘的第一个扇区（512B，即 `bootsect` 程序）加载到内存 0x07c00 处并跳转到这里执行。

引导加载程序 `bootsect` 程序将操作系统的其余部分通过 BIOS 设置的中断服务全部读入内存中。包括 `setup.s` 程序和 `system` 模块。`system` 模块的头部是 `head.s` 程序，它也是为操作系统引导服务的。

`setup.s` 作用是利用 BIOS 设置的中断服务读取机器数据，比如光标位置，磁盘参数，根设备号，供后面的系统初始化使用。然后它将 `system` 模块移动到物理地址 0 处，并开启了 32 位保护模式（为此临时设置了 GDT 表）。最后通过 `jumpi 0, 8` （这是个 32 位地址，8 是段选择子）跳转到物理地址 0 处即 `head.s`。

`head.s` 重新设置了 GDT 表，并设置了页目录表和供内核使用的 4 个页表开启分页，共映射 16 MB 内存空间（当时软盘的物理内存大小为这个级别）。这 16 MB 空间的线性地址和物理地址是完全一致的，又由于内核代码段基址为 0，因此在内核代码中的逻辑地址即为物理地址，这给内核代码的编写带来方便（例如，内核函数 [free_page](https://github.com/NaChen95/Linux0.11/blob/05d5343b1569e797e4075f7c1340a42dddaee628/mm/memory.c#LL89C23-L89C23) 释放物理页，其入参 `addr` 既是逻辑地址，又是物理地址） 。注意页目录表和页表是放在物理地址 0 处，这相当于 `head.s` 废弃了自己。最后通过将 `main` 压栈和 `ret` 从汇编代码跳转到 C 语言的 `main` 执行。

### 实验参考

[参考这个提交](https://github.com/NaChen95/Linux0.11/compare/master...Experiment1_OS_boot)。在实验楼的实验指导中已给出参考答案。注意 `bootsect.s` 和 `setup.s` 文件末尾需要保留一个空行，否则在执行 as86 链接时会报错。

### 实验报告

有时，继承传统意味着别手蹩脚。x86 计算机为了向下兼容，导致启动过程比较复杂。请找出 x86 计算机启动过程中，被硬件强制，软件必须遵守的两个“多此一举”的步骤（多找几个也无妨），说说它们为什么多此一举，并设计更简洁的替代方案。

1. BIOS 将 `bootsect.s` 从磁盘读到 `0x07c00` 并跳转到这里执行。然后 `bootsect.s` 又将自己拷贝（为什么要拷贝）到 `0x90000` 处。这个拷贝是多此一举，一种替代方案是修改 BIOS，将 `bootsect.s` 直接读到 `0x90000`。

2. BIOS 在物理地址 0 处开始初始化中断向量表，而操作系统的页目录表最好放在物理地址 0 处，方便实现内核代码的逻辑地址和物理地址的恒等映射。因此`bootsect.s` 在读入操作系统 `system` 模块时，先将它放在`0x10000` 处，等 `setup.s` 使用完 BIOS 中断后，再将操作系统 `system` 模块拷贝到 物理地址 0 处。这个拷贝是多此一举的步骤，一种替代方案是修改 BIOS，让它将中断向量表放在其他不冲突的地方。

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

上述 4 个源文件在 [test/](https://github.com/NaChen95/Linux0.11/tree/Experiment2_system_call/test) 目录下。测试流程为：

```bash
gcc -o iam iam.c
gcc -o whoami whoami.c
gcc -o testlab2 testlab2.c
./testlab2 # 满分 50 分
./testlab2.sh # 满分 30 分
```

### 原理分析

系统调用的本质是中断，因为调用内核段的函数不能像用户段那样简单直接的函数调用，否则会有安全问题（比如某个用户程序读取修改了 root 的密码），即使它们都处于你买的同一个内存条中。

以 C 语言库函数 `printf` 为例，系统调用的路径依次为：

- `printf`
- `int 0x80`
- `system_call`
- 查表 `sys_call_table`
- `sys_write`

下面依次介绍。

（1）`printf` -> `int 0x80`

`printf` 是 C 运行库提供的 API。这是因为如果让用户程序直接调用 `int 0x80`，那么不同平台的可移植性不好（例如Windows 系统调用的中断向量号是 `0x2e` 而不是 `0x80`），同时也比较复杂，于是运行库充当了中间层。从而不同硬件平台都可以通过 `printf` API 进行打印，对用户程序屏蔽了硬件差异。

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

`_syscall3` 有 3 个入参，它们分别通过 EBX，ECX 和 EDX 寄存器传递进入 `int 0x80` 中。同时还会将 `__NR__##name` 通过 EAX 寄存器传入内核中。在 `printf` 中，`__NR__##name` 会拼接为 `__NR__write`。`__NR__write` 是系统调用号，代表着内核态函数 `sys_write` 在函数指针数组 `sys_call_table` 的下标号。处理完中断返回后，会通过 EAX 寄存器将结果返回到用户态变量 `__res` 中。从代码可以看出，`__res` 代表中断处理是否成功，如果它大于等于 `0` 代表成功；否则失败，此时会将它的相反数赋给全局变量 `errno` 中，最后返回 `-1`。

（2）`int 0x80` -> `system_call`

这里是系统调用的最关键之处，也是用户态和内核态发生改变的边界。在 CPU 执行 `int 0x80` 之前还是处于用户态，执行完跳转到 `system_call` 后就变成内核态了，在之后就是普通的函数调用了。

那么这一步发生了什么呢？我们知道，CPU 的工作就是取指令执行，当它看到 `int 0x80` 指令，就会根据中断向量 0x80 ，在中断向量描述符表 IDT 中找到相应的门描述符，特权级检查通过后，就会跳转到相应的中断处理程序的入口地址。

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

代码细节可以看李治军老师的 [P5 节课程](https://www.bilibili.com/video/BV19r4y1b7Aw?p=5&vd_source=683a01bdc1972c35f5b27445f6fa8ccd)。本实验的作业不需要对初始化部分进行改动。系统调用的门描述符会被初始化为：

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

### 实验参考

[参考这个提交](https://github.com/NaChen95/Linux0.11/commit/caf3ae0e30bb2a0e58b7bcb79ebe7bf930940402)。

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

## 实验三 进程运行轨迹的跟踪与统计

### 实验内容

进程从创建（Linux 下调用 `fork()`）到结束的整个过程就是进程的生命期，进程在其生命期中的**运行轨迹实际上就表现为进程状态的多次切换**，如进程创建以后会成为就绪态；当该进程被调度以后会切换到运行态；在运行的过程中如果启动了一个文件读写操作，操作系统会将该进程切换到阻塞态（等待态）从而让出 CPU；当文件读写完毕以后，操作系统会在将其切换成就绪态，等待进程调度算法来调度该进程执行……

本次实验包括如下内容：

- 基于模板 `process.c` 编写多进程的样本程序，实现如下功能：所有子进程都并行运行，每个子进程的实际运行时间一般不超过 30 秒； + 父进程向标准输出打印所有子进程的 id，并在所有子进程都退出后才退出；
- 在 `Linux 0.11` 上实现进程运行轨迹的跟踪。 基本任务是在内核中维护一个日志文件 `/var/process.log`，把从操作系统启动到系统关机过程中所有进程的运行轨迹都记录在这一 log 文件中。
- 在修改过的 0.11 上运行样本程序，通过分析 log 文件，统计该程序建立的所有进程的等待时间、完成时间（周转时间）和运行时间，然后计算平均等待时间，平均完成时间和吞吐量。可以自己编写统计程序，也可以使用 python 脚本程序—— `stat_log.py`（在 `/home/teacher/` 目录下） ——进行统计。
- 修改 0.11 进程调度的时间片，然后再运行同样的样本程序，统计同样的时间数据，和原有的情况对比，体会不同时间片带来的差异。

`/var/process.log` 文件的格式必须为：

```txt
pid    X    time
```

其中：

- pid 是进程的 ID；
- X 可以是 N、J、R、W 和 E 中的任意一个，分别表示进程新建(N)、进入就绪态(J)、进入运行态(R)、进入阻塞态(W) 和退出(E)；
- time 表示 X 发生的时间。这个时间不是物理时间，而是系统的滴答时间(tick)；

三个字段之间用制表符分隔。例如：

```txt
12    N    1056
12    J    1057
4    W    1057
12    R    1057
13    N    1058
13    J    1059
14    N    1059
14    J    1060
15    N    1060
15    J    1061
12    W    1061
15    R    1061
15    J    1076
14    R    1076
14    E    1076
......
```

### 原理分析

本实验有好几点内容，最核心的是第二点的修改内核代码，从而在 `/var/process.log` 文件中记录所有进程的运行轨迹。运行轨迹就是状态的切换，那么进程状态保存在哪里？回答是 `task_struct` 的 `state` 变量。 Linux 0.11 的状态都有哪些呢？答案在源码的 `sched.h` 文件中，一共有 5 个宏：

```
#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4
```

虽然 `TASK_STOPPED` 被定义了，但是 Linux 0.11 代码中并没有将它赋值给任何 `task_struct` 的 `state`，因此 Linux 0.11 尚未实现该状态。

因此，从代码上看，Linux 0.11 的可用状态一共只有 4 种。但它们和实验要求的新建（N）、进入就绪态（J）、进入运行态（R）、进入阻塞态（W） 和退出（E）不是一一对应的。比如， `TASK_RUNNING` 的对应两种状态：就绪态（J）和运行态（R）；而 `TASK_INTERRUPTIBLE` 和 `TASK_UNINTERRUPTIBLE` 通常都对应阻塞态（W）。但是有一个例外，即在 `copy_process` 中创建了进程后马上将该进程的状态置为 `TASK_UNINTERRUPTIBLE`，这里的含义应该就是实验内容中的新建（N），而不是阻塞态（W）。

那么怎么能**毫无遗漏地**找到全部的状态切换点呢？我的方法是在源码里全局搜索上述的状态宏名，再全局搜索 `state` ，因为源码里有时候会直接给 `state` 赋值 0，而非 `TASK_RUNNING`。这样能确保找到全部状态切换点所在的函数，再去理解这些函数。下面依次分析不同的状态宏：

#### TASK_RUNNING

严格地说，`TASK_RUNNING` 有三种含义：就绪态，内核态运行，用户态运行，对应着实验要求两种状态：就绪态（J）和运行态（R）。它出现的地方有：

- `fork.c` 的 `copy_process()`

```c
int copy_process(...) // 入参省略
{
    // fork 和 copy_process() 的详细分析见实验四
	...
    p->state = TASK_RUNNING;	/* do this last, just in case */
    // 此时子进程 p 切换成就绪态（J）
	return last_pid;
}
```

- `sched.c` 的 `schedule()`

```c
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
			if (((*p)->signal & ~(_BLOCKABLE & (*p)->blocked)) && (*p)->state==TASK_INTERRUPTIBLE)
                // 如果进程 p 的信号位图中除去被阻塞的信号还有其他信号，那么它从阻塞态（W）切换为就绪态（J）
				(*p)->state=TASK_RUNNING;
		}

/* this is the scheduler proper: */
	while (1) {
		c = -1;
        // 初始值为 0，如果没有可调度程序，会调度任务 0（虽然此时的任务 0 可能是阻塞态）。任务 0 只会执行系统调用 pause，又会进入这里
        // 因此任务 0 是唯一一个可能从阻塞态到运行态的
		next = 0;
		i = NR_TASKS;
		p = &task[NR_TASKS];
		while (--i) {
			if (!*--p)
				continue; // 跳过不含任务的任务槽
			if ((*p)->state == TASK_RUNNING && (*p)->counter > c) // 两个条件：其一就绪，其二 counter 最大
				c = (*p)->counter, next = i;
		}
		if (c) break; // 如果存在某一个进程的 counter 不为 0（代表时间片未用完），或者没有可以运行的任务（c == -1）则跳出循环
		for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
			if (*p)
				(*p)->counter = ((*p)->counter >> 1) + (*p)->priority;
	}
    // 如果 next 不为当前进程 current，那么在 switch_to 会将 current 切换为就绪态（J），next 切换为运行态（R）
    // Linux 0.11 用 TASK_RUNNING 同时表示就绪态（J）和运行态（R），所以源码里不需要改变 current 和 next 的 state
    // 但是按照本实验要求，需要将它们作区分并打印出来
	switch_to(next); // 切换到另一个任务后，switch_to 同时实现指令流和 TSS 的切换，即其后半部分代码不会执行
} // 该括号不会执行，只有在原任务被切换回来才会继续执行
```

`TASK_RUNNING` 以 0 出现的地方有：

- `sched.c` 的 `sleep_on()`

```c
// 将当前任务（这里任务和进程是一个意思，但是任务更强调进程进入了内核态）置为不可中断的等待状态，并让睡眠队列头指针指向当前任务
// Linux 0.11 未采用真正链表的形式，而是通过内核栈中的 tmp 隐式串联起来
// wake_up 总是唤醒队列头指针指向的任务，而在 tmp 里面保存了后一个任务的地址
// 对于不可中断睡眠 (TASK_UNINTERRUPTIBLE) 只能由 wake_up 函数显式地从隐式等待队列头部唤醒队列头进程
// 再由这个队列头部进程执行 tmp->state = 0 依次唤醒等待队列
void sleep_on(struct task_struct **p) // *p 为等待队列头指针，它总是指向最前面的等待任务
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task)) // current 为当前任务的指针
		panic("task[0] trying to sleep");
	tmp = *p; // tmp 指向队列中队列头指针指向的原等待任务
	*p = current; // 队列头指针指向新加入的等待任务，即调用本函数的任务
	current->state = TASK_UNINTERRUPTIBLE; // 当前进程 current 从运行态（R）切换成（不可中断的）阻塞态（W），只能用 wake_up() 唤醒
	schedule(); // 本任务睡眠，切换到其他任务去了
    // 当调用本函数的任务被唤醒并重新执行时（注意任务被唤醒后不一定会立刻执行它），将比它早进入等待队列中的那一个任务唤醒进入就绪状态
    // 对于不可中断的睡眠，一定是严格按照“队列”的头部进行唤醒
	if (tmp)
		tmp->state=0; // 进程 tmp（比它早进入等待队列中的那一个任务）变成就绪态（J）
}
```

- `sched.c` 的 `interruptible_sleep_on()`

```c
// 除了 wake_up 之外，可以用信号（存放在进程 PCB 中一个向量的某个位置）唤醒
// 与 sleep_on 不同，它会遇到一个问题，即可能唤醒等待队列中间的某个进程，此时需要适当调整队列。这是两个函数的主要区别
void interruptible_sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task))
		panic("task[0] trying to sleep");
	tmp=*p;
	*p=current;
     // 当前进程 current 从运行态（R）切换成（可中断的）阻塞态（W），可以通过信号（在 schedule() 中）和 wake_up() 唤醒
repeat:	current->state = TASK_INTERRUPTIBLE;
	schedule();
    // 当调用本函数的任务被唤醒并重新执行时，判断自己是否为队列头。如果不是（信号唤醒），则将队列头唤醒，并将自己睡眠，重新调度
	if (*p && *p != current) {
		(**p).state=0; // 进程 **p 变成就绪态（J）
		goto repeat;
	}
	*p=NULL;
	if (tmp)
		tmp->state=0; // 进程 tmp（比它早进入等待队列中的那一个任务）从阻塞态（W）切换成就绪态（J）
}
```

- `sched.c` 的 `wake_up()`

```c
void wake_up(struct task_struct **p)
{
	if (p && *p) {
		(**p).state=0; // 进程 **p 从阻塞态（W）切换成就绪态（J）
		*p=NULL;
	}
}
```

上面三个函数都和阻塞态有关。如果只是要完成实验，显然只需在 `state` 改变后加打印即可。但如果要彻底搞懂这三个函数，那比较困难。（可加图片进行分析）赵老师的《Linux 内核完全注释》第 8 章第 7 节对此做了详细分析。此外，赵老师还认为源码 `sleep_on` 和  `wake_up` 存在错误，但我认为应该没有错误，原因见[这里](https://blog.csdn.net/fukai555/article/details/42169885)。

#### TASK_INTERRUPTIBLE

`TASK_INTERRUPTIBLE` 的含义是可中断睡眠/阻塞状态。如果进程在内核态中需要等待系统的某个资源时，那么该进程就能通过 `interruptible_sleep_on()` 进入此状态； 通过信号和 `wake_up()` 都能将此状态切换为 `TASK_RUNNING`。它出现的地方有：

- `sched.c` 的 `schedule()`

- `sched.c` 的 `interruptible_sleep_on()`

上面两个内核函数在对 `TASK_RUNNING` 分析已经出现过了。`schedule()` 中是通过判断信号，若满足条件则将 `TASK_INTERRUPTIBLE` 切换成 `TASK_RUNNING`。

- `sched.c` 的 `sys_pause()`

```c
// 系统无事可做的时候，进程 0 会始终循环调用 sys_pause()，以激活调度算法
// 此时它的状态可以是等待态，等待有其它可运行的进程；也可以叫运行态，因为它是唯一一个在 CPU 上运行的进程，只不过运行的效果是等待
// 这里采用第二种方式，因为如果用第一种的方式，那么 /var/process.log 会多出来许多进程 0 的状态切换而冗杂
// 因此，打印的时候需要判断当前任务是否为 0，如果是则不进行打印
int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE; // 当前任务从运行态（R）切换为阻塞态（W）
	schedule();
	return 0;
}
```

- `exit.c` 的 `sys_waitpid()`

`sys_waitpid()` 被用户态的 `waitpid()` 调用，`waitpid()` 被 `wait()` 调用，`wait()` 被任务 1 的初始化函数 `init()` 调用。

```c
// 挂起当前进程，直到 pid 执行的子进程退出（终止）或者收到终止该进程的信号
// 如果 pid 所指的子进程已经僵死（TASK_ZOMBIE），则本调用立即返回
// 详细分析见《Linux 内核完全注释》
int sys_waitpid(pid_t pid,unsigned long * stat_addr, int options)
{
	...
	if (flag) {
		if (options & WNOHANG) // waitpid 传进来的 options 为 0，因此不会立即返回
			return 0;
		current->state=TASK_INTERRUPTIBLE; // 当前进程从就绪态（J）变成阻塞态（W）
		schedule();
        // 重新调度本任务后，如果没有收到除了 SIGCHLD 以外的信号，还是重复处理
		if (!(current->signal &= ~(1<<(SIGCHLD-1))))
			goto repeat;
		else
			return -EINTR;
	}
	return -ECHILD;
}
```

#### TASK_UNINTERRUPTIBLE

`TASK_UNINTERRUPTIBLE` 的含义是不可中断睡眠/阻塞状态。和 `TASK_INTERRUPTIBLE` 的区别是它只能通过 `wake_up()` 唤醒。它出现的地方有：

- `fork.c` 的 `copy_process()`

```c
int copy_process(...) // 入参省略
{
    // fork 和 copy_process() 的详细分析见实验四
	...
    // get_free_page 获得一页(4KB)内存（内存管理中会讲，不能用 malloc 因为它是用户态的库代码，内核中不能用）
    // 找到 mem_map 为 0（空闲）的那一页，将其地址返回。并且进行类型强制转换，即将该页内存作为 task_stuct(PCB)
    // 这一页 4KB 专门用来存放 task_struct 和内核栈
	p = (struct task_struct *) get_free_page();
	if (!p)
		return -EAGAIN;
	task[nr] = p;
	*p = *current;
    // 新建任务 p 切换为 TASK_UNINTERRUPTIBLE，但是按照实验要求它对应着新建（N）
	p->state = TASK_UNINTERRUPTIBLE;
	...
}
```

- `sched.c` 的 `sleep_on()`

该内核函数在对 `TASK_RUNNING` 分析已经出现过了。

#### TASK_ZOMBIE

`TASK_ZOMBIE` 的含义是僵死状态。当进程已停止运行，但是其父进程还没有询问其状态时，则称该进程处于僵死状态。为了让父进程能够获取其停止运行的信息，此时该子进程的任务数据结构还需要保留着。一旦父进程调用 `wait()` 取得了子进程的信息，则处于该状态进程的任务数据结构会被释放掉。该状态出现的地方有：

- `exit.c` 的 `do_exit()`

```c
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
	current->exit_code = code;
	tell_father(current->father); // 通知父进程，即向父进程发送信号 SIGCHLD 告知当前进程将终止
	schedule();
	return (-1);	/* just to suppress warnings */
}
```

- `exit.c` 的 `sys_waitpid()`

在该函数中，`TASK_ZOMBIE` 只是作为 `switch-case` 的条件，没有赋给任何任务的 `task_struct` ，因此不需要增加打印。

#### TASK_STOPPED

`TASK_STOPPED` 的含义是暂停状态。当进程收到信号 `SIGSTOP`、`SIGTSTP`、`SIGTTIN` 或 `SIGTTOU` 时会进入暂停状态。可向其发送 `SIGGCONT` 信号让进程切换成就绪状态。在 Linux 0.11 中，该状态仅作为 `switch-case` 的条件，因此 Linux 0.11 尚未实现该状态的转换处理，对其不需要增加打印。

可见进程的状态切换点遍布源码各处，可以借此对 `fork.c`，`sched.c` 和 `exit.c` 以及 `main.c` 的 `init()` 函数通盘了解。

#### 多进程样本程序

[process.c](https://github.com/NaChen95/Linux0.11/blob/Experiment3_process_tracking_and_statistics/homework/process.c) 基于 [Wangzhike 仓库](https://github.com/Wangzhike/HIT-Linux-0.11/blob/master/3-processTrack/linux-0.11/process.c)的基础上加了少量注释和打印。

#### 修改时间片

通过实验楼的[分析](https://www.lanqiao.cn/courses/115/learning/?id=570)，进程的时间片初值源于父进程的 `priority`，最终源于进程 0。它的时间片初值是在 `sched.h` 的进程 0 的宏定义中：

```c
#define INIT_TASK \
// 三个值分别对应 state、counter 和 priority。这里的数值代表多少个时钟滴答（tick），在 Linux 0.11 软硬件系统中一个时钟滴答为 10ms
// 修改第二个值影响进程 0 的时间片初值，修改第三个值影响除进程 0 以外所有进程的时间片初值
    { 0,15,15,
```

时间片设置过大，那么其他进程的等待时间会变长；时间片设置过小，那么进程调度次数/耗时（这是一种内耗）变大。因此时间片不宜过小或过大，应合理设置。

### 实验参考

本实验的作业可参考[该提交](https://github.com/NaChen95/Linux0.11/commit/595556a2a8500cf1610bb3b4019d0f09b68f9235)。注意在退出 Bochs 模拟器前，需要先在 Linux 0.11 shell 中执行 `exit`，这样 `process.c` 中的进程的状态信息才会输出到日志中。另外注意提供的 `stat_log.py` 使用的是过时的 Python2 语法，如果要在 Python3 环境运行，需要进行[转换](https://dev.to/rohitnishad613/convert-python-2-to-python-3-in-1-single-click-2a8p)。

### 实验报告

- 结合自己的体会，谈谈从程序设计者的角度看，单进/线程编程和多进/线程编程最大的区别是什么？

单进/线程中代码的运行顺序是固定从上到下的，但是多进/线程的运行顺序是不确定的，可能一会运行进程 A，之后某个未知时刻运行进程 B。对于 I/O-bound 的任务（比如从网络请求数据，访问数据库，读写文件）显然多进/线程的执行时间能更短。对于 CPU-bound 的任务（比如数值计算和图形处理），多进/线程的优势在于能提供一个 UI 界面给用户，从而能监控管理这些任务（单进/线启动了这些任务就没法中途停止或者显示状态，只能等待它们执行完毕）。

## 实验四 基于内核栈切换的进程切换

进程是操作系统的核心，进程的切换与创建是进程的核心。可以说本实验涉及到了操作系统最核心的地方，值得投入大量时间精力，力求弄懂涉及到的每一行代码。

### 实验内容

原生 Linux 0.11 借助 TSS 的硬件机制（后面会有详细论述），一条指令就能完成任务切换。它虽然简单，但这条指令的执行时间却很长，在实现任务切换时大概需要 200 多个时钟周期。

通过内核栈实现任务切换的时间更短，并且它还能使用指令流水的并行优化技术进一步优化时间。所以无论是 Linux 还是 Windows，进程/线程的切换都没有使用 Intel 提供的这种 TSS 切换手段，而都是通过内核栈实现的。

本次实验内容就是将原生 Linux 0.11 中采用的 TSS 切换部分去掉，取而代之的是基于内核栈的切换程序。

### 原理分析

#### 基于 TSS 切换的进程切换

这是 Linux 0.11 原生的方式，即进程切换会将原进程的硬件上下文保存到属于本进程的 TSS 段中，然后再将新进程的 TSS 段中之前保存的硬件上下文赋值给物理寄存器，从而实现进程切换。

在该方式中，每个进程都会有自己单独的 TSS 段。TSS 是一个**段**（LDT 表也是一个段；但是 GDT 表不是，它只是一个结构体），在内核代码中用一个结构体 `tss_struct` 描述，保存在进程的 PCB，即 `task_struct` 结构体中。因此，进程的 TSS 段是其 PCB 结构体的一部分。

整个流程是什么样的呢？

（1）`int 0x80`

首先当然是从上一个进程的用户态通过中断（系统调用）进入内核。在实验二中已经描述过 `int 0x80` 会发生什么，但是现在看来还不够，缺少了对栈切换的描述。执行 `int 0x80` 指令后，堆栈会从用户栈切换到内核栈，并将此时的寄存器 SS、ESP（用户栈的状态），状态寄存器 EFLAGS 和用户态的下一条指令地址 CS、EIP 压栈。这里有几个问题：

- CPU 是怎么找到内核栈的？或者问，从内存的什么位置找到值赋给物理寄存器 SS、ESP？

  它们保存在进程的 TSS 段。想一想也是合理的，因为 TSS 就是用来保存进程的硬件上下文的，它不仅会保存进程的用户栈状态，还会保存内核栈初始状态。那是怎么找到当前进程的 TSS 段呢？这就需要引出 TR 寄存器了，它存放着指向当前进程 TSS 段的段选择子（进程切换时，会确保 TR 寄存器跟着切换）。

  串起来讲：系统调用发生时，CPU 会自动读取 TR 寄存器里的 TSS 段选择子，找到当前进程的 TSS 段描述符，TSS  描述符里面的段基址会指向当前进程的 TSS 段，最终能找到 TSS 段保存的内核栈初始值的 SS0 和 ESP0。那为什么 TSS 描述符的段基址会指向当前进程的 TSS 段呢？这是内核在 `fork` 出该进程时，在 `copy_process` 函数中通过 `set_tss_desc` 宏做的。

- 会跳转到内核的什么位置？

  实验二已分析过，系统调用后跳转的 CS 为 `0x0008`，是内核代码段的段选择子；EIP 为 `$system_call`，是系统调用函数的入口地址。这意味着所有进程从用户态通过系统调用进入内核时，它们用着一样的、共用的内核代码段（当然内核数据段也是一样的，它在 `system_call` 中被设置为 `0x0010`），跑着相同的代码。其实从 GDT 表也能看出来，最开始是内核代码段和数据段选择子（只有一份），接着是各个进程的 LDT 和 TSS 段选择子。

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

下表给出了在进入`switch_to` 之前的内核栈状态：

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
	# 省略了信号相关的代码
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

  需要。因为虽然现在不是靠 TSS 做进程切换了，但是当进程进入内核时，还是要依赖 TSS 去找到内核栈，这是硬件提供的方法，也是限制。但是与之前不同，现在可以只需要一个 TSS 段，TR 寄存器永远指向该 TSS 段，进程切换不会改变 TR 寄存器，而是会改变该 TSS 段的 ESP0 的内容，将 ESP0 赋值为该进程 `task_struct` 相同的物理页的页顶。SS0 不用改是因为所有进程的 SS0 都是 `0x10`，即指向内核数据段。

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

### 实验参考

参考[这个提交](https://github.com/NaChen95/Linux0.11/commit/fd8fa5f875051721ae7ccda3b403945c36fe891e)。

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

## 实验五 信号量的实现和应用

### 实验内容

- 在 Ubuntu 下编写程序，用信号量解决生产者——消费者问题；
- 在 Linux 0.11 中实现信号量，用生产者—消费者程序检验之。

### 原理分析

多进程执行是无序的，为了使得各个进程**同步（合理有序，走走停停）**，需要信号量。例如：

- 对于生产者，当没有空闲缓冲区时，不能继续往里面写数，需要停下来，直到有一个消费者从缓冲区取走一个数；
- 对于消费者，当缓冲区为空时，不能从中取数，需要停下来，直到有一个生产者从缓冲区写入一个数。

信号量包括整形数字（代表当前资源的个数），阻塞队列。以及对信号量的操作，包括创建，**P 原子操作**（消费资源），**V 原子操作**（生产资源）和删除。

```c
typedef struct semaphore {
	char name[SEM_NAME_LEN]; // 信号量名
	int value; // 整形数字
	struct task_struct *queue; // 阻塞队列
} sem_t;
```

为什么要有一个整形数字，而不能指简单的 0,1 开关？因为资源个数是多个，0 和 1 只能表明当前是否有资源，而无法代表当前的资源个数是多少。

对信号量的操作关键是 P 原子操作 `sem_wait`和 V 原子操作 `sem_post`。

### P 原子操作

标准做法已在视频里给出：

```c
P(semaphore s) 
{
	s.value--;
	if(s.value < 0) {
		sleep(s.queue)
	}
}
```

这种 `if` 方式容易理解，但是需要自己实现队列。许多网上的参考答案用的是下面这种：

```c
int sys_sem_wait(semaphore *sem)
{
	cli();
	while( sem->value <= 0 )
		sleep_on(&(sem->queue))
	sem->value--; // 和 while 的顺序不能交换！
	sti();
	return 0;
}
```

它和标准做法相比，主要有四个区别：

- 增加了 `cli` 和 `sti` 。这是通过开关中断来确保对信号量的修改同时只有一个进程，形成**临界区**。对于单 CPU 适用，是比较简单的方法。但实际上，标准做法在 P 原子操作前后也需要临界区，只是这里将它们都放在了一个函数。

- `if` 换成了 `while` 。这是因为直接调用了 Linux 0.11 原生的 `sleep_on` （它可谓是 Linux 0.11 最难理解的函数之一），它通过**内核栈**形成了一个阻塞进程的**隐式链表（栈）**，所以这时我们不需要自己写一个阻塞队列了。这个隐式链表不是阻塞队列，而是阻塞栈。由此而来的副作用是必须将 `if` 换成 `while` ，因为根据 `sleep_on` 的实现，一旦唤醒了阻塞队列的头部进程，那么当它被调度时，会**自动地**唤醒下一个进程，从而最终堵塞队列的全部进程都会被唤醒。因此需要用 `while`，防止生产者只生产了一个资源而唤醒了所有等待的消费者。

- 整形数字的顺序放在了下面。如果是下面的实现

  ```c
  int sys_sem_wait(semaphore *sem)
  {
  	cli();
      sem->value--;
  	while( sem->value < 0 )
  		sleep_on(&(sem->queue))
  	sti();
  	return 0;
  }
  ```

  会有问题。考虑先来了两个消费者，信号量变成 -2。此时来了一个生产者使得信号量加一，信号量变成 -1。生产了一个资源本应该唤醒一个消费者进程，但按照这种实现，两个消费者都没有被唤醒，矛盾。

- 小于号变成了小于等于号。这是因为整形数字顺序的不同。

- 注意 `while` 方式会使得信号量始终大于等于零，而 `if` 方式的信号量可正可负。

### V 原子操作

标准做法已在视频里给出：

```c
V(semaphore s) 
{
	s.value++;
	if(s.value <= 0) {
		sleep(s.queue)
	}
}
```

而参考答案为：

```c
int sys_sem_post(sem_t *sem)
{
    cli();
    sem->value++;
    if ((sem->value) <= 1) // 应该也可以用等号
        wake_up(&(sem->queue));
    sti();
    return 0;
}
```

主要的区别是小于等于号后边从 0 变成了 1。这是因为 `while` 方式的信号量始终大于等于零。如果加一后小于等于一，那么加一前为零，有可能是有消费者在等待中的。`wake_up` 也做了处理，如果阻塞队列中没有进程，直接退出。

### 用户态程序

要求用文件建立共享缓冲区，生产者往文件中写数，消费者从文件中取数。用户态程序的难点在于，消费者每读取一个数，需要将它从文件中删除，而 C 语言文件操作函数没有一个能直接删除数字。为此，当消费者进程读数时，首先调用 `lseek` 得到目前文件指针的位置 A，读出全部 10 个数字，第一个数是要取出送到标准输出的，再将后面 9 个数字写到文件中，最后再调用 `lseek` 将文件指针移动到位置 A 减一个数的位置，从而实现了删除一个数字。如果文件里实际保存的数字不足 10 个是否会有问题呢？不会，因为这相当于扩展了文件，而扩展的数字不会影响前面的。

理解`lseek` ：每个打开的文件都有一个“当前文件偏移量”，是一个非负整数，用以度量从文件开始处计算的字节数。

```markdown
函数原型：
off_t lseek(int fd, off_t offset, int whence); // 打开一个文件的下一次读写的开始位置
参数：
- fd 表示要操作的文件描述符
- offset 是相对于 whence（基准）的偏移量
- whence 可以是 SEEK_SET（文件头），SEEK_CUR（当前文件指针位置） ，SEEK_END 文件尾
返回值：
- 文件读写指针距文件开头的字节大小，出错，返回-1
```

因此用户态程序的消费者进程代码如下：

```c
sem_wait(p_full_buf);
sem_wait(p_mutex);
if( (pos=lseek(fd,0,SEEK_CUR)) == -1){ // 保存当前文件指针位置 A
	printf("seek pos error\n");
}
if( (num = lseek(fd,0,SEEK_SET)) == -1){ // 文件指针置零（文件开头）
	printf("lseek error\n");
}
if( (num=read(fd,data,sizeof(int)*10)) == -1){ // 读出 10 个数字
	printf("read error\n");
}
else{
	printf("consumer 1, pid: %d, buffer index: %d, data: %d\n", consumer1, pos / sizeof(int) - 1, data[0]); /* 减一是因为 pos 指向的是下一个资源的位置*/
}
fflush(stdout); // 将输出缓冲区的信息送到终端，防止多进程终端输出混乱
if( (num = lseek(fd,0,SEEK_SET))== -1){ // 文件指针置零（文件开头）
	printf("lseek error\n");
}
if( (num = write(fd,&data[1],sizeof(int)*9)) == -1){ // 写入 9 个数字
	printf("read and write error\n");
}
if( (num = lseek(fd,pos-sizeof(int),SEEK_SET))== -1){ // 将文件指针移动到位置 A 减一个数的位置
	printf("lseek error\n");
}
sem_post(p_mutex);
sem_post(p_empty_buf);
```

### 总结

信号量的原理是容易理解的，但本实验的代码实现却不简单。难点有两个：

- 没有自己实现阻塞队列，而套用了 Linux 0.11 最难理解的原生函数之一 `sleep_on` 。由此需要将 `if` 方式改为 `while` 方式以及其他一系列改动。
- 用户态程序的文件操作利用 `lseek` 完成删除第一个数字。

### 挂载
本实验需要将用户程序 `pc.c` 从开发环境 Ubuntu 拷贝到 Linux 0.11 的硬盘中。`hdc-0.11.img`是 0.11 内核启动后的根文件系统镜像文件，相当于硬盘。因此，如果需要做文件交换，那么需要先将 `hdc-0.11.img` 挂载带 Ubuntu 下。所谓挂载（mount），就是操作系统的文件系统能够访问存储器的过程，或者说将外存储器的文件系统绑定到了操作系统的[文件系统](https://mp.weixin.qq.com/s?__biz=Mzk0MjE3NDE0Ng==&mid=2247494176&idx=1&sn=b4680b50090bb3c7b9c49379241c536c&chksm=c2c5908df5b2199b361885b32b07ab0f597ab25cd1d70bb75ca13fb897c200285685318f145f&scene=21#wechat_redirect)的过程。在这个过程中，操作系统会将硬盘中的数据以文件系统的格式解读，并将其加载到内存对应的数据结构中。这样操作系统才能通过内存的数据，以文件系统的方式访问硬盘中以二进制形式存放的的文件。而 `unmout` 则是相反的卸载过程。注意在进行 Linux 0.11 和开发环境的文件交换时，需要先在 Linux 0.11 执行 `exit` 并关闭 x86 模拟器 Bochs （点击其右上角的关闭电源按钮）后， Linux 0.11 生成的文件才会保存起来从而能被开发环境访问。

### 实验参考

[参考这个提交](https://github.com/NaChen95/Linux0.11/commit/4a6a351f933e6e969843ff34b19af0c7993ef783)。在 [log.txt](https://github.com/NaChen95/Linux0.11/commit/4a6a351f933e6e969843ff34b19af0c7993ef783#diff-d7e60b33c666e6c4849584b9c36ca85791b55bca10162b75a1d6aa02e8dfdd9e) 中，`producer` 在缓冲区 `buffer` 满了之后停止生产，`consumer` 在缓冲区空的时候停止消费，各进程做到了合作有序，实现同步。为什么总是生产/消费完 10 个资源才转移到其他线程，估计每个进程的时间片时间远大于生产/消费的 10 个资源的时间。注意及时使用 `fflush`，因为 `printf` 只是将信息保存到输出缓冲区，在多个进程同时输出时，它也是一个临界资源（每次仅允许一个进程访问的资源）。

## 实验六 地址映射与共享

### 实验内容

- 用 Bochs 调试工具跟踪 Linux 0.11 的地址翻译（地址映射）过程，了解 IA-32 和 Linux 0.11 的内存管理机制

- 在信号量实验的基础上，为 Linux 0.11 增加共享内存功能，并将生产者—消费者程序移植到 Linux 0.11

具体要求在 `mm/shm.c` 中实现 `shmget()` 和 `shmat()` 两个系统调用。它们能支持 `producer.c` 和 `consumer.c` 的运行即可，不需要完整地实现 POSIX 所规定的功能。

`shmget` （share memory get）功能为：

```
函数原型：int shmget(key_t key, size_t size, int shmflg);
shmget() 会新建/打开一页内存，并返回该页共享内存的 shmid（该块共享内存在操作系统内部的 id）。
所有使用同一块共享内存的进程都要使用相同的 key 参数。如果 key 所对应的共享内存已经建立，则直接返回 shmid。
如果 size 超过一页内存的大小，返回 -1，并置 errno 为 EINVAL。如果系统无空闲内存，返回 -1，并置 errno 为 ENOMEM。
shmflg 参数可忽略。
```

`shmat` （share memory attach）功能为：

```
函数原型：void *shmat(int shmid, const void *shmaddr, int shmflg);
shmat() 会将 shmid 指定的共享页面映射到当前进程的虚拟地址空间中，并将其首地址返回。
如果 shmid 非法，返回 -1，并置 errno 为 EINVAL。
shmaddr 和 shmflg 参数可忽略。
```

### 原理分析

#### 第一部分

实验的第一部分是跟踪一个应用程序的全局变量 `i` 的地址映射（分段、分页）过程，一步步跟踪最终找到它的物理地址。

##### 分段

首先是要得到全局变量的线性地址。在汇编中以 `ds:[eax]` 表示，后者是偏移量（即应用程序中 `&i` 的值），ds 是数据段选择子。它的段基址在哪里呢？在进程的 LDT 中。应用程序的 LDT 又在哪里呢？在 GDT 中。GDTR 寄存器存放 GDT 的物理地址，LDTR 寄存器存放当前进程的 LDT 的段选择子。注意 GDT 只是内存中的一个数据结构，而 LDT 是（专门存放段描述符的）一个段。从而找到了当前进程的 LDT 段基址，再由 ds 找到 LDT 中的数据段描述符，最终得到数据段段基址。数据段段基址加上偏移量等于线性地址。

分段是 CPU 硬件层面提供的机制，但现代 32 位操作系统（无论是 Windows 还是 Linux）都会将段基址置为零，段限长为全空间 4GB （平坦模式）。64 位 CPU 甚至直接将段基址置为零（清晰地区分哪些是 CPU 引入的，哪些是操作系统引入的很重要）。因此可以说现代操作系统实际上没有使用分段机制，只有分页。为什么还要保留分段，可能是为了前向兼容：段的概念是起源于 8086，它是 16 位处理器，但是地址总线是 20 位。16 的位的寄存器如何能访问 20 位的地址？段基地址左移 4 位（就是乘16）再加上段内偏移就是 20 位的地址（这也是 Linux 0.11 刚启动时进入的实模式）。

##### 分页

Linux 0.11 的线性地址有 32 位，前 10 位是页目录表偏移，中间 10 位是页表偏移，最后 12 位是页内偏移。寄存器 CR3 存放页目录表物理基址（全为零），然后根据页目录表偏移查询页目录表，得到页目录表项。页目录表项是一个 32 位的结构体，前 20 位是物理页框号，是页表的物理基址。再根据页表偏移查询页表，得到页表项。页表项和页目录项的结构相同，其前 20 位也是物理页框号，是页的物理基址（页基址）。最后将页基址和页内偏移相加得到最终的物理地址。

分页也是CPU 硬件层面提供的机制。其作用是**提高内存利用率**，**缓解内存碎片问题**，同时提供**保护**。通过分页，操作系统可以不将进程对应的可执行文件全部加载到内存中，而是一页一页地加载进来。

#### 第二部分

实验的第二部分是 Linux0.11 增加共享内存功能，并将生产者—消费者程序检验之。

共享内存的数据结构为：

```c
typedef struct shm_ds
{
    unsigned int key; // 共享内存 id
    unsigned int size; // 共享内存的大小，但实验要求超过一页报错，所以没有实际作用
    unsigned long page; // 共享内存的物理基地址
}shm_ds;
```

`sys_shmget` 的逻辑很简单。如果 `key` 存在，返回共享内存的结构体，如果不存在，调用 `get_free_page` 得到一页物理内存并插入结构体数组 `shm_list` 中。

```c
int sys_shmget(unsigned int key, size_t size)
{
    int i;
    void *page;
    if (size > PAGE_SIZE || key == 0)
        return -EINVAL;
    for (i = 0; i < SHM_SIZE; i++) /* 如果 key 存在，直接返回共享内存的 id */
    {
        if (shm_list[i].key == key)
        {
            printk("Find previous shm key:%u\n", shm_list[i].key);
            return i;
        }
    }
    page = get_free_page(); /* get_free_page 中会将 mem_map 相应位置置为 1 */
    /* 需要将 mem_map 相应位置清零，因为在 sys_shmat 才会将申请的物理页和虚拟地址关联增加引用次数 */
    decrease_mem_map(page);

    if (!page)
        return -ENOMEM;
    printk("Shmget get memory's address is 0x%08x\n", page);
    for (i = 0; i < SHM_SIZE; i++) /* 找到空闲的共享内存 */
    {
        if (shm_list[i].key == 0)
        {
            shm_list[i].page = page;
            shm_list[i].key = key;
            shm_list[i].size = size;
            printk("Generate a new shm key:%u\n", shm_list[i].key);
            return i;
        }
    }
    return -1;
}
```

注意，`decrease_mem_map` 是自定义的函数，它和 `sys_shmat` 中的 `increase_mem_map` 配套使用，函数定义在 [`memory.c`](https://github.com/NaChen95/Linux0.11/blob/fb29004fd027fddcef16353bfdb086a20f253c47/mm/memory.c#L228) 中：

```c
void increase_mem_map(unsigned long page)
{
	page -= LOW_MEM;
	page >>= 12;
	mem_map[page]++;
}

void decrease_mem_map(unsigned long page)
{
	page -= LOW_MEM;
	page >>= 12;
	mem_map[page]--;
}
```

如果不加它们（网上许多参考答案都没有加），那么一个物理页面被两个进程共享，它在两个进程退出时都会被释放一次，而 `mem_map[page]` 值为 1。因此操作系统会在第二次释放的 `free_page` 中 [`panic`](https://github.com/NaChen95/Linux0.11/blob/Experiment6_address_mapping_and_sharing/mm/memory.c#L98) 死机。

`sys_shmat` 建立当前进程的虚拟空间和共享页面的映射。

`current->start_code` 在 fork 进程（执行一个可执行文件是先 `fork` 再 `execve` ）时初始化为进程号乘以 64MB。`current->brk` 在 `do_execve` 中初始化为可执行文件的 Bss 段（存放未初始化的全局变量）加 Data（存放已初始化的全局变量） 加 Text 段的大小。由于我们用的消费者和生产者程序没有使用全局变量，故进程中 Bss 段大小为零；又《完全注释》关于 `exec.c` 注释中写明由于使用的是 ZMAGIC 格式的可执行文件，其数据段和代码段都是页对齐的，所以 `current->brk + current->start_code` 也是页对齐的，无需向上取整。

```c
void *sys_shmat(int shmid)
{
    if (shmid < 0 || SHM_SIZE <= shmid || shm_list[shmid].page == 0 || shm_list[shmid].key == 0)
        return (void *)-EINVAL;
    /* 建立物理地址和线性地址的映射（前 20 位）*/
    /* current->brk 和 current->start_code 都是 4KB 对齐的 */
    printk("current->brk: 0x%08x, current->start_code: 0x%08x\n", current->brk, current->start_code);
    put_page(shm_list[shmid].page, current->brk + current->start_code);

    /* 需要增加一次共享物理页的引用次数，否则会在 free_page 中 panic 死机 */
    increase_mem_map(shm_list[shmid].page);
    current->brk += PAGE_SIZE;
    return (void *)(current->brk - PAGE_SIZE);
}
```

### 实验参考

参考[这个提交](https://github.com/NaChen95/Linux0.11/commit/f2bc091c75504d7b736fdc78177e13d150a66ef6)。除了信号量和共享内存，还增加了一个系统调用 [`get_jiffies`](https://github.com/NaChen95/Linux0.11/commit/fb29004fd027fddcef16353bfdb086a20f253c47#diff-f0650daa46b3094cc95addf8c615b82435874446cde5d6992a777e66a1e86667) 获取系统已走过的时钟中断数来判断信号量是否工作正常。从 [`producer.log`](https://github.com/NaChen95/Linux0.11/commit/f2bc091c75504d7b736fdc78177e13d150a66ef6#diff-3d94a1bf04c5a7891471bc11e98c55e805f349d1157f1f74ed44fa40cf8a7a41)  和 [`consumer.log`](https://github.com/NaChen95/Linux0.11/commit/f2bc091c75504d7b736fdc78177e13d150a66ef6#diff-403c58c34bb897c68559d697a052245a4fb794f7bbb8f71714755709062b750e) 可以看出，生产者进程和消费者进程实现了同步。

## 实验七 终端设备的控制

### 实验内容

本实验的基本内容是修改 Linux 0.11 的终端设备处理代码，对键盘输入和字符显示进行非常规的控制。

在初始状态，一切如常。用户按一次 F12 后，把应用程序向终端输出所有字母都替换为 `*`。用户再按一次 F12，又恢复正常。第三次按 F12，再进行输出替换。依此类推。以 `ls` 命令为例，正常情况：

```shell
# ls
hello.c hello.o hello
```

第一次按 F12，然后输入 ls：

```shell
# **
*****.* *****.* *****
```

第二次按 F12，然后输入 ls：

```shell
# ls
hello.c hello.o hello
```

### 原理分析

Linux 0.11 外设有两类：块设备和字符设备。块设备将信息存储在大小（Linux 0.11 为 1KB）固定的块中，能被随机访问，比如硬盘；字符设备则按照字符流的方式被顺序访问，例如鼠标、键盘、显示器、串口。如李治军老师所说，理解外设管理，要理解三部分：

- 最终触发外设读写的是通过 `out` 往外设控制器的端口（x86 为独立编址）发送指令；
- 理解其中的中断过程；
- 理解如何利用了缓冲区和等待队列。

#### 进程读取磁盘过程

##### 定位文件的磁盘数据块位置

读取文件前需要先 `sys_open` ，它一方面将 `task_struct` 中的 `flip[NR_OPEN]` 和 `file_table[NR_FILE]` 绑定，另一方面将 `file_table[NR_FILE]` 和 `inode_table[NR_INODE]` 绑定，从而通过文件路径名，能找到相应的 inode 节点（存放文件的元信息，比如文件大小，创建时间以及所在块号），数据结构如下下：

```c
// 代码路径：inculde/linux/fs.h
#define NR_OPEN 20
#define NR_INODE 32
#define NR_FILE 64
...
struct file {
	unsigned short f_mode; // 文件操作模式
	unsigned short f_flags; // 文件打开，控制标志
	unsigned short f_count; // 文件句柄数
	struct m_inode * f_inode; // 指向文件对应的 inode 节点
	off_t f_pos; // 当前文件读写位置指针
};

// 代码路径：inculde/linux/sched.h
struct task_struct {
	...
	struct file * filp[NR_OPEN];
	...
};

// 代码路径：fs/file_table.c
struct file file_table[NR_FILE];

// 代码路径：inculde/linux/fs.h
struct m_inode {
	unsigned short i_mode; // 各种标记位，读写执行等，我们 ls 时看到的
	unsigned short i_uid; // 文件的用户 id
	unsigned long i_size; // 文件大小
	unsigned long i_mtime;
	unsigned char i_gid; // 文件的用户组 id
	unsigned char i_nlinks; // 文件入度，即有多少个目录指向它
	unsigned short i_zone[9];  // 文件内容对应的硬盘数据块号
/* these are in memory also */ // 在内存中使用的字段
	struct task_struct * i_wait; // 等待该 inode 节点的进程队列
	unsigned long i_atime; // 文件被访问就会修改这个字段
	unsigned long i_ctime; // 修改文件内容和属性就会修改这个字段
	unsigned short i_dev; // inode所属的设备号
	unsigned short i_num; // inode 的编号，或者说 inode 指针
	unsigned short i_count; // 多少个进程在使用这个 inode
	unsigned char i_lock; // 互斥锁（用于多进程访问磁盘）
	unsigned char i_dirt; // inode 内容是否被修改过
	unsigned char i_pipe; // 是不是管道文件
	unsigned char i_mount; // 该节点是否挂载了另外的文件系统
	unsigned char i_seek;
	unsigned char i_update;
};
```

文件描述符 `fd` 是在 `flip[NR_OPEN]` 数组中的偏移量，由于建立了上述的两级映射关系，有了文件描述符就能找到相应文件的硬盘数据块的位置。

##### 将硬盘数据块读入内核缓冲区数据块

内核缓冲区的作用是提高文件操作的效率。可能会有人有疑问，没有缓冲区，是从进程用户态空间直接到磁盘，有了缓冲区则是先从进程用户态空间到内核缓冲区，然后再是磁盘，明明增加了一次拷贝为什么会更快？因为访问内存速度比访问硬盘快两个数量级，进程 A 在某个时刻执行一段程序，过段时间后可能还是相同数据块的程序（程序的空间局部性）；又或者是进程 B 也要执行相同程序，这都能利用缓冲区，而不是花上百倍时间从硬盘读取。缓冲块和硬盘数据块是一一对应关系，Linux 0.11 中大小都为 1KB。

表面上只调用了 `bread`（block read） ，但其中发生了许多事情：

- 在内核缓冲区找相应设备号，硬盘数据块号的缓冲块，如果有现成已和硬盘数据块同步的，则直接返回（不用执行后面步骤），否则申请一块缓冲块；
- 将申请的缓冲块加锁，保护这个数据块在解锁前不会被其他进程操作；
- 为申请的缓冲块构造请求项（存放要操作的磁头、扇区、柱面等），如果请求项队列为空，那么将请求项置为当前请求项 `dev->current_request` ，并调用请求项处理函数 `do_hd_request` 来处理它。如果请求项队列不空，则将它按照电梯算法（磁头移动距离最小）插入到请求项队列中；
- `outb_p` 往磁盘控制器的端口发送指令，并设置中断服务程序，然后返回 `bread` 中将该进程睡眠（通过内核栈形成链式结构）；
- 过了很久，硬盘控制器将一个扇区的数据从硬盘读入到硬盘控制器的缓冲区（注意它区别于内核缓冲区，它依然属于外设）后，触发硬盘中断，进入中断服务程序；
- 硬盘中断服务程序将磁盘控制器缓冲区的数据块拷贝到内核缓冲块，然后判断数据是否读完，如果否则退出；如果是则唤醒睡眠的进程（由于是内核栈，所以后睡眠的先被唤醒），并调用 `do_hd_request` 处理下一个请求项（如果请求项队列为空则直接返回），这样就实现了处理请求项队列里的循环操作。

##### 将内核缓冲区数据块拷贝到进程用户态空间

这部分很简单，通过 `put_fs_byte` 一个字节一个字节的完成拷贝。

#### 键盘中断处理过程

每个终端设备都对应一个 `tty_struct` 数据结构：

```c
#define TTY_BUF_SIZE 1024

struct tty_struct {
	struct termios termios;
	int pgrp;
	int stopped;
	void (*write)(struct tty_struct * tty); /* tty 写函数指针 */
	struct tty_queue read_q; /* tty 读队列 */
	struct tty_queue write_q; /* tty 写队列 */
	struct tty_queue secondary; /* tty 辅助队列 */
	};

struct tty_queue {
	unsigned long data; // tty 队列缓冲区当前数据的字符行数
	unsigned long head; // 缓冲区中数据头指针
	unsigned long tail; // 缓冲区中数据尾指针
	struct task_struct * proc_list; // 等待本缓冲区的进程列表
	char buf[TTY_BUF_SIZE]; // 缓冲区数据
};

```

tty 是 teletype terminal 的缩写，代指终端设备（字符设备）。`read_q` 用来临时存放从键盘或串行终端输入的原始（Raw）字符序列，`write_q` 用来存放写到控制台显示屏或串行终端的字符序列，辅助队列 `secondary` 用来存放从 `read_q` 中取出的经过行规则模式程序处理过后的数据，称为熟（cooked）模式数据。这是在行规则程序将原始数据中的特殊字符如删除字符变换后的规范输入数据。`secondary` 队列的字符会被上层终端函数 `tty_read` 读取。

当用户在键盘键入了一个字符时，会引起键盘中断响应，中断处理汇编程序会从键盘控制器的端口读取（`inb`）键盘扫描码，然后将其译成相应字符，放入 `read_q` 中，然后调用 C 函数 `do_tty_interrupt`，它有直接调用行规则函数 `copy_to_cooked` 对该字符过滤处理，放入辅助队列中，同时放入写队列 `write_q`，并调用写控制台函数 `con_write`，即回显。`con_write` 中通过 out 将队列的字符写入显示器的端口中。

放入辅助队列之后呢？之后就是唤醒在辅助队列等待的进程，然后该进程会将字符拷贝到自己的用户空间。实际上，键盘中断的最开始并不是用户在键盘键入字符，而是某个进程运行 `scanf` 后通过 `sleep_if_empty` 阻塞自己，然后用户键入字符后才会被唤醒。从进程 `scanf` 的流程为：

```c
// ------------ 进程文件视图 ------------
// 1. scanf 最终会调用 sys_read(0, buf, count)，0 是标准输入的文件描述符。这是因为在 main 中，1 号进程打开的第一个文件是 /dev/tty0 设备文件。而其他的用户进程都是 1 fork 出来的，因为她们的 0 号描述符也都对应 /dev/tty0。/dev/tty0 的 inode 是在制造磁盘时弄好的。

// 2. sys_read
int sys_read(unsigned int fd,char * buf,int count)
{
	struct file * file;
	struct m_inode * inode;
    file=current->filp[fd]；
	inode = file->f_inode;
	if (S_ISCHR(inode->i_mode)) // 会走到这个分支
		return rw_char(READ, inode->i_zone[0], buf, count, &file->f_pos); // inode->i_zone[0] 存放设备号
}

// 3. rw_char
typedef int (*crw_ptr)(int rw,unsigned minor,char * buf,int count,off_t * pos);
static crw_ptr crw_table[]={
	NULL,		/* nodev */
	rw_memory,	/* /dev/mem etc */
	NULL,		/* /dev/fd */
	NULL,		/* /dev/hd */
	rw_ttyx,	/* /dev/ttyx */ // 串行终端，指通过系统串口接入的终端
	rw_tty,		/* /dev/tty */ // 控制（台）终端，即显示器和键盘
	NULL,		/* /dev/lp */
	NULL};		/* unnamed pipes */
int rw_char(int rw,int dev, char * buf, int count, off_t * pos)
{
	crw_ptr call_addr;

	if (MAJOR(dev)>=NRDEVS)
		return -ENODEV;
	if (!(call_addr=crw_table[MAJOR(dev)])) /* call_addr 为函数指针，根据主设备号来确定设备类型，选择不同的处理函数，这里是 rw_tty */
		return -ENODEV;
	return call_addr(rw,MINOR(dev),buf,count,pos); /* MINOR(dev) 获得次设备号 */
}
// 设备号：Linux 0.11 将通过主设备号 + 次设备号来定位某一个设备。主设备号用来区分设备的类型，比如软驱设备（2）、硬盘设备（3）、ttyx 设备（4）。而次设备号用来区分同一类型的多个设备。

// 4. rw_tty 和 rw_ttyx
static int rw_ttyx(int rw,unsigned minor,char * buf,int count,off_t * pos) // 串口终端设备处理函数
{
	return ((rw==READ)?tty_read(minor,buf,count): // 最终调用 tty_read，minor 是次设备号
		tty_write(minor,buf,count));
}
static int rw_tty(int rw,unsigned minor,char * buf,int count, off_t * pos) // 控制台终端读写函数
{
	if (current->tty<0)
		return -EPERM;
	return rw_ttyx(rw,current->tty,buf,count,pos);
}

// 5. tty_read
static void sleep_if_empty(struct tty_queue * queue)
{
	cli();
	while (!current->signal && EMPTY(*queue))
		interruptible_sleep_on(&queue->proc_list);
	sti();
}

int tty_read(unsigned channel, char * buf, int nr)
{
    struct tty_struct * tty;
	char c, * b=buf;
    tty = &tty_table[channel]; /* channel 为次设备号 */
    ...
    while (nr>0) { /* 当欲读取字符数大于零时 */
    	/* 当辅助队列为空，或设置了规范模式标志且辅助队列字符数为零且空闲空间大于 20 时该进程阻塞。
           规范和非规范模式的区别在于，前者会按照实际逻辑处理特殊字符，比如擦除字符会删除缓冲队列的上一个字符；
           而非规范模式则将这些特殊字符都视为普通字符处理。Linux 0.11 应该主要工作在规范模式。
        */
		if (EMPTY(tty->secondary) || (L_CANON(tty) && !tty->secondary.data && LEFT(tty->secondary)>20)) {
			sleep_if_empty(&tty->secondary); /* 本进程进入可中断睡眠状态，等待用户键盘输入 */
			continue;
		}
		do {
            /* 到这里时辅助队列必有字符可以取出，将它放在 c 中 */
			GETCH(tty->secondary,c);
            /* 如果是文件结束符或者换行符，表示取完一行字符，字符行数减一 */
			if (c==EOF_CHAR(tty) || c==10) 
				tty->secondary.data--;
			/* 如果是文件结束符且为规范模式，则返回已读出字符 */
			if (c==EOF_CHAR(tty) && L_CANON(tty))
				return (b-buf);
			else {
            /* 否则将该字符拷贝到用户空间，并将欲读字符数减一 */
				put_fs_byte(c,b++);
				if (!--nr)
					break;
			}
		} while (nr>0 && !EMPTY(tty->secondary));
	...
}
```

### 实验参考

参考[这个提交](https://github.com/NaChen95/Linux0.11/compare/master...Experiment7_terminal_device_control)。

### 实验报告

- 在原始代码中，按下 F12，中断响应后，中断服务程序会调用 `func` ？它实现的是什么功能？

是的。键盘终端处理程序入口点为 [keyboard_interrupt](https://github.com/NaChen95/Linux0.11/blob/c5355bb4b8d57b53384c802e9b106d560e6046cd/kernel/chr_drv/keyboard.S#L37)，它根据键盘扫描码调用 [key_table](https://github.com/NaChen95/Linux0.11/blob/c5355bb4b8d57b53384c802e9b106d560e6046cd/kernel/chr_drv/keyboard.S#L53) 不同的函数。对于 F12，为 `func`。它实现的功能是将功能键转换为特殊字符，比如 F1 为 `esc[[A`，F2 为 `esc[[B`。

- 在你的实现中，是否把向文件输出的字符也过滤了？如果是，那么怎么能只过滤向终端输出的字符？如果不是，那么怎么能把向文件输出的字符也一并进行过滤？

没有。只过滤向终端输出的字符是通过 `con_write` 函数的修改来实现的。过滤向文件输出的字符则通过修改`file_write` 函数来实现。

## 实验八 proc 文件系统的实现

### 实验内容

在 Linux 0.11 上实现 procfs（proc 文件系统）内的 psinfo 结点。当读取此结点的内容时，可得到系统当前所有进程的状态信息。例如，用 cat 命令显示 `/proc/psinfo` 的内容，可得到：

```
$ cat /proc/psinfo
pid    state    father    counter    start_time
0    1    -1    0    0
1    1    0    28    1
4    1    1    1    73
3    1    1    27    63
6    0    4    12    817
```

```
$ cat /proc/hdinfo
total_blocks:    62000;
free_blocks:    39037;
used_blocks:    22963;
...
```

`procfs` 及其结点要在内核启动时自动创建。

相关功能实现在 `fs/proc.c` 文件内。

### 原理分析

整个 Linux 0.11 的文件系统至少有五层抽象：

1. 从磁盘的柱面、扇区和磁头到逻辑块（block）。
2. 多个进程通过请求项形成阻塞队列读写数据块。
3. 为了加速磁盘读写，引出内核缓冲块，它与请求项、数据块一一对应。
4. 从逻辑块到文件，引出 inode 节点。
5. 多个文件，目录组成目录树，形成文件系统：引导块、超级块、inode 节点位图、数据块位图、inode 块和数据块。

实验七是为了理解上面的 1 到 3 层，而本实验是为了理解 4 到 5 层。

根据实验指导，本实验通过 `sys_mkdir`创建了 `/proc` 目录，再通过 `sys_mknod` 新建一个设备文件的 inode 节点，它们的核心代码如下：

```c
#define NAME_LEN 14
/* 目录项（注意区别页目录表中的目录项）数据结构，可见一个目录项大小为 16B */
struct dir_entry {
	unsigned short inode; /* inode 节点的编号（指针）*/
	char name[NAME_LEN]; /* 文件名 */
};

/* sys_mknod 的核心代码 */
/* 创建一个特殊文件或普通文件的 inode 节点 */
int sys_mknod(const char * filename, int mode, int dev)
{
	const char * basename;
	int namelen;
	struct m_inode * dir, * inode;
	struct buffer_head * bh;
	struct dir_entry * de;
	...
    /* 返回 filename 路径所在目录的 inode 节点，赋值给 dir
       例如，filename 为 /mnt/user/hello.txt，则 dir 为 /mnt/user 的 inode 节点，
       basename 为 /hello.txt，namelen 为 hello.txt 的长度。
    */
	if (!(dir = dir_namei(filename,&namelen,&basename)))
		return -ENOENT;
	...
    /* 在 dir 尝试找 basename 的目录项 */
	bh = find_entry(&dir,basename,namelen,&de);
	if (bh) { /* 如果找到了，说明 filename 已存在，返回出错码 */
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
     /* 新建一个 inode 节点：找到空闲的 inode 节点，载入 inode_table[32] 中，更新 inode 位图 */
	inode = new_inode(dir->i_dev);
	if (!inode) {
		iput(dir);
		return -ENOSPC;
	}
	inode->i_mode = mode;
	if (S_ISBLK(mode) || S_ISCHR(mode)) /* 对于块设备和字符设备，i_zone[0] 为其设备号，注意这和普通文件不同 */
		inode->i_zone[0] = dev;
	inode->i_mtime = inode->i_atime = CURRENT_TIME; /* 更新 inode 的修改时间 */
	inode->i_dirt = 1; /* inode 节点在内存改了，但是没有同步到硬盘的标志 */
	bh = add_entry(dir,basename,namelen,&de); /* 在 dir 目录中新增一个目录项 */
	if (!bh) { /* 新增失败，则释放 dir 的 inode 节点并退出 */
		iput(dir);
		inode->i_nlinks=0;
		iput(inode);
		return -ENOSPC;
	}
	de->inode = inode->i_num; /* 设置目录项的 inode 节点编号 */
	bh->b_dirt = 1; /* 置位高速缓冲区已修改标志 */
	iput(dir); /* 释放 dir 的 inode 节点 */
	iput(inode); /* 释放 dir 的 inode 节点 */
	brelse(bh); /* 释放高速缓冲区 */
	return 0;
}

int sys_mkdir(const char * pathname, int mode)
{
	const char * basename;
	int namelen;
	struct m_inode * dir, * inode;
	struct buffer_head * bh, *dir_block;
	struct dir_entry * de;

	...
	if (!(dir = dir_namei(pathname,&namelen,&basename)))
		return -ENOENT;
	...
	bh = find_entry(&dir,basename,namelen,&de);
	if (bh) {
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
	inode = new_inode(dir->i_dev);
	if (!inode) {
		iput(dir);
		return -ENOSPC;
	}
     /* 目录 inode 节点对应的数据块存放目录项，一个目录项大小为 16B，有两个目录项 */
	inode->i_size = 32;
	inode->i_dirt = 1;
	inode->i_mtime = inode->i_atime = CURRENT_TIME;
    /* 新建一个数据块，用于存放 . 和 .. 两个目录项 */
	if (!(inode->i_zone[0]=new_block(inode->i_dev))) {
		iput(dir);
		inode->i_nlinks--;
		iput(inode);
		return -ENOSPC;
	}
	inode->i_dirt = 1;
	if (!(dir_block=bread(inode->i_dev,inode->i_zone[0]))) {
		iput(dir);
		free_block(inode->i_dev,inode->i_zone[0]);
		inode->i_nlinks--;
		iput(inode);
		return -ERROR;
	}
	de = (struct dir_entry *) dir_block->b_data;/* 指向目录项数据块 */
	de->inode=inode->i_num;
	strcpy(de->name,"."); /* 设置当前目录的目录项 */
	de++;
	de->inode = dir->i_num;
	strcpy(de->name,".."); /* 设置上级目录的目录项 */
	inode->i_nlinks = 2; /* 新建目录的硬连接数为 2, 每多一个文件, i_nlinks 加1 */
	dir_block->b_dirt = 1;
	brelse(dir_block);
	inode->i_mode = I_DIRECTORY | (mode & 0777 & ~current->umask);
	inode->i_dirt = 1;
	bh = add_entry(dir,basename,namelen,&de);
	if (!bh) {
		iput(dir);
		free_block(inode->i_dev,inode->i_zone[0]);
		inode->i_nlinks=0;
		iput(inode);
		return -ENOSPC;
	}
	de->inode = inode->i_num;
	bh->b_dirt = 1;
	dir->i_nlinks++;
	dir->i_dirt = 1;
	iput(dir);
	iput(inode);
	brelse(bh);
	return 0;
}
```

值得注意的是，cat 命令每次只会 `read` 512B：

```c
/* cat 命令的核心实现 */
int main(int argc, char* argv[])
{
    char buf[513] = {'\0'};
    int nread;

    int fd = open(argv[1], O_RDONLY, 0);
    while(nread = read(fd, buf, 512)) /* nread 为剩余字节数，当它为 0 时才退出循环 */
    {
        buf[nread] = '\0';
        puts(buf);
    }
    return 0;
}
```

如果进程数较多，需要打印的进程状态的字节数可能大于 512，那就要区分是两次不同的 cat，还是相同的 cat 但是不同的 `read` 。在 [proc.c](https://github.com/NaChen95/Linux0.11/commit/f3f1b41087921548c74566dfeabbbbdd6f1f5153#diff-b39b0987584da45443e0dc21ebd7d3a984dbf489ea622a7079013226a200898d) 中，每次都会将所有进程的状态存在全局数组 `k_buffer` 中（可能会超过 512B），而在拷贝到用户态数组 `buf` 时，拷贝字节数不会超过 512，通过文件指针 `pos` 来确定每次拷贝的起始位置。

### 实验参考

参考[这个提交](https://github.com/NaChen95/Linux0.11/commit/f3f1b41087921548c74566dfeabbbbdd6f1f5153)。

### 实验报告

完成实验后，在实验报告中回答如下问题：

- 如果要求你在 `psinfo` 之外再实现另一个结点，具体内容自选，那么你会实现一个给出什么信息的结点？为什么？

我会给出超级块的信息，因为里面有 inode 节点数，逻辑块数，inode 节点位图块数和逻辑块位图块数等磁盘的全局信息。

- 一次 `read()` 未必能读出所有的数据，需要继续 `read()`，直到把数据读空为止。而数次 `read()` 之间，进程的状态可能会发生变化。你认为后几次 `read()` 传给用户的数据，应该是变化后的，还是变化前的？ + 如果是变化后的，那么用户得到的数据衔接部分是否会有混乱？如何防止混乱？ + 如果是变化前的，那么该在什么样的情况下更新 `psinfo` 的内容？

[这种实现](https://github.com/NaChen95/Linux0.11/commit/f3f1b41087921548c74566dfeabbbbdd6f1f5153)是变化后的。会有混乱，因为拷贝到用户态数组 `buf` 时是按字节拷贝，而 `p->pid`，`p->state` 等是整型占据多个字节。如果两次 `read` 在一个整型数字之间且此时进程状态发生了变化，可能会导致混乱。一种解决方式是保证每次都输出完整的一行进程信息，向下取整。
