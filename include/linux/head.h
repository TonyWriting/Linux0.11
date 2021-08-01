#ifndef _HEAD_H
#define _HEAD_H

typedef struct desc_struct {
	unsigned long a,b;
} desc_table[256]; // 段描述符（8个字节）

extern unsigned long pg_dir[1024];
extern desc_table idt,gdt; // 外部全局变量，定义在 head.s 中

#define GDT_NUL 0
#define GDT_CODE 1
#define GDT_DATA 2
#define GDT_TMP 3

#define LDT_NUL 0
#define LDT_CODE 1
#define LDT_DATA 2

#endif
