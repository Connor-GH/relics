; make sure string is pointed to in ebx
; string should be in its own label, with a db command
; the user also needs to have the colour they want
; in the ch register. (ah can't be used)
; example:
;
; thingIWantToPrint:
;      db "Hi Mom!", 0
; mov ah, GREEN
; mov ebx, [thingIWantToPrint]
; call print32Bit
[bits 32]                  ; because we are in 32-bit protected mode
VIDEO_MEMORY equ 0xb8000
; all colour names are on a black background
BLACK       equ 0x00       ; 0x00 looks like nothing has been printed until
                           ; the background colour changes.
BLUE        equ 0x01
GREEN       equ 0x02
CYAN        equ 0x03
RED         equ 0x04
MAGENTA     equ 0x05
BROWN       equ 0x06
L_GRAY      equ 0x07
D_GRAY      equ 0x08
L_BLUE      equ 0x09
L_GREEN     equ 0x0a
L_CYAN      equ 0x0b
L_RED       equ 0x0c
L_MAGENTA   equ 0x0d
YELLOW      equ 0x0e
WHITE       equ 0x0f

print32Bit:
    pusha
    pop     ebx
    ;pop     ah
    mov     edx, VIDEO_MEMORY
    jmp     print32BitLoop

        print32BitLoop:
            ; mov     eax, 0
            mov     al, [ebx] ; ebx value is derived from before the call
            ; colour should be in ah register...
            cmp     al, 0       ; making sure we haven't hit end of string
            je      print32BitEnd
            mov     ah, ch
            ;mov     ch, ah
            mov     [edx], ax
            inc     ebx
            add     edx, 2      ; adds 2 bytes to current address.
                                ; 16-bit chars currently (2 bytes is 16 bits).
            jmp     print32BitLoop
        print32BitEnd:
            push    ebx
            ;push    ah
            popa
            ret
