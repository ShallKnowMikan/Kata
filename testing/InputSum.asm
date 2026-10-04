section .data
    msg1 db "Insert a value",0xA
    len1 equ $ -msg1 
    msg2 db "Insert another value",0xA

    len2 equ $ -msg2 
section .text
    global _start
_start:

    mov rax, 1
    mov rdi, 1
    lea rsi, [rel msg1]
    mov rdx, len1
    syscall

    sub rsp, 5
    mov rax, 0
    mov rdi, 1
    mov rsi, rsp
    mov rdx, 4
    syscall

    mov byte [rsp + 4], 10

    mov rax, 1
    mov rdi, 1
    lea rsi, [rel msg2]
    mov rdx, len1
    syscall

    sub rsp, 5
    mov rax, 0
    mov rdi, 1
    mov rsi, rsp
    mov rdx, 4
    syscall

    mov byte [rsp + 4], 10


    mov rax, 60
    mov rdi, 0
    syscall