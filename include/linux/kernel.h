/*
 * 'kernel.h' contains some often-used function prototypes etc
 */
void verify_area(void * addr,int count);
volatile void panic(const char * str);
int printf(const char * fmt, ...);
int printk(const char * fmt, ...);
// 应该加上下一行函数原型的声明，因为从 C99 之后需要先声明/定义函数后才能调用它。而早期的 C89/C90 允许隐式声明，因此如果没有下一行只是报警，编译是成功的
int fprintk(int fd, const char *fmt, ...);
int tty_write(unsigned ch,char * buf,int count);
void * malloc(unsigned int size);
void free_s(void * obj, int size);

#define VERBOSE 1 // 0 时不打印函数名（作业要求），1 时则打印
#define free(x) free_s((x), 0)

/*
 * This is defined as a macro, but at some point this might become a
 * real subroutine that sets a flag if it returns true (to do
 * BSD-style accounting where the process is flagged if it uses root
 * privs).  The implication of this is that you should do normal
 * permissions checks first, and check suser() last.
 */
#define suser() (current->euid == 0)

