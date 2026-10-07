#pragma once
#include <optional>

#include <vector>

#include "arena.h"
#include "sugar.h"
#include "tokenizer.h"
#include "ast.h"

class Parser {
    int index {0};
    Vec<Token> tokens;
    ArenaAllocator allocator;

public:
    explicit Parser(Vec<Token> source);
    Opt<NodeStmt*> parseStatement();
    NodeStmt* parseExitNode();
    NodeStmt* parseLetNode();
    NodeStmt* parsePrintNode();

    Deq<Token> infixToPostfixExpr();
    Opt<NodeProg*> parse();

private:
    Token consume();
    [[nodiscard]] Opt<Token> peek(const int offset = 0) const;


    Opt<NodeExpr*> parseExpr();
};