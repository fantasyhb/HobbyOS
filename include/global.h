#ifndef _GLOBAL_H_
#define _GLOBAL_H_

extern int disp_pos;

extern u8 gdt_ptr[8];
extern DESCRIPTOR gdt[GDT_SIZE];
extern u8 idt_ptr[8];
extern GATE idt[IDT_SIZE];
extern DESCRIPTOR ldt[2];
extern TSS tss;

extern PROCESS *p_proc_ready;
extern PROCESS proc_table[];
extern TASK task_table[];
extern TASK user_proc_table[];
extern char task_stack[STACK_SIZE_TOTAL];
extern u32	k_reenter;
extern irq_handler irq_table[];
extern system_call sys_call_table[];
extern int ticks;

extern TTY tty_table[NR_CONSOLE];
extern CONSOLE console_table[NR_CONSOLE];

extern int nr_current_console;


#endif
