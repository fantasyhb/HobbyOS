;;; ----------------------------------------
;;; Display the hex number in al
;;; ----------------------------------------
DispAL:
	push 	edx
	push	ecx
	push	edi

	mov	edi, [dwDispPos]

	mov	ah, 0fh
	mov	dl, al
	shr	al, 4
	mov 	ecx, 2
.begin:
	and	al, 0fh
	cmp 	al, 9
	ja	.1
	add	al, '0'
	jmp	.2	
.1:
	sub	al, 0ah
	add	al, 'A'
.2:
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, dl
	loop	.begin

	mov	[dwDispPos], edi

	pop	edi
	pop	ecx
	pop	edx
	ret	
;;; end of DispAL

;;; ----------------------------------------
;;; Display 32-bit int number in eax
;;; ----------------------------------------
DispInt:
	push	eax
	push	ebx
	mov	ebx, eax
	shr	eax, 24
	call	DispAL
	mov	eax, ebx
	shr	eax, 16
	call	DispAL
	mov	eax, ebx
	shr	eax, 8
	call	DispAL
	mov	eax, ebx
	call	DispAL


	mov	ah, 07h
	mov	al, 'h'
	push	edi
	mov	edi, [dwDispPos]
	mov	[gs:edi], ax
	add	edi, 4
	mov	[dwDispPos], edi
	pop	edi
	pop	ebx
	pop 	eax
	ret
	
;;; end of 

;;; ----------------------------------------
;;; DispStr
;;; ----------------------------------------
DispStr:
	push	ebp

	mov	ebp, esp
	push	ebx
	push	edi
	push 	esi
	
	mov	esi, [ebp+8]	
	mov	edi, [dwDispPos]
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
	mov	[dwDispPos], edi

	pop 	esi
	pop	edi
	pop	ebx
	pop	ebp
	ret
;;; end of DispStr
;;; ----------------------------------------
;;; display return
;;; ----------------------------------------
DispReturn:	
	push	strReturn
	call	DispStr
	add	esp, 4
	ret
;;; end of display return

;;; ----------------------------------------
;;; Name: DispMemInfo
;;; Comment: Display memmory info
;;; ----------------------------------------
DispMemInfo:
	push	edx
	push	ecx
	push	eax
	push	esi
	push	edi
	
	mov	ecx, [dwMemChkCount]
	mov	esi, memChkBuff
LoopMemChkCount:
	mov	edx, 5
	mov	edi, memChkStruct
LoopMemChk:
	mov	eax, [esi]
	call	DispInt
	stosd
	
	add	esi, 4
	dec	edx
	cmp	edx, 0
	jnz	LoopMemChk
	call	DispReturn
	
	cmp	dword [dwType], 1
	jne	.loop
	mov	eax, [dwBaseAddLow]
	add	eax, [dwLengthLow]
	cmp	eax, [dwMemSize]
	jb	.loop
	mov	[dwMemSize], eax
.loop:
	loop	LoopMemChkCount

	call	DispReturn
	push	strMemSize
	call	DispStr
	add	esp, 4
	mov	eax, [dwMemSize]
	call	DispInt

	pop	edi
	pop	esi
	pop	eax
	pop	ecx
	pop	edx
	ret
;;; end of DispMemInfo


;;; ----------------------------------------
;;; name: SetupPage
;;; ----------------------------------------
SetupPage:
	push	eax
	push	ebx
	push	ecx
	push	edx
	push	edi

	xor	edx, edx
	mov	eax, [dwMemSize]
	mov 	ebx, 400000h	; 4M per physical page
	div	ebx
	mov	ecx, eax
	test	edx, edx
	jnz	.noRemainder
	inc	ecx


.noRemainder:
	push	ecx		; save pages' number
	mov	ax, SelFlatRW
	mov	es, ax
	mov	edi, PageDirBase
	xor	eax, eax
	mov	eax, PageTblBase | PG_P | PG_USU | PG_RWW
.LoopPDE:
	stosd
	add	eax, 4096
	loop 	.LoopPDE

	pop	eax
	mov	ebx, 1024
	mul	ebx
	mov	ecx, eax
	mov	edi, PageTblBase
	xor	eax, eax
	mov	eax, PG_P | PG_USU | PG_RWW
.LoopPTE:
	stosd
	add	eax, 4096
	loop	.LoopPTE

	mov	eax, PageDirBase
	mov	cr3, eax
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	jmp	short .Nop
.Nop:
	nop
	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	pop	eax
	ret
	
;;; end of SetupPage


;;; ----------------------------------------
;;; name: MemCpy
;;; comments: void * MemCpy(void *pDest, void *pSrc, int nSize)
;;; ----------------------------------------
MemCpy:
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
;;; end of MemCpy
