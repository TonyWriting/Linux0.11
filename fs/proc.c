#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <stdarg.h>

char k_buffer[2048];

int proc_read(int devNum, off_t *pos, char *buffer, int count)
{
    char *k_buffer_pointer = &k_buffer[0];
    struct task_struct *p;
    int j, i;

    int offset = sprintf(k_buffer_pointer, "%s\t%s\t%s\t%s\t%s\n", "pid", "state", "father", "counter", "start_time");
    k_buffer_pointer += offset;
    for (i = 0; i <= NR_TASKS; i++)
    {
        p = task[i];
        if (p == NULL)
            break;
        if (p != NULL)
        {
            int offset = sprintf(k_buffer_pointer, "%d\t%d\t%d\t%d\t%ld\n", p->pid, p->state, p->father, p->counter, p->start_time);
            k_buffer_pointer += offset;
        }
    }
    for (j = 0; j < count; j++) /* cat 命令传进来的 count 始终为 512 */
    {
        if (k_buffer[j + (*pos)] == '\0')
            break;
        put_fs_byte(k_buffer[j + (*pos)], &buffer[j + (*pos)]); /* 从内核态内存拷贝到进程用户态内存 */
    }
    *pos = (*pos) + j;
    //printk("total_count: %d, j: %d\n", k_buffer_pointer - k_buffer, j);
    return j; /* 返回读取的字节数 */
}

/* 字符串拷贝 */
int sprintf(char *buf, const char *fmt, ...) /* ... 是 C 语言中的可变参数 */
{
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}