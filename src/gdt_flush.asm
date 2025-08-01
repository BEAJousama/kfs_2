[BITS 32]

global _start
global gdt_flush
extern kernel_main

_start:
    cli
    call kernel_main
    hlt

; GDT flush function
; Takes GDT pointer address as parameter
gdt_flush:
    mov eax, [esp+4]  ; Get the pointer to the GDT passed as parameter
    lgdt [eax]        ; Load the new GDT pointer

    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax        ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax        ; Stack segment
    
    ; Far jump to reload CS register with new code segment (0x08)
    jmp 0x08:.flush

.flush:
    ret               ; Return to caller