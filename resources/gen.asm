section .text
global _start
_start:
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
    call print_int
    mov rax, 60
    mov rdi, 0
    syscall
print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 32            ; Alloca un buffer di 32 byte sullo stack

    mov rax, rdi           ; Copia il numero da stampare in rax
    mov rcx, 10            ; Divisore costante = 10
    lea rsi, [rbp - 1]     ; rsi punta alla fine del buffer
    mov byte [rsi], 10     ; Aggiungi il newline '\n' alla fine

.print_loop:
    xor rdx, rdx           ; Azzera rdx prima della divisione
    div rcx                ; Divide (rdx:rax) per 10. Quoziente in rax, resto in rdx
    add dl, 48             ; Converte il resto (0-9) nel carattere ASCII ('0'-'9')
    dec rsi                ; Sposta il puntatore indietro di 1 byte
    mov [rsi], dl          ; Salva il carattere nel buffer

    cmp rax, 0             ; Se il quoziente non è 0, continua a dividere
    jnz .print_loop

    ; Ora prepariamo la syscall write
    mov rax, 1             ; sys_write
    mov rdi, 1             ; stdout
    ; rsi punta già all'inizio della stringa appena generata

    ; Calcoliamo la lunghezza della stringa stampata
    lea rdx, [rbp]
    sub rdx, rsi           ; lunghezza = rbp (fine) - rsi (inizio)

    syscall

    leave                  ; Ripristina lo stack
    ret                    ; Torna al programma principale
