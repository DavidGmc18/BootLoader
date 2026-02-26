org 0x7C00
bits 16

jmp entry
nop

%define STAGE1_SECTORS 31

DRIVE_NUMBER db 0

DAP:
    db 0x10
    db 0
    DAP_sectors: dw 0
    DAP_offset: dw 0
    DAP_segment: dw 0
    DAP_LBA: dq 0

entry:
    mov [DRIVE_NUMBER], dl

    ; setup data segments
    mov ax, 0
    mov ds, ax
    mov es, ax
    
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00

    ; some BIOSes might start us at 07C0:0000 instead of 0000:7C00, make sure we are in the
    ; expected location
    push es
    push word check_drive_number
    retf

check_drive_number:
    cmp byte [DRIVE_NUMBER], 0x80
    jae test_extensions

    mov si, warn_drive_number_not_specified
    call print

    mov byte [DRIVE_NUMBER], 0x80

test_extensions:
    mov ah, 41h
    mov dl, [DRIVE_NUMBER]
    mov bx, 0x55AA
    clc ; clear carry flag
    int 13h

    jc .error ; Set on extentions not present

    cmp bx, 0xAA55
    jnz .error

    test cx, 1 ; get bit one
    jnz load_second_stage ; check for DAP

.error:
    mov si, err_extensions_not_present
    call print
    jmp halt

load_second_stage:
    mov dword [DAP_LBA], 1      ; LBA low
    mov dword [DAP_LBA+4], 0    ; LBA high

    mov word [DAP_sectors], STAGE1_SECTORS

    mov word [DAP_offset], 0x0500
    mov word [DAP_segment], 0

    mov di, 5 ; Retry count

.retry:
    push di

    ; Read disk sectors
    mov dl, [DRIVE_NUMBER]
    mov ah, 42h
    mov si, DAP
    stc
    int 13h

    pop di

    ; Check
    jnc .success

    push di

    ; Reset drive after fail
    mov ah, 0
    mov dl, [DRIVE_NUMBER]
    stc
    int 13h

    pop di

    ; Check
    jc .disk_reset_error

    dec di
    test di, di
    jnz .retry

    ; Attempts exhausted
    mov si, err_disk_read_failed
    call print
    jmp halt

.success:
    mov ax, 0x0500
    mov ds, ax
    mov es, ax

    jmp 0x0000:0x0500

    jmp halt

.disk_reset_error:
    mov si, err_disk_reset_failed
    call print
    jmp halt


; si - Error string
print:
    pusha

.loop:
    lodsb
    or al, al
    jz .return

    mov ah, 0Eh
    mov bh, 0
    int 10h

    jmp .loop

.return:
    popa
    ret


halt:
    cli
    hlt
    jmp halt

%define ENDL 0x0D, 0x0A

warn_drive_number_not_specified: db 'Warn: BIOS did not specify drive number, using 0x80', ENDL, 0
err_extensions_not_present: db 'Error: Disk extensions not present', ENDL, 0
err_disk_read_failed: db 'Error: Disk read failed', ENDL, 0
err_disk_reset_failed: db 'Error: Disk reset failed', ENDL, 0

times 446-($-$$) db 0

; MBR
times 64 db 0

dw 0xAA55