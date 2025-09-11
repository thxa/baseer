section .data
    msg db "Hello, Arch!", 0Ah   ; النص الأصلي مع newline
    len equ $ - msg

section .bss
    rev resb len                 ; مكان لتخزين النص المقلوب

section .text
    global _start

_start:
    ; نحط ESI على آخر بايت من msg
    lea rsi, [msg + len - 1]
    ; نحط EDI على بداية rev
    lea rdi, [rev]
    ; عدد الأحرف
    mov rcx, len

    std              ; DF = 1 → ESI/EDI تتناقص
    rep movsb        ; ينسخ من msg (من الأخير) إلى rev (من البداية)
    cld              ; DF ← 0 (عشان ما يخرب أي كود بعده)

    ; نطبع النص المقلوب
    mov rax, 1       ; sys_write
    mov rdi, 1       ; stdout
    lea rsi, [rev]
    mov rdx, len
    syscall

    ; exit(0)
    mov rax, 60
    xor rdi, rdi
    syscall
