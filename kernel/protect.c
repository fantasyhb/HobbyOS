#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"

/* defined in kernel.asm */
void	divide_error();
void	single_step_exception();
void	nmi();
void	breakpoint_exception();
void	overflow();
void	bounds_check();
void	inval_opcode();
void	copr_not_available();
void	double_fault();
void	copr_seg_overrun();
void	inval_tss();
void	segment_not_present();
void	stack_exception();
void	general_protection();
void	page_fault();
void	copr_error();
void    hwint01();
void    hwint00();

static void init_idt_desc(unsigned char vector_no, unsigned char desc_type,
			  int_handler handler, unsigned char privilege);
static void init_descriptor(DESCRIPTOR *p_desc, u32 base, u32 limit, u16 attr);
static u32 seg2phy(u16 seg);
static void init_idts();
static void init_gdt();
static void test_idt();

void init_prot()
{
     init_8259A();
     init_idts();
     init_gdt();
}


void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags)
{
     int i;
     int text_color = 0x74; /* 灰底红字 */

     char * err_msg[] = {"#DE Divide Error",
			 "#DB RESERVED",
			 "--  NMI Interrupt",
			 "#BP Breakpoint",
			 "#OF Overflow",
			 "#BR BOUND Range Exceeded",
			 "#UD Invalid Opcode (Undefined Opcode)",
			 "#NM Device Not Available (No Math Coprocessor)",
			 "#DF Double Fault",
			 "    Coprocessor Segment Overrun (reserved)",
			 "#TS Invalid TSS",
			 "#NP Segment Not Present",
			 "#SS Stack-Segment Fault",
			 "#GP General Protection",
			 "#PF Page Fault",
			 "--  (Intel reserved. Do not use.)",
			 "#MF x87 FPU Floating-Point Error (Math Fault)",
			 "#AC Alignment Check",
			 "#MC Machine Check",
			 "#XF SIMD Floating-Point Exception"
     };

     /* 通过打印空格的方式清空屏幕的前五行，并把 disp_pos 清零 */
     disp_pos = 0;
     for(i=0;i<80*40;i++){
	  print_str(" ");
     }
     disp_pos = 0;

     print_str_color("Exception! --> ", text_color);
     print_str_color(err_msg[vec_no], text_color);
     print_str_color("\n\n", text_color);
     print_str_color("EFLAGS:", text_color);
     print_int(eflags);
     print_str_color("CS:", text_color);
     print_int(cs);
     print_str_color("EIP:", text_color);
     print_int(eip);

     if(err_code != 0xFFFFFFFF){
	  print_str_color("Error code:", text_color);
	  print_int(err_code);
     }
}

void spurious_irq(int irq)
{
     print_str("spurious_irq: ");
     print_int(irq);
     print_str("\n");
}

static void init_idts()
{
 
     init_idt_desc(INT_VECTOR_DIVIDE,	DA_386IGate,
		   divide_error,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_DEBUG,		DA_386IGate,
		   single_step_exception,	PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_NMI,		DA_386IGate,
		   nmi,			PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_BREAKPOINT,	DA_386IGate,
		   breakpoint_exception,	PRIVILEGE_USER);

     init_idt_desc(INT_VECTOR_OVERFLOW,	DA_386IGate,
		   overflow,			PRIVILEGE_USER);

     init_idt_desc(INT_VECTOR_BOUNDS,	DA_386IGate,
		   bounds_check,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_INVAL_OP,	DA_386IGate,
		   inval_opcode,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_COPROC_NOT,	DA_386IGate,
		   copr_not_available,	PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_DOUBLE_FAULT,	DA_386IGate,
		   double_fault,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_COPROC_SEG,	DA_386IGate,
		   copr_seg_overrun,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_INVAL_TSS,	DA_386IGate,
		   inval_tss,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_SEG_NOT,	DA_386IGate,
		   segment_not_present,	PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_STACK_FAULT,	DA_386IGate,
		   stack_exception,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_PROTECTION,	DA_386IGate,
		   general_protection,	PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_PAGE_FAULT,	DA_386IGate,
		   page_fault,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_COPROC_ERR,	DA_386IGate,
		   copr_error,		PRIVILEGE_KRNL);

     init_idt_desc(INT_VECTOR_IRQ0, DA_386IGate, hwint00, PRIVILEGE_KRNL);
	
     init_idt_desc(INT_VECTOR_IRQ0 + 1, DA_386IGate, hwint01, PRIVILEGE_KRNL);
     /*
       init_idt_desc(INT_VECTOR_IRQ0+1, DA_386IGate, test_idt, PRIVILEGE_KRNL);	*/

     init_idt_desc(INT_VECTOR_SYS_CALL, DA_386IGate, sys_call, PRIVILEGE_USER);
}

static void init_gdt()
{
     int i;
     memset(&tss, 0, sizeof(tss));
     tss.ss0 = SELECTOR_KERNEL_DS;
     init_descriptor(&gdt[INDEX_TSS],
		     vir2phy(seg2phy(SELECTOR_KERNEL_DS), &tss),
		     sizeof(tss)-1,
		     DA_386TSS);
     tss.iobase = sizeof(tss);

     PROCESS *p_proc = proc_table;
     u16 selector_ldt = INDEX_LDT_FIRST;
     for (i=0; i<NR_TASK+NR_PROC; i++)
     {
	  init_descriptor(&gdt[selector_ldt],
			  vir2phy(seg2phy(SELECTOR_KERNEL_DS), proc_table[i].ldts),
			  LDT_SIZE*sizeof(DESCRIPTOR)-1,
			  DA_LDT);    
	  p_proc++;
	  selector_ldt +=1;
     }

}


static u32 seg2phy(u16 seg)
{
     DESCRIPTOR *p_desc = &gdt[seg>>3];
     return (p_desc->base_high<<24|p_desc->base_mid<<16|p_desc->base_low);
}


static void init_idt_desc(unsigned char vector_no, unsigned char desc_type,
			  int_handler handler, unsigned char privilege)
{
     GATE * p_gate = &idt[vector_no];
     u32 base = (u32)handler;
     p_gate->offset_low = base & 0xffff;
     p_gate->selector = SELECTOR_KERNEL_CS;
     p_gate->dcount = 0;
     p_gate->attr =  desc_type | (privilege << 5);
     p_gate->offset_high = (base>>16) & 0xffff;
}

static void init_descriptor(DESCRIPTOR *p_desc, u32 base, u32 limit, u16 attr)
{
     p_desc->limit_low = limit & 0x0ffff;
     p_desc->base_low = base & 0x0ffff;
     p_desc->base_mid = (base>>16) & 0x0ffff;
     p_desc->attr1 = attr & 0xff;
     p_desc->limit_high_attr2 = ((limit>>16)&0x0f) | (attr>>8) & 0xf0;
     p_desc->base_high = (base>>24) & 0x0ff;
}
