#ifndef _TYPE_H_
#define _TYPE_H_

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef void (*int_handler)();
typedef void (*irq_handler)(int irq);

typedef void* system_call;

typedef char* va_list ;
#endif
