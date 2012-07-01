#ifndef _CONST_H_
#define _CONST_H_
/* assert */
#define ASSERT
#ifdef ASSERT
void assertion_failurel(char *exp, char *file, char* base_file, int nline);
#define assert(exp) if (exp) ;\
     else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif

/* GDT and LDT  */
#define	GDT_SIZE	128
#define	IDT_SIZE	256
#define LDT_SIZE        2
#define INDEX_LDT_C     0
#define INDEX_LDT_RW    1
/* privilege  */
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

#define RPL_KRNL        SA_RPL0
#define RPL_TASK        SA_RPL1
#define RPL_USER        SA_RPL3

/* 8259A interrupt controller ports. */
#define INT_M_CTL     0x20 /* I/O port for interrupt controller       <Master> */
#define INT_M_CTLMASK 0x21 /* setting bits in this port disables ints <Master> */
#define INT_S_CTL     0xA0 /* I/O port for second interrupt controller<Slave>  */
#define INT_S_CTLMASK 0xA1 /* setting bits in this port disables ints <Slave>  */

/* interupts */
#define NR_IRQ        16
#define	CLOCK_IRQ	0
#define	KEYBOARD_IRQ	1
#define	CASCADE_IRQ	2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ	3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ	4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ	5	/* xt winchester */
#define	FLOPPY_IRQ	6	/* floppy disk */
#define	PRINTER_IRQ	7
#define	AT_WINI_IRQ	14	/* at winchester */


/* 8253 PIT */
#define TIMER0          0x40
#define TIMER_MODE       0x43
#define RATE_GENERATOR  0x34     /* 00 11 010 0*/
#define TIMER_FREQ      1193182L
#define HZ              100

/* syscall */
#define NR_SYS_CALL     2

/* VGA */
#define CRTC_ADDR_REG   0x3D4
#define CRTC_DATA_REG   0x3D5
#define START_ADDR_H    0xC
#define START_ADDR_L    0xD
#define CURSOR_H        0xE
#define CURSOR_L        0xF
#define V_MEM_BASE      0xB8000
#define V_MEM_SIZE      0x8000 	/* 32KB */

/* tty */
#define NR_CONSOLE 3

#define MAG_CH_PANIC	'\002'
#define MAG_CH_ASSERT	'\003'

/* printf */
#define STR_LEN 1024

/* ipc */
#define SEND 1
#define RECEIVE 2
#define BOTH 3
#define INTERRUPT	-10

#define TASK_TTY	0
#define TASK_SYS	1
#define ANY		(NR_TASK + NR_PROC + 10)
#define NO_TASK		(NR_TASK + NR_PROC + 20)

enum msgtype {
	HARD_INT = 1,
	GET_TICKS,
};
/* Process */
#define SENDING   0x02	/* set when proc trying to send */
#define RECEIVING 0x04	/* set when proc trying to recv */

#define	RETVAL		u.m3.m3i1
#endif
