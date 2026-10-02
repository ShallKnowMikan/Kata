section .text
global _start
_start:
    mov rax, 9
    push rax
    mov rax, 11
    push rax
    push [rsp + 8]

    mov rax, 20
    push rax
    push [rsp + 16]

    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall