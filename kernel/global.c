#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"

int disp_pos;
u8 gdt_ptr[8];
DESCRIPTOR gdt[GDT_SIZE];
u8 idt_ptr[8];
GATE idt[IDT_SIZE];

PROCESS proc_table[NR_TASK+NR_PROC];
PROCESS *p_proc_ready;

TASK user_proc_table[NR_PROC] = {{TestA, STACK_SIZE_TESTA, "TestA"},
				 {TestB, STACK_SIZE_TESTB, "TestB"},
				 {TestC, STACK_SIZE_TESTC, "TestC"}};

TASK task_table[NR_TASK] = {{task_tty, STACK_SIZE_TTY, "tty"},
			    {task_sys, STACK_SIZE_SYS, "sys"}};

char task_stack[STACK_SIZE_TOTAL];
u32	k_reenter;
TSS tss;
DESCRIPTOR ldt[2];
irq_handler irq_table[NR_IRQ];
system_call sys_call_table[NR_SYS_CALL] = {sys_printx,
					   sys_sendrec};
int ticks;


TTY tty_table[NR_CONSOLE];
CONSOLE console_table[NR_CONSOLE];

int nr_current_console;
