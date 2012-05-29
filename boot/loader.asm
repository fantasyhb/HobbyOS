org	0100h

jmp 	LoaderStart
	
%include	"fat12h.inc"
%include	"loader16.inc"
%include	"pm.inc"
	
StackTop	equ	0100h
KernelBase	equ	8000h
KernelOffset	equ	0
KernelBasePhy	equ 	80000h
KernelEntryPhy	equ	030400h
LoaderBasePhy	equ	LoaderBase*10h
LoaderBase	equ	9000h
LoaderOffset	equ	0100h

PageDirBase	equ	100000h
PageTblBase	equ	101000h
	
KernelFilename		db	"KERNEL  BIN"
LenKernelFilename	equ	$-KernelFilename
KernelError		db	"No Kernel"
LenKernelError		equ	$-KernelError
	

dwSectorStart		dw	0
dwRootDirSecIndex	dw	0

;;; ------------------------------GDT------------------------------
GDT:			Descriptor	0,		0,		0
Desc_Flat_Code:	Descriptor	0,		0fffffh,	DA_CR  | DA_32| DA_LIMIT_4K
Desc_Flat_RW:		Descriptor	0,		0fffffh,	DA_DRW | DA_32| DA_LIMIT_4K
Desc_Video:		Descriptor	0b8000h,	0ffffh,	DA_DRW | DA_DPL3

SelFlatC	equ	Desc_Flat_Code - GDT
SelFlatRW	equ	Desc_Flat_RW - GDT
SelVideo	equ	Desc_Video - GDT | SA_RPL3

GDTLen		equ	$ - GDT
GDTPtr		dw	GDTLen - 1
		dd	LoaderBasePhy + GDT
;;; end of GDT------------------------------------------------------------
	
LoaderStart:
	mov	ax, cs
	mov 	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, StackTop

;;; get mem info
	mov 	ebx, 0
	mov	di, _memChkBuff
	mov	dword [_dwMemChkCount], 0
LoopMemInfo:	
	mov	eax, 0e820h
	mov	ecx, 20
	mov	edx, 0534d4150h	;	'SMAP'
	int 	15h
	jc	MemInfoError
	add	di, 20
	inc	dword [_dwMemChkCount]
	cmp	ebx, 0
	jnz	LoopMemInfo
	jmp	MemInfoOK
MemInfoError:
	mov	dword [_dwMemChkCount], 0
MemInfoOK:	
	xor 	ah, ah
	xor	dl, dl
	int 	13h		;reset fd

	mov	ax, KernelBase
	mov	es, ax
	mov 	bx, KernelOffset
	mov	ax, 19
	mov 	cl, 1
 	call	ReadSec
		
;;; begin to find loader.bin and load it if existed
	mov	dword [dwSectorStart], RootDirSectorStart
	mov	dword [dwRootDirSecIndex], RootDirSectorCount
LoopReadRootDir:
	cmp    	word [dwRootDirSecIndex], 0
	jz	NoKernelFile
	dec	word [dwRootDirSecIndex]
	mov 	ax, KernelBase
	mov 	es, ax
	mov	bx, KernelOffset
	mov 	ax, [dwSectorStart]
	mov	cl, 1
	call	ReadSec
	mov	si, KernelFilename		;ds:si -> KernelFilename: 'Kernel.bin'
	mov	di, KernelOffset		;es:di -> KernelBase:loaderOffset
	mov 	dx, 16		;files number per sector :512B/32B
LoopReadFilesInSector:
	cmp	dx, 0
	jz	NextRootDirSector
	dec	dx

	mov	cx, LenKernelFilename
LoopCmpFilename:
	cmp	cx, 0
	jz	FindKernelFile
	dec	cx
	lodsb
	cmp	al, byte [es:di]
	jnz	NextDirFile
	inc	di
	jmp	LoopCmpFilename
NextDirFile:
	and	di, 0ffe0h	;remove	low 32 bits of file offset in each sector
	add	di, 20h
	mov	si, KernelFilename
	jmp	LoopReadFilesInSector
NextRootDirSector:
	inc	word [dwSectorStart]
	jmp	LoopReadRootDir
NoKernelFile:
	mov	dh, 2
	mov 	ax, KernelError
	mov	cx, LenKernelError
	call	DispStr16
	jmp	$			
FindKernelFile:
	
	and	di, 0ffe0h
	add	di, 01ah
	
	mov	cx, word [es:di] ;first	FAT entry
	push	cx		 ;save this entry
	add	cx, DataSectorStart
	sub	cx, 2		;

	mov 	ax, KernelBase
	mov	es, ax
	mov	bx, KernelOffset
	mov	ax, cx
	
