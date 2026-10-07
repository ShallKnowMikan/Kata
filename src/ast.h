#pragma once
#include "tokenizer.h"
#include "sugar.h"

struct NodeExprIdent;
struct NodeExprIntLit;
struct NodeBinExpr;
struct NodeNegateExpr;
struct NodePowerExpr;
struct NodeIntDivExpr;
struct NodeSumExpr;
struct NodeSubExpr;
struct NodeProdExpr;

struct NodeExprIdent {
    Token identifier;
};

struct NodeExprIntLit {
    Token intLit;
};

struct NodeExpr {
    Variant<NodeExprIdent*, NodeExprIntLit*, NodeBinExpr*, NodeNegateExpr*> var;
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
    Variant<NodeSumExpr*, NodeProdExpr*, NodePowerExpr*, NodeSubExpr*, NodeIntDivExpr*> variant;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr* expr {nullptr};
};

struct NodeStmtPrint {
    NodeExpr* expr;
};

struct NodeStmt {
    Variant<NodeStmtExit*, NodeStmtLet*, NodeStmtPrint*> val;
};

struct NodeProg {
    Vec<NodeStmt*> stmts;
};