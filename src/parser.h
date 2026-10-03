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

struct NodeBinExpr;

struct NodeNegateExpr;
struct NodeExpr {
    Variant<NodeExprIdent*,NodeExprIntLit*,NodeBinExpr*,NodeNegateExpr*> var;
};

struct NodePowerExpr {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeIntDivExpr {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeSumExpr {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeSubExpr {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeProdExpr {
    NodeExpr* left;
    NodeExpr* right;
};

struct NodeNegateExpr {
    NodeExpr* expr;
};

struct NodeBinExpr {
    Variant<NodeSumExpr*,NodeProdExpr*,NodePowerExpr*,NodeSubExpr*,NodeIntDivExpr*> variant;
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
        int openParens = 0;
        int closedParens = 0;

        bool expectOperand {true};

        while (peek().has_value()) {
            TokenType type = peek()->type;

            bool isExprToken = BIN_EXPR_TOKEN_PRIO.contains(type) ||
                               type == TokenType::OPEN_PAREN ||
                               type == TokenType::CLOSED_PAREN ||
                               type == TokenType::IDENT ||
                               type == TokenType::INT_LITERAL;

            if (!isExprToken) {
                break;
            }

            if (type == TokenType::CLOSED_PAREN) {
                if (closedParens >= openParens
                    && peek(1).has_value()
                    && !BIN_EXPR_TOKEN_PRIO.contains(peek(1).value().type))
                    break;
                closedParens ++;
            }

            Token token = consume();

            if (token.type == TokenType::INT_LITERAL || token.type == TokenType::IDENT) {
                postfix.push_back(token);
                expectOperand = false;
                continue;
            }

            if (token.type == TokenType::OPEN_PAREN) {
                s.push(token);
                openParens ++;
                expectOperand = true;
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
                expectOperand = false;
                continue;
            }

            if (token.type == TokenType::MINUS && expectOperand) {
                token.type = TokenType::UNARY_MINUS;
                s.push(token);
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
                expectOperand = true;
            }
        }

        while (!s.empty()) {
            postfix.push_back(s.top());
            s.pop();
        }

        return postfix;
    }



    Opt<NodeExpr*> parseExpr() {
        if (!peek().has_value()
            || (peek().value().type != TokenType::INT_LITERAL && peek().value().type != TokenType::IDENT
                && peek().value().type != TokenType::MINUS && peek(1).has_value() && peek(1).value().type != TokenType::INT_LITERAL && peek(1).value().type != TokenType::IDENT)
            ) return {};

        print("============= Starting Expr =============");

        Deq<Token> postfix = infixToPostfixExpr();
        String vals;

        // print("Postfix: {}", vals.substr(0,vals.length() - 2));

        Stack<NodeExpr*> stack;

        while (!postfix.empty()) {
            vals.clear();
            for (auto p : postfix) {
                vals.append(tokenTypeToString(p.type)).append(", ");
            }
            print("Postfix: {}", vals.substr(0,vals.length() - 2));
            const Token token = std::move(postfix.front());
            postfix.pop_front();

            if (token.type == TokenType::INT_LITERAL) {
                auto node_expr_int_lit = allocator.allocate<NodeExprIntLit>();
                node_expr_int_lit->intLit = token;
                auto expr = allocator.allocate<NodeExpr>();
                expr->var = node_expr_int_lit;
                stack.push(expr);
                continue;
            }
            if (token.type == TokenType::IDENT) {
                auto node_expr_int_lit = allocator.allocate<NodeExprIdent>();
                node_expr_int_lit->identifier = token;
                auto expr = allocator.allocate<NodeExpr>();
                expr->var = node_expr_int_lit;
                stack.push(expr);
                continue;
            }

            const auto right = stack.top();
            stack.pop();

            if (token.type == TokenType::UNARY_MINUS) {
                auto expr = allocator.allocate<NodeExpr>();
                auto negate_expr = allocator.allocate<NodeNegateExpr>();
                negate_expr->expr = right;
                expr->var = negate_expr;
                stack.push(expr);
                continue;
            }


            const auto left = stack.top();
            stack.pop();

            if (token.type == TokenType::PLUS) {
                auto sum = allocator.allocate<NodeSumExpr>();
                sum->left = left;
                sum->right = right;

                auto bin_expr = allocator.allocate<NodeBinExpr>();
                bin_expr->variant = sum;
                auto expr = allocator.allocate<NodeExpr>();
                expr->var = bin_expr;

                stack.push(expr);
                continue;
            }
            if (token.type == TokenType::MINUS) {
                auto prod_expr = allocator.allocate<NodeSubExpr>();
                prod_expr->left = left;
                prod_expr->right = right;
                auto bin_expr = allocator.allocate<NodeBinExpr>();
                bin_expr->variant = prod_expr;
                auto expr = allocator.allocate<NodeExpr>();
                expr->var = bin_expr;
                stack.push(expr);
                continue;
            }
            if (token.type == TokenType::PRODUCT) {
                auto prod_expr = allocator.allocate<NodeProdExpr>();
                prod_expr->left = left;
                prod_expr->right = right;
                auto bin_expr = allocator.allocate<NodeBinExpr>();
                bin_expr->variant = prod_expr;
                auto expr = allocator.allocate<NodeExpr>();
                expr->var = bin_expr;
                stack.push(expr);
                continue;
            }
            if (token.type == TokenType::POWER) {
                auto prod_expr = allocator.allocate<NodePowerExpr>();
                prod_expr->left = left;
                prod_expr->right = right;
                auto bin_expr = allocator.allocate<NodeBinExpr>();
                bin_expr->variant = prod_expr;
                auto expr = allocator.allocate<NodeExpr>();
                expr->var = bin_expr;
                stack.push(expr);
                continue;
            }
            if (token.type == TokenType::DIVISION) {
                auto prod_expr = allocator.allocate<NodeIntDivExpr>();
                prod_expr->left = left;
                prod_expr->right = right;
                auto bin_expr = allocator.allocate<NodeBinExpr>();
                bin_expr->variant = prod_expr;
                auto expr = allocator.allocate<NodeExpr>();
                expr->var = bin_expr;
                stack.push(expr);
                continue;
            }

            print(stderr,"Unimplemented token: {}", tokenTypeToString(token.type));
            exit(1);

        }

        print("============= ENDING Expr =============");
        return stack.top();
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
                exit(1);
            }
            if (peek().has_value() && peek().value().type == TokenType::SEMI) consume();
            else {
                print(stderr,"[Error] Let statement without ending ';'");
                exit(1);
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