LoadKernelFile:
	;; print ....
	push	ax
	push	bx
	mov 	ah, 0eh
	mov	al, '.'
	mov 	bl, 0fh
	int 	10h
	pop 	bx
	pop 	ax

	mov	cl, 1
	call	ReadSec
	
	pop	ax		;get the saved entry
	call	GetFATEntry	;get next FAT entry
	cmp	ax, 0fffh
	jz	KernelDone
	push	ax
	add	ax, DataSectorStart
	sub	ax, 2
	add	bx, [BPB_BytesPerSec]
	jmp	LoadKernelFile	;go on loading file
KernelDone:
;	jmp	KernelBase:KernelOffset	; jmp to Kernel.bin!
	lgdt	[GDTPtr]
	cli

	in	al, 92h
	or	al, 00000010b
	out	92h, al

	mov	eax, cr0
	or	eax, 1
	mov 	cr0, eax

	jmp	dword SelFlatC:(LoaderBasePhy + PM_Start) ;jmp to protect mode

;;; end of Loader
;;; ------------------------------
;;; PM code section
;;; ------------------------------
[section PM32]
align	32
[bits 32]
	
PM_Start:
	mov	ax, SelVideo
	mov	gs, ax


	mov	ax, SelFlatRW
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	ss, ax
	mov	esp, StackTop32

	push	strHello
	call	DispStr

	push	strMemInfoHead
	call	DispStr
	add	esp, 4
	call	DispMemInfo

	call	SetupPage
	
InitKernel:
	mov	cx, word [KernelBasePhy+2ch]		; e_phnum: 	program header counter
	movzx	ecx, cx
	mov	ax, word [KernelBasePhy+2ah]		; e_phentsize: 	program header size
	movzx 	eax, ax
	xor	esi, esi
	mov	esi, [KernelBasePhy+1ch]		; e_phoff:	program header table offset in file
	add	esi, KernelBasePhy

LoopReadPH:
	push	eax
	mov	eax, [esi + 10h]
	cmp	eax, 0
	jz	.NextPH
	push	dword [esi + 10h]					; p_filesz:	int nSize
	
	mov	eax, [esi + 04h]			; p_offset
	add	eax, KernelBasePhy
	push	eax					; void *pSrc

	push	dword [esi + 08h] 			; void *pDest
	call	MemCpy
	add	esp, 12
.NextPH:
	pop	eax
	add	esi, eax	;	loop 	LoopReadPH
	dec	cx
	cmp	cx, 0
	jnz	LoopReadPH
	
	
	jmp	SelFlatC:KernelEntryPhy 		;jmp to kernel
	
	
%include	"loader32.inc"

;;; ----------------------------------------
;;; 32-bits data section
;;; ----------------------------------------
[section data32]
align 32
	
DataStart:
;;; real mode
_strHello		db	"hello world", 0ah, 0
_strMemInfoHead:	db	 "BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_strMemSize		db	"RAM Size:", 0
_strReturn		db	0ah,0

_dwDispPos		dd	(80*6+0)*2
_dwMemChkCount		dd	0
_dwMemSize		dd	0
_memChkStruct:	
	_dwBaseAddLow	dd	0
	_dwBaseAddHigh	dd	0
	_dwLengthLow	dd	0
	_dwLengthHigh	dd	0
	_dwType		dd	0
_memChkBuff	times 	20*10	db	0	
;;; pm
dwDispPos	equ	LoaderBasePhy + _dwDispPos
strHello	equ	LoaderBasePhy + _strHello
strMemSize	equ	LoaderBasePhy + _strMemSize
strReturn	equ	LoaderBasePhy + _strReturn
strMemInfoHead	equ	LoaderBasePhy + _strMemInfoHead
dwMemChkCount	equ	LoaderBasePhy + _dwMemChkCount
dwMemSize	equ	LoaderBasePhy + _dwMemSize
memChkBuff	equ	LoaderBasePhy + _memChkBuff
memChkStruct	equ	LoaderBasePhy +	_memChkStruct
	dwBaseAddLow	equ 	LoaderBasePhy + _dwBaseAddLow
	dwBaseAddHigh	equ	LoaderBasePhy + _dwBaseAddHigh
	dwLengthLow	equ	LoaderBasePhy + _dwLengthLow
	dwLengthHigh	equ	LoaderBasePhy + _dwLengthHigh
	dwType		equ	LoaderBasePhy + _dwType

StackSpace	times 	1024	db	0
StackTop32	equ	LoaderBasePhy + $
;;; end of 32-bits data section
