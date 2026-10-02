#pragma once
#include <optional>
#include <print>
#include <utility>
#include <vector>

#include "arena.h"
#include "sugar.h"
#include "tokenizer.h"

struct NodeExprIdent {
    Token identifier;
};
struct NodeExprIntLit {
    Token intLit;
};

struct BinExpr;

struct NodeExpr {
    Variant<NodeExprIdent*,NodeExprIntLit*,BinExpr*> var;
};

struct SumExpr {
    NodeExpr* left;
    NodeExpr* right;
};

struct ProdExpr {
    NodeExpr* left;
    NodeExpr* right;
};

struct BinExpr {
    Variant<SumExpr*,ProdExpr*> variant;
};


struct NodeExit {
    NodeExpr* expr;
};

struct NodeStmtExit {
    NodeExpr* expr;
};
struct NodeStmtLet {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    Variant<NodeStmtExit*,NodeStmtLet*> val;
};

struct NodeProg {
    Vec<NodeStmt*> stmts;
};

class Parser {
    int index {0};
    Vec<Token> tokens;
    ArenaAllocator allocator;

    Token consume() {
        print("Consuming: {}",tokenTypeToString(peek().value().type));
        return tokens.at(index++);
    }

    [[nodiscard]] Opt<Token> peek(const int offset = 0) const {
        if (index + offset < tokens.size())
            return tokens.at(index + offset);
        return {};
    }

    Deq<Token> infixToPostfixExpr() {
        Stack<Token> s;
        Deq<Token> postfix;

        while (peek().has_value()) {
            TokenType type = peek()->type;

            bool isExprToken = BIN_EXPR_TOKEN_PRIO.contains(type) ||
                               type == TokenType::OPEN_PAREN ||
                               type == TokenType::CLOSED_PAREN ||
                               type == TokenType::INT_LITERAL;

            if (!isExprToken) {
                break;
            }

            const Token token = consume();

            if (token.type == TokenType::INT_LITERAL) {
                postfix.push_back(token);
                continue;
            }

            if (token.type == TokenType::OPEN_PAREN) {
                s.push(token);
                continue;
            }

            if (token.type == TokenType::CLOSED_PAREN) {
                while (!s.empty() && s.top().type != TokenType::OPEN_PAREN) {
                    postfix.push_back(s.top());
                    s.pop();
                }
                if (!s.empty()) {
                    s.pop();
                }
                continue;
            }

            if (BIN_EXPR_TOKEN_PRIO.contains(token.type)) {
                while (!s.empty() && s.top().type != TokenType::OPEN_PAREN) {
                    if (BIN_EXPR_TOKEN_PRIO.at(s.top().type) >= BIN_EXPR_TOKEN_PRIO.at(token.type)) {
                        postfix.push_back(s.top());
                        s.pop();
                    } else break;
                }
                s.push(token);
            }
        }

        while (!s.empty()) {
            postfix.push_back(s.top());
            s.pop();
        }

        return postfix;
    }



    Opt<BinExpr*> parseBinaryExp() {
        if (!peek().has_value() || peek().value().type != TokenType::INT_LITERAL) return {};

        const Deq<Token> postfix = infixToPostfixExpr();

        Stack<Variant<Token,Variant<SumExpr*,ProdExpr*>>> stack;

        while (!postfix.empty()) {
            const Token& token = postfix.front();
            if (token.type == TokenType::INT_LITERAL) {
                stack.push(token);
                continue;
            }
            Variant<Token,Variant<SumExpr*,ProdExpr*>> left = stack.top();
            stack.pop();
            Variant<Token,Variant<SumExpr*,ProdExpr*>> right= stack.top();
            stack.pop();

            Variant<SumExpr*,ProdExpr*> variant {};
            if (token.type == TokenType::SUM) {
                auto sum = allocator.allocate<SumExpr>();
                const auto left_expr = allocator.allocate<NodeExpr>();
                const auto right_expr = allocator.allocate<NodeExpr>();
                auto left_expr_int_lit = allocator.allocate<NodeExprIntLit>();
                auto right_expr_int_lit = allocator.allocate<NodeExprIntLit>();
                left_expr_int_lit->intLit = left;
                right_expr_int_lit->intLit = right;
                left_expr->var = left_expr_int_lit;
                right_expr->var = right_expr_int_lit;
                sum->left = left_expr;
                sum->right = right_expr;
                variant = sum;

                stack.push(sum);

                continue;
            }
            if (token.type == TokenType::PRODUCT) {
                auto sum = allocator.allocate<ProdExpr>();
                const auto left_expr = allocator.allocate<NodeExpr>();
                const auto right_expr = allocator.allocate<NodeExpr>();
                auto left_expr_int_lit = allocator.allocate<NodeExprIntLit>();
                auto right_expr_int_lit = allocator.allocate<NodeExprIntLit>();
                left_expr_int_lit->intLit = left;
                right_expr_int_lit->intLit = right;
                left_expr->var = left_expr_int_lit;
                right_expr->var = right_expr_int_lit;
                sum->left = left_expr;
                sum->right = right_expr;
                variant = sum;
                continue;
            }

            print(stderr,"Unimplemented token: {}", tokenTypeToString(token.type));
            exit(1);

        }

    }

