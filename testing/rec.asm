
global _start
_start:

    mov rbx, 26
    call _countdown


    mov rax, 60
    mov rdi, 0
    syscall

_countdown:
    push rbp
    mov rbp, rsp

    cmp rbx, 0
    je .end
    dec rbx
    push rbx

    call _countdown
    pop rbx

    sub rsp, 2
    mov rcx, 97
    add rcx, rbx

    mov byte [rsp], cl
    mov byte [rsp + 1], 10

    
    mov rax, 1
    mov rdi, 1
    mov rsi, rsp
    mov rdx, 2
    syscall

    add rsp, 2

.end:
    mov rsp, rbp
    pop rbp
    ret
