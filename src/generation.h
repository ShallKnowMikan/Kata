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

                        gen->pop("rax");
                        gen->pop("rbx");
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

        return out.str();
    }
};
