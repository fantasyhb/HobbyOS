%include "sconst.inc"
	
Sel_Kernel_Cs	equ 	8

extern 	cstart			;switch to new GDT loader
extern  exception_handler
extern 	spurious_irq
extern	gdt_ptr			;new GDT pointer
extern	gdt
extern 	idt_ptr
extern 	tss
extern  p_proc_ready
extern  kernel_main
extern 	print_str
extern 	print_int
extern  delay
extern  k_reenter
extern  clock_handler
extern 	irq_table
extern 	sys_call_table	
[section .data]
clock_int_msg	db	"^_^", 0

[section .bbs]
StackSpace	resb	4*1024
StackTop:	
	
[section .text]	
global _start	
global sys_call
	
global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error
global	hwint00
global	hwint01
global	restart			
_start:

	sgdt 	[gdt_ptr]
	call	cstart
	lgdt	[gdt_ptr]
	lidt	[idt_ptr]
	jmp	Sel_Kernel_Cs:csinit
csinit:
	xor 	eax, eax
	mov	ax, SELECTOR_TSS
	ltr	ax
	jmp	kernel_main
	hlt

restart:
	mov	esp, [p_proc_ready]
	lldt	[esp + P_LDT_SEL]
	lea	eax, [esp + P_STACKTOP]
	mov	dword [tss + TSS3_S_SP0], eax

restart_reenter:
	dec	dword [k_reenter]
	pop	gs
	pop	fs
	pop	es
	pop 	ds
	popad
	
	add	esp, 4  	;jump over retaddr
	iretd
	
;;; exception-vector table
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	call	exception_handler
	add	esp, 4*2	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt

;; 8259 int-vector table
%macro	hwint_master	1
	call	save
	
	in	al, INT_M_CTLMASK
	or	al, (1<<%1)
	out	INT_M_CTLMASK, al
	
	mov	al, EOI
	out	INT_M_CTL, al
	
	sti
	push	%1
	call	[irq_table + 4*%1]
	pop	ecx
 	cli
	
	in	al, INT_M_CTLMASK
	and	al, ~(1<<%1)
	out	INT_M_CTLMASK, al
	ret
%endmacro
	
align 	16
hwint00:
	hwint_master	0	
align 	16
hwint01:
	hwint_master	1

;;; end of 8259


save:	
	pushad
	push	ds
	push	es
	push	fs
	push	gs

	mov	esi, edx
	
	mov	dx, ss
	mov	ds, dx
	mov	es, dx

	mov 	edx, esi
	
	mov	esi, esp

	inc	dword [k_reenter]
	cmp	dword [k_reenter], 0
	jne	.1
	
	mov	esp, StackTop	;switchs to kernel stack
	push	restart
	jmp	[esi + RETADR - P_STACKBASE]

.1:
	push	restart_reenter
	jmp	[esi + RETADR - P_STACKBASE]
;; end of save

sys_call:
	call	save
	push	dword [p_proc_ready]
	sti
	
	push	edx
	push	ecx
	push	ebx	
	call	[sys_call_table + eax*4]
	add 	esp, 4*4
	
	mov	[esi + EAXREG - P_STACKBASE], eax
	cli
	ret