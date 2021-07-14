/* the implementation of two new system calls(sys_iam and sys_whoami). */
#include <asm/segment.h>
#include <errno.h>
#include <linux/kernel.h>
#include <unistd.h>
#define MAX_NAME_SIZE 23

// we should avoid non-const global variables in the real work, but this is just an experiment
char STORE_NAME[MAX_NAME_SIZE + 5];
int NAME_SIZE = 0;

// parameter name is the offset adress of the USER space, which can not be accessed in the kernel space
int sys_iam(const char *name)
{

    int i = 0;
    while (get_fs_byte(name + i) != '\0')
    { // copy a byte data from user space to the kernel space
        i++;
        if (i > MAX_NAME_SIZE)
        {
            printk("char num[%d] is more than MAX_NAME_SIZE[%d]\n", i, MAX_NAME_SIZE); // use printk NOT printf in the kernel space for debugging
            return -(EINVAL);
        }
    }

    NAME_SIZE = i;
    i = 0;
    for (; i < NAME_SIZE; i++)
    {
        STORE_NAME[i] = get_fs_byte(name + i);
    }
    return NAME_SIZE;
}

int sys_whoami(char *name, unsigned int size)
{

    if (size < NAME_SIZE)
    {
        return -(EINVAL);
    }
    int i = 0;
    for (; i < NAME_SIZE; i++)
    {
        put_fs_byte(STORE_NAME[i], name + i); // copy a byte data from kernel space to the user space
    }
    put_fs_byte('\0', name + i); // add '\0' to the end
    return NAME_SIZE;
}
