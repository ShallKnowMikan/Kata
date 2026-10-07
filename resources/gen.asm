default rel
extern kata_print_float
extern kata_print_int
section .text
global main
main:
    push rbp
    mov rbp, rsp
    mov rax, 3
    push rax
    mov rax, 2
    push rax
    pop rcx
    pop rbx
    mov rax, 1
    cmp rcx, 0
    je .power_res_1
    .power_loop_1:
        imul rax, rbx
        dec rcx
        jnz .power_loop_1
    .power_res_1:
        push rax
    mov rax, 4
    push rax
    mov rax, 2
    push rax
    pop rbx
    pop rax
    cqo
    idiv rbx
    push rax
    pop rbx
    pop rax
    sub rax, rbx
    push rax
    mov rax, 2
    push rax
    mov rax, 1
    push rax
    pop rax
    imul rax, -1
    push rax
    pop rax
    pop rbx
    imul rax, rbx
    push rax
    pop rax
    pop rbx
    add rax, rbx
    push rax
    mov rax, 1
    push rax
    pop rax
    imul rax, -1
    push rax
    pop rax
    pop rbx
    add rax, rbx
    push rax
    push [rsp + 0]

    pop rdi
    call kata_print_int wrt ..plt

    mov eax, 0
    mov rsp,rbp
    pop rbp
    ret
