; section .data
;     msg db  "Welcome to assambly"
;     len equ $ - msg
;     x equ $-1

; section .text

; global _start

; _start:

;     mov ecx, msg
;     mov esi, x   ; point to last char
;     mov ecx, len
;     std                   ; process backwards
;     rep movsb  


;     mov     edx, len
;     mov     ebx, 1
;     mov     eax, 4
;     int     0x80


;     ; mov     ecx, sum
;     ; mov     edx, 1
;     ; mov     ebx, 1
;     ; mov     eax, 4
;     ; int     0x80

;     mov     eax, 1
;     int     0x80


section .data
    msg db "Hello, x86!"  ; النص الأصلي + newline
    len equ $ - msg

section .bss
    rev resb len               ; مكان لتخزين النص المقلوب

section .text
    global _start

_start:
    ; مؤشرات النسخ
    mov esi, msg + len - 1     ; ESI → آخر حرف في msg
    mov edi, rev               ; EDI → بداية rev
    mov ecx, len               ; عدد الأحرف

    std                        ; DF = 1 → ينسخ من الخلف للأمام
    rep movsb                  ; انسخ ECX بايت من [ESI] إلى [EDI]
    cld                        ; رجّع DF للوضع الطبيعي

    ; اطبع النص المقلوب
    mov eax, 4                 ; sys_write
    mov ebx, 1                 ; stdout
    mov ecx, rev
    mov edx, len
    int 0x80

    ; خروج
    mov eax, 1                 ; sys_exit
    xor ebx, ebx
    int 0x80
