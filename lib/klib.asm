%include "sconst.inc"
extern disp_pos

[section .text]

global	print_str
global	print_str_color
global  out_byte
global	in_byte
global	enable_irq
global	disable_irq
global  enable_int
global  disable_int
;;; ----------------------------------------
;;; void print_str(char *);
;;; ----------------------------------------
print_str:
	push	ebp

	mov	ebp, esp
	push	ebx
	push	edi
	push 	esi
	
	mov	esi, [ebp+8]	
	mov	edi, [disp_pos]
	mov	ah, 0fh
.1:
	lodsb
	cmp	al, 0
	jz	.3
	cmp	al, 0ah
	jnz	.2

	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0ffh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.2:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1
.3:
	mov	[disp_pos], edi

	pop 	esi
	pop	edi
	pop	ebx
	pop	ebp
	ret
;;; end of print_str

;;; ----------------------------------------
;;; void print_str_color(char *str, int color)
;;; ----------------------------------------
print_str_color:
	push	ebp

	mov	ebp, esp
	push	ebx
	push	edi
	push 	esi
	
	mov	esi, [ebp+8]	
	mov	edi, [disp_pos]
	mov	ah, [ebp+12]
.1:
	lodsb
	cmp	al, 0
	jz	.3
	cmp	al, 0ah
	jnz	.2

	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0ffh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.2:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1
.3:
	mov	[disp_pos], edi

	pop 	esi
	pop	edi
	pop	ebx
	pop	ebp
	ret
;;; end of print_str_color

;;; ----------------------------------------
;;; void out_byte(u16 port, u8 value)
;;; ----------------------------------------
out_byte:
	mov	edx, [esp+4]
	mov	al, [esp+4+4]
	out	dx, al
	nop
	nop
	ret
;;; end of out_byte

;;; ----------------------------------------
;;; u8 in_byte(u16 port)
;;; ----------------------------------------
in_byte:
	mov	edx, [esp+4]
	xor	eax, eax
	in	al, dx
	nop
	nop
	ret
;;; end of in_byte

;;; ----------------------------------------
;;; void disable_irq(int irq);
;;; ----------------------------------------
disable_irq:
	mov	ecx, [esp + 4]	;irq
	pushf
	cli
	mov	ah, 1
	rol	ah, cl
	cmp	cl, 8
	jae	disable_s 	;slave 8259
disable_m:
	in	al, INT_M_CTLMASK
	or	al, ah
	out	INT_M_CTLMASK, al
	popf
	mov	eax, 1
	ret
disable_s:
	in	al, INT_S_CTLMASK
	or	al, al
	out	INT_S_CTLMASK, al
	popf
	mov	eax, 1
	ret
	
;;; end of disable_irq

;;; ----------------------------------------
;;; void enable_irq(int irq);
;;; ----------------------------------------
enable_irq:
	mov	ecx, [esp+4] 	;irq
	pushf
	cli
	mov	ah, ~1
	rol	ah, cl
	cmp	cl, 8
	jae	enable_s
enable_m:
	in	al, INT_M_CTLMASK
	and	al, ah
	out	INT_M_CTLMASK, al
	popf
	ret
enable_s:
	in	al, INT_S_CTLMASK
	and	al, ah
	out	INT_S_CTLMASK, al
	popf
	ret	
;;; end of enable_eiq

;;; ----------------------------------------
;;; void enable_int();
;;; ----------------------------------------
enable_int:
	sti
	ret
;;; end of enable_int();

;;; ----------------------------------------
;;; void disable_int();
;;; ----------------------------------------
disable_int:
	cli
	ret
;;; end of diable_int();