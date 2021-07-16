# 哈工大操作系统课程实验

## 前言

## 环境搭建

如果对如何编译、运行 Linux 0.11 以及如何与 Linux 0.11 的文件系统进行文件交换有疑问，请参考实验楼的[《熟悉实验环境》](https://www.lanqiao.cn/courses/115)。

如果不想用实验楼的现成环境，想要在 Ubuntu 16.04/18.04 环境自行搭建 Linux 0.11 的实验环境，请参考 Wangzhike 仓库的[《环境准备》](https://github.com/Wangzhike/HIT-Linux-0.11/blob/master/0-prepEnv/%E5%87%86%E5%A4%87%E5%AE%89%E8%A3%85%E7%8E%AF%E5%A2%83.md)。如果要进行内核的 C 语言级别调试，除了按照上述的《环境准备》搭建环境外，之后还需要执行：

```bash
apt-get install libncurses5:i386
apt-get install libexpat1-dev:i386
```

- 强烈建议**不用** WSL/WSL2 ！

  我曾尝试用 WSL/WSL2，解决了一个个问题，最终还是发现开发主机和 Linux 0.11 的文件系统无法兼容，从而无法与 Linux 0.11 交换文件，最终转向了[阿里云](https://www.aliyun.com/activity/ambassador/share-gift/goods?taskCode=xfyh2107&recordId=757672&userCode=i5mn5r7m)，选择了其中最便宜的云服务器，算上新用户优惠券一年不到 90 元。

## 代码注释

代码注释在 [Annotation 分支](https://gitee.com/na-chern/hit-os-learning/tree/Annotation/)。它与原来的 Linux0.11 源码相比，只增加了一个 commit。

## 实验一 操作系统引导

## 实验二 系统调用

### 实验要求

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
- `sys_call`
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

（2）`int 0x80` -> `sys_call`

这里是系统调用的最关键之处，也是用户态和内核态发生改变的边界。在 CPU 执行 `int 0x80` 之前还是处于用户态，执行完跳转到 `sys_call` 后就变成内核态了，在之后就是普通的函数调用。

那么这一步发生了什么呢？我们知道，CPU 的工作就是取址执行，当它看到 `int 0x80` 指令，就会根据中断向量 0x80 ，在中断向量描述符表 IDT 中找到相应的门描述符，特权级检查通过后，就会跳转到相应的中断处理程序的入口地址。

注意在 32 位机中，寻址是通过段选择子指定的段描述符中的段基址 + 段内偏移的机制（如果对此不了解请看《Linux 内核完全注释》的 4.3 节），因此，在 IDT 表中，存放的每一个表项（也称为门描述符）必须包括两个部分：（中断处理函数所在的）段选择子和段内偏移。除此之外，还会将 EFLAGS，CS 和 EIP 寄存器压栈，如果特权级发生了改变还会涉及栈切换。

该步骤的主要流程是：

- 中断向量号 -> 门描述符 -> （段选择子 + 段内偏移）
- 段选择子 -> 段描述符 -> 段基址
- （段基址 + 段内偏移 + 特权级检查通过） -> 栈切换 -> 寄存器压栈 -> 装载 CS 和 EIP 寄存器，跳转

另一个值得注意的地方是权限检查。需要同时满足两个条件才能跳转：

- 当前特权级 CPL（存放在 CS 寄存器后两位）的值小于等于 IDT 表的门描述符中的 DPL
- 当前特权级 CPL（存放在 CS 寄存器后两位）的值大于等于 GDT 表的段描述符（由门描述符中的段选择子指定）中的 DPL

由于值越小优先级越大，因此这会保证系统调用前后程序的特权级不会降低。

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

可见，系统调用的 CS 段选择符指向了内核代码段描述符，段内偏移是 `system_call` 函数的地址。

（3）`sys_call` -> `sys_call_table`

中断向量号的个数往往很有限，但是需要的中断服务处理有很多种。因此解决方式是，将所有的系统调用都用同一个中断向量号 int 0x80 来表示，然后在 `sys_call` 内部，根据系统调用号 `__NR__##name` 来确定具体的函数是什么。`sys_call_table` 就是存放这些函数的函数指针的数组，`__NR__##name` 是数组偏移。

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

这两个都是整型，并且都是在数组中的偏移。区别在于，`0x80` 是中断向量，它是中断描述符表 IDT 的偏移，代表着这个中断是个系统调用 ；而 `__NR__##name` 则是 `sys_call_table` 数组的偏移，称为系统调用号，它被放在 EAX 寄存器中作为入参传递给内核。`sys_call_table` 里面的元素是函数指针，由 `__NR__##name` 确定调用哪个内核态的函数。IDT 和 `sys_call_table` 都是全局变量。
