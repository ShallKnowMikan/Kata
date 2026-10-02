#pragma once
#include <assert.h>
#include <sstream>
#include <utility>

#include "parser.h"
using String = std::string;


class Generator {
    struct Var {
         size_t stackLocation;
    };
    NodeProg* root;
    std::stringstream output;
    size_t stackOffset {0};

    HashMap<String, Var> variables {};

    void push(const String& reg) {
        output << "    push " << reg << "\n";
        stackOffset++;
    }

    void pop(const String& reg) {
        output << "    pop " << reg << "\n";
        stackOffset--;
    }

public:
    explicit Generator(NodeProg* root) : root(root) {}

    void generateExpr(const NodeExpr* nodeExpr) {
        struct ExprVisitor {
            Generator* gen;
            explicit ExprVisitor(Generator* gen) :  gen(gen) {}
            void operator()(const NodeExprIntLit* intLit) const {
                gen->output << "    mov rax, " << intLit->intLit.value.value() << "\n";
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
            void operator()(const BinExpr* bin_expr) const {
                assert(false); // Not implemented
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
                gen->output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->output << "    syscall\n";
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
        output << "section .text\n";
        output << "global _start\n_start:\n";

        for (const auto & stmt : root->stmts) {
            generateStmt(stmt);
        }

        output << "    mov rax, 60\n";
        output << "    mov rdi, 0\n";
        output << "    syscall";

        return output.str();
    }
};
