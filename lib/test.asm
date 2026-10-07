default rel

extern kata_float_to_string
extern kata_print_str

section .data
    mio_num dd 7.77

section .text
    global main
main:
    push rbp
    mov rbp, rsp

    movss xmm0, dword [mio_num]
    

    sub rsp, 32
    mov rdi, rsp
    call kata_float_to_string wrt ..plt

    mov [rsp + rax], 0xA
    mov rdi, rsp
    mov rsi, rax
    inc rsi
    call kata_print_str wrt ..plt

    mov eax, 0
    mov rsp,rbp
    pop rbp
    ret