    Opt<NodeExpr*> parseExpr() {
        if (peek().has_value() && peek().value().type == TokenType::INT_LITERAL) {
            auto node_expr_int_lit = allocator.allocate<NodeExprIntLit>();
            node_expr_int_lit->intLit = consume();

            auto expr = allocator.allocate<NodeExpr>();
            expr->var = node_expr_int_lit;
            return expr;
        }
        if (peek().has_value() && peek().value().type == TokenType::IDENT) {

            auto node_expr_ident = allocator.allocate<NodeExprIdent>();
            node_expr_ident->identifier = consume();

            auto node_expr = allocator.allocate<NodeExpr>();
            node_expr->var = node_expr_ident;

            return node_expr;
        }
        return {};
    }


public:
    explicit Parser(Vec<Token> source) : tokens(std::move(source)),allocator(1024 * 1024 * 4){}

    Opt<NodeStmt*> parseStatement() {
        Opt<NodeStmt*> node;
        if (peek().value().type == TokenType::EXIT) {
            consume();
            if (peek().has_value() && peek().value().type == TokenType::OPEN_PAREN)
                consume();
            else {
                print(stderr,"Missing open paren for exit instruction!");
                exit(1);
            }
            if (const auto nodeExpr = parseExpr()) {
                auto node_stmt = allocator.allocate<NodeStmt>();
                auto node_stmt_exit = allocator.allocate<NodeStmtExit>();
                node_stmt_exit->expr = nodeExpr.value();
                node_stmt->val = node_stmt_exit;

                node = node_stmt;
            } else {
                print(stderr,"Invalid expression for exit node!");
                exit(1);
            }
            if (peek().has_value() && peek().value().type == TokenType::CLOSED_PAREN)
                consume();
            else {
                print(stderr,"Missing closed paren for exit instruction!");
                exit(1);
            }
            if (!peek().has_value() || peek().value().type != TokenType::SEMI){
                print(stderr,"Missing semi column for exit instruction!");
                exit(1);
            }
            consume();
        } else if (peek().has_value() && peek().value().type == TokenType::LET
            && peek(2).has_value() && peek(2).value().type == TokenType::EQUALS
            && peek(1).has_value() && peek(1).value().type == TokenType::IDENT) {
            consume();
            auto stmtLet = allocator.allocate<NodeStmtLet>();
            stmtLet->ident = consume();
            consume();
            if (const auto expr = parseExpr()) {
                stmtLet->expr = expr.value();
                const auto node_stmt = allocator.allocate<NodeStmt>();
                node_stmt->val = stmtLet;
                node = node_stmt;
            } else {
                print(stderr,"[Error] Let statement invalid expression.");
            }
            if (peek().has_value() && peek().value().type == TokenType::SEMI) consume();
            else {
                print(stderr,"[Error] Let statement without ending ';'");
            }
        }

        return node;
    }

    Opt<NodeProg*> parse() {
        Vec<NodeStmt*> stmts;
        while (peek().has_value()) {
            if (auto stmtOpt = parseStatement())
                stmts.push_back(stmtOpt.value());
            else {
                print(stderr,"[Error] invalid statement.");
                exit(1);
            }
        }
        const auto prog = allocator.allocate<NodeProg>();
        prog->stmts = stmts;
        return prog;
    }

};
