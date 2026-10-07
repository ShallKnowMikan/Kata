default rel
extern snprintf
section .data
    prefisso_float db "%g", 10,0
    fmt_float db "%g", 0

section .text
    global input:function
input:
    mov rdx, rsi
    mov rsi, rdi
    mov rax, 0
    mov rdi, 0
    syscall
    ret
section .text
    global kata_print_str:function
kata_print_str:
    ; Convenzione che decidiamo noi per il tuo compilatore:
    ; rdi = conterrà il puntatore (indirizzo) della stringa
    ; rsi = conterrà la lunghezza della stringa in byte
    
    ; Prepariamo la syscall sys_write di Linux
    mov rdx, rsi      ; 3° argomento sys_write: la lunghezza
    mov rsi, rdi      ; 2° argomento sys_write: il puntatore alla stringa
    mov rdi, 1        ; 1° argomento sys_write: File Descriptor 1 (Standard Output)
    mov rax, 1        ; Numero della syscall (1 = sys_write)
    
    syscall           ; Bam! Stampato senza usare libc e senza zeri.
    
    ret

extern printf
global kata_print_float:function

kata_print_float:
    cvtss2sd xmm0, xmm0        
    
    lea rdi, [rel prefisso_float]  
    
    mov eax, 1                 
    
    sub rsp, 8                 ; Allinea lo stack
    call printf wrt ..plt      
    add rsp, 8                 ; Ripristina lo stack
    ret
section .text
    global kata_float_to_string:function

kata_float_to_string:
    ; CONTRATTO:
    ; xmm0 = Float da formattare (32 bit)
    ; rdi  = Puntatore al buffer di destinazione allocato dal chiamante
    ; RITORNA:
    ; rax  = Lunghezza della stringa generata (senza 0x00)

    ; 1. Prologo standard
    push rbp
    mov rbp, rsp

    ; Salviamo rdi (il buffer di destinazione) in un registro "callee-saved" 
    ; perché snprintf sovrascriverà rdi. Usiamo r12.
    ; pushiamo anche r13 solo per mantenere lo stack allineato a 16 byte!
    push r12
    push r13     
    mov r12, rdi

    ; 2. Allochiamo 32 byte sullo stack locale per il buffer temporaneo
    sub rsp, 32

    ; 3. Prepariamo gli argomenti per snprintf(buf, size, fmt, val)
    mov rdi, rsp               ; Arg 1: puntatore al buffer (il nostro stack)
    mov rsi, 32                ; Arg 2: dimensione del buffer temporaneo
    lea rdx, [rel fmt_float]   ; Arg 3: la stringa di formato "%g"
    cvtss2sd xmm0, xmm0        ; Arg 4: convertiamo il nostro float in double
    mov eax, 1                 ; Diciamo che c'è 1 parametro XMM
    
    ; snprintf scrive la stringa sullo stack, AGGIUNGE lo 0x00, ma
    ; RESTITUISCE in RAX la lunghezza SENZA lo 0x00! Una manna dal cielo.
    call snprintf wrt ..plt

    ; 4. Travaso dei dati (dal buffer stack temporaneo al buffer definitivo)
    ; Utilizziamo le istruzioni hardware x86 di copia della memoria (rep movsb)
    ; eax ora contiene la lunghezza. Se <= 0, qualcosa è andato storto.
    cmp eax, 0
    jle .cleanup

    mov rcx, rax               ; rcx = quanti byte copiare (la lunghezza di snprintf)
    mov rsi, rsp               ; rsi = sorgente (il buffer sullo stack)
    mov rdi, r12               ; rdi = destinazione (il buffer originale del chiamante)
    cld                        ; Assicuriamoci che la copia vada in avanti
    rep movsb                  ; COPIA MAGICAMENTE RCX BYTE DA RSI A RDI!

.cleanup:
    ; Il nostro buffer di destinazione ora ha la stringa pura, niente 0x00.
    ; RAX contiene ancora la lunghezza corretta da restituire al chiamante.
    
    ; 5. Ripuliamo lo stack e usciamo
    add rsp, 32
    pop r13
    pop r12
    pop rbp
    ret
section .text
    global kata_print_int:function
kata_print_int:
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