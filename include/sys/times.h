#ifndef _TIMES_H
#define _TIMES_H

#include <sys/types.h>

struct tms {
	time_t tms_utime; /* User CPU time.  用户态 CPU 时间*/
	time_t tms_stime; /* System CPU time. 内核态 CPU 时间 */ 
	time_t tms_cutime; /* User CPU time of dead children. 已死掉子进程的用户态 CPU 时间*/
	time_t tms_cstime; /* System CPU time of dead children.  已死掉子进程所耗费的内核态 CPU 时间*/
};

extern time_t times(struct tms * tp);

#endif
