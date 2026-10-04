#pragma once
#include <sstream>
#include <utility>

#include "parser.h"
using String = std::string;

class Generator {
    struct Var {
         size_t stackLocation;
    };
    NodeProg* root;
    std::stringstream out;
    size_t stackOffset {0};

    HashMap<String, Var> variables {};

    void push(const String& reg,const int tabs = 0) {
        String padding;
        for (int i = 0; i < tabs; i++) {
            padding.append("    ");
        }
        out << padding << "    push " << reg << "\n";
        stackOffset++;
    }

    void pop(const String& reg) {
        out << "    pop " << reg << "\n";
        stackOffset--;
    }

public:
    explicit Generator(NodeProg* root) : root(root) {}

    void generateExpr(const NodeExpr* nodeExpr) {
        struct ExprVisitor {
            Generator* gen;
            explicit ExprVisitor(Generator* gen) :  gen(gen) {}

            void operator()(const NodeExprIntLit* intLit) const {
                print("Int literal");
                gen->out << "    mov rax, " << intLit->intLit.value.value() << "\n";
                gen->push("rax");
            }
            void operator()(const NodeExprIdent* ident) const {
                const String& varName = ident->identifier.value.value();
                if (!gen->variables.contains(varName)) {
                    print("[Error] Invalid or undeclared identifier: {}",varName);
                    exit(1);
                }
                const auto&[stackLocation] = gen->variables.at(varName);
                std::stringstream offset;
                offset << "[rsp + " << (gen->stackOffset - stackLocation - 1) * 8 << "]\n";
                gen->push(offset.str());
            }
            void operator()(const NodeNegateExpr* expr) const {
                print("Negation");
                gen->generateExpr(expr->expr);
                gen->pop("rax");
                gen->out << "    imul rax, -1\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExpr* bin_expr) const {
                struct BinExprVisitor {
                    Generator* gen;
                    explicit BinExprVisitor(Generator* gen) :  gen(gen) {}
                    void operator()(const NodeSumExpr* expr) const {
                        print("SUM");
                        gen->generateExpr(expr->left);
                        gen->generateExpr(expr->right);

                        gen->pop("rax");
                        gen->pop("rbx");
                        gen->out << "    add rax, rbx\n";
                        gen->push("rax");
                    }
                    void operator()(const NodeSubExpr* expr) const {
                        gen->generateExpr(expr->left);
                        gen->generateExpr(expr->right);

                        gen->pop("rbx");
                        gen->pop("rax");
                        gen->out << "    sub rax, rbx\n";
                        gen->push("rax");
                    }
                    void operator()(const NodeProdExpr* expr) const {
                        print("PRODUCT");
                        gen->generateExpr(expr->left);
                        gen->generateExpr(expr->right);

                        gen->pop("rax");
                        gen->pop("rbx");
                        gen->out << "    imul rax, rbx\n";
                        gen->push("rax");
                    }
                    void operator()(const NodePowerExpr* expr) const {
                        gen->generateExpr(expr->left);
                        gen->generateExpr(expr->right);

                        gen->pop("rcx"); // counter
                        gen->pop("rbx");

                        static int id = 0;
                        id++;

                        gen->out << "    mov rax, 1\n";
                        gen->out << "    cmp rcx, 0\n";
                        gen->out << "    je .power_res_"<< id <<"\n";
                        gen->out << "    .power_loop_"<< id <<":\n";
                        gen->out << "        imul rax, rbx\n";
                        gen->out << "        dec rcx\n";
                        gen->out << "        jnz .power_loop_"<< id <<"\n";

                        gen->out << "    .power_res_"<< id <<":\n";
                        gen->push("rax",1);
                    }

                    void operator()(const NodeIntDivExpr* expr) const {
                        gen->generateExpr(expr->left);
                        gen->generateExpr(expr->right);

                        gen->pop("rbx");
                        gen->pop("rax");

                        gen->out << "    cqo\n";
                        gen->out << "    idiv rbx\n";

                        gen->push("rax");
                    }
                };
                BinExprVisitor visitor {gen};
                std::visit(visitor,bin_expr->variant);
            }
        };

        ExprVisitor visitor(this);
        std::visit(visitor,nodeExpr->var);
    }

    void generateStmt(const NodeStmt* node) {
        struct StmtVisitor {
            Generator* gen;
            explicit StmtVisitor(Generator* gen) :  gen(gen) {}

            void operator()(const NodeStmtExit* exitNode) const {
                gen->generateExpr(exitNode->expr);
                gen->out << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->out << "    syscall\n";
            }
            void operator()(const NodeStmtLet* letNode) const {
                const String& varName = letNode->ident.value.value();
                if (gen->variables.contains(varName)) {
                    print(stderr,"[Error] Variable: {} is already declared in this scope!",varName);
                    exit(1);
                }

                gen->variables[varName] = Var{.stackLocation = gen->stackOffset};
                gen->generateExpr(letNode->expr);

            }
            void operator()(const NodeStmtPrint* printNode) const {
                gen->generateExpr(printNode->expr);

                gen->pop("rdi");
                gen->out << "    call print_int\n";
            }
        };
        StmtVisitor visitor(this);
        std::visit(visitor,node->val);

    }

     [[nodiscard]] String generate() {
        out << "section .text\n";
        out << "global _start\n_start:\n";

        for (const auto & stmt : root->stmts) {
            generateStmt(stmt);
        }

        out << "    mov rax, 60\n";
        out << "    mov rdi, 0\n";
        out << "    syscall";

        out << R"(
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
)";

        return out.str();
    }
};
