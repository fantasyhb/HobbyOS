[section .text]

global	memcpy
global	memset
global  strcpy
global  strlen
;;; ----------------------------------------
;;; void *memcpy(void *es:pDest, void *ds:pSrc, int nSize);
;;; ----------------------------------------

memcpy:
	push	ebp
	mov	ebp, esp
	push	edi
	push	esi
	push	ecx
	mov	edi, [ebp + 8]
	mov	esi, [ebp + 12]
	mov 	ecx, [ebp + 16]
	mov	eax, ecx
	cld
.LoopCpy:
	movsb
	loop .LoopCpy

	mov	eax, ecx
	pop 	ecx
	pop	esi
	pop 	edi
	mov	esp, ebp
	pop	ebp
	ret
;;; ----------------------------------------
;;; void memset(void *pDest, char ch, int nSize);
;;; ----------------------------------------
memset:
	push	ebp
	mov	ebp, esp
	push	edi
	push	eax
	push	ecx
	
	mov	edi, [ebp+8]
	xor 	eax, eax
	mov	eax, [ebp+12]
	mov	ecx, [ebp+16]
	cld
.loopset:
	stosb
	loop .loopset

	pop	ecx
	pop	eax
	pop	edi
	mov	esp, ebp
	pop	ebp
	ret

;;; ----------------------------------------
;;; char *strcpy(char *pDest, char *pSrc)
;;; ----------------------------------------
strcpy:
	push	ebp
	mov	ebp, esp
	push	edi
	push	esi
	mov	edi, [ebp + 8]
	mov	esi, [ebp + 12]
	cld
.LoopStrcpy:
	movsb
	mov	al, [esi]
	cmp 	al, 0
	jnz 	.LoopStrcpy

	mov	eax, [esp+8]
	pop 	ecx
	pop	esi
	pop 	edi
	mov	esp, ebp
	pop	ebp
	ret

;;; ----------------------------------------
;;; int strlen(const char* p_str)
;;; ----------------------------------------
strlen:
	mov 	eax, 0
	mov 	esi, [esp + 4]

.1:
	cmp	byte [esi], 0
	jz	.2
	inc 	eax
	inc 	esi
	jmp	.1
.2:
	ret