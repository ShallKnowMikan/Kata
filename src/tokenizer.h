#pragma once
#include <string>
#include <vector>
#include <utility>

enum class TokenType {
    EXIT,
    INT_LITERAL,
    SEMI,
    OPEN_PAREN,
    CLOSED_PAREN,
    IDENT,
    LET,
    EQUALS,
    PLUS,
    MINUS,
    PRODUCT,
    DIVISION,
    POWER,
    UNARY_MINUS,
    PRINT,
    STRING_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    TYPE_DEF,
};

inline HashSet native_types = {
    TokenType::STRING_TYPE,
    TokenType::INT_TYPE,
    TokenType::FLOAT_TYPE,
};
inline HashMap<TokenType,int> BIN_EXPR_TOKEN_PRIO = {
    {TokenType::PLUS, 0},
    {TokenType::MINUS, 0},
    {TokenType::DIVISION, 1},
    {TokenType::PRODUCT, 1},
    {TokenType::POWER, 2},
    {TokenType::UNARY_MINUS, 3},
};

inline std::string tokenTypeToString(const TokenType type) {
    switch (type) {
        case TokenType::EXIT: return "EXIT";
        case TokenType::INT_LITERAL : return "INT_LITERAL";
        case TokenType::SEMI: return "SEMI";
        case TokenType::OPEN_PAREN: return "OPEN_PAREN";
        case TokenType::CLOSED_PAREN: return "CLOSED_PAREN";
        case TokenType::PLUS: return "SUM";
        case TokenType::MINUS: return "SUB";
        case TokenType::IDENT: return "IDENT";
        case TokenType::LET: return "LET";
        case TokenType::EQUALS: return "EQUALS";
        case TokenType::PRODUCT: return "PRODUCT";
        case TokenType::DIVISION: return "DIVISION";
        case TokenType::POWER: return "POWER";
        case TokenType::UNARY_MINUS: return "UNARY_MINUS";
        case TokenType::PRINT: return "PRINT";
        case TokenType::STRING_TYPE: return "STRING_TYPE";
        case TokenType::INT_TYPE: return "INT_TYPE";
        case TokenType::FLOAT_TYPE: return "FLOAT_TYPE";
        case TokenType::TYPE_DEF: return "TYPE_DEF";
    };
    return "INVALID TYPE";
}

struct Token {
    TokenType type;
    Opt<String> value {};
};

class Tokenizer {
    int index = 0;
    const std::string content;

    char consume() {
        return content.at(index++);
    }

    [[nodiscard]] std::optional<char> peek(const int offset = 0) const {
        if (index + offset < content.length())
            return content.at(index + offset);
        return {};
    }

public:
    explicit Tokenizer(std::string content) : content(std::move(content)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        std::string buffer;
        while (peek().has_value()) {
            if (isalpha(peek().value())) {
                while (isalnum(peek().value())) {
                    buffer.push_back(consume());
                }

                if (buffer == "exit") {
                    tokens.push_back({.type = TokenType::EXIT});
                    buffer.clear();
                    continue;
                }
                if (buffer == "let") {
                    tokens.push_back({.type = TokenType::LET});
                    buffer.clear();
                    continue;
                }
                if (buffer == "print") {
                    tokens.push_back({.type = TokenType::PRINT});
                    buffer.clear();
                    continue;
                }
                if (buffer == "string") {
                    tokens.push_back({.type = TokenType::STRING_TYPE});
                    if (tokens.size() < 2 || tokens.at(tokens.size() - 2).type != TokenType::TYPE_DEF) {
                        print("[Error] Incorrect type definition, must use ':' before declaring types.");
                        exit(1);
                    }
                    buffer.clear();
                    continue;
                }
                if (buffer == "string" || buffer == "int" || buffer == "float") {
                    tokens.push_back({.type = TokenType::STRING_TYPE});
                    if (tokens.size() < 2 || tokens.at(tokens.size() - 2).type != TokenType::TYPE_DEF) {
                        print("[Error] Incorrect type definition, must use ':' before declaring types.");
                        exit(1);
                    }
                    buffer.clear();
                    continue;
                }
                tokens.push_back({.type = TokenType::IDENT, .value = buffer});
                buffer.clear();

            } else if (isdigit(peek().value())) {
                while (isdigit(peek().value())) {
                    buffer.push_back(consume());
                }
                tokens.push_back({.type = TokenType::INT_LITERAL,.value = buffer});
                buffer.clear();
                continue;
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::SEMI});
                continue;
            } else if (peek().value() == ':') {
                consume();
                tokens.push_back({.type = TokenType::TYPE_DEF});
                if (tokens.size() < 2 || tokens.at(tokens.size() - 2).type != TokenType::IDENT) {
                    print("[Error] Cannot incorrect type definition.");
                    exit(1);
                }
                continue;
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::OPEN_PAREN});
                continue;
            } else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::CLOSED_PAREN});
                continue;
            } else if (peek().value() == '+') {
                consume();
                tokens.push_back({.type = TokenType::PLUS});
                continue;
            } else if (peek().value() == '-') {
                consume();
                tokens.push_back({.type = TokenType::MINUS});
                continue;
            } else if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '*') {
                consume();
                consume();
                tokens.push_back({.type = TokenType::POWER});
                continue;
            } else if (peek().value() == '*') {
                consume();
                tokens.push_back({.type = TokenType::PRODUCT});
                continue;
            } else if (peek().value() == '/') {
                consume();
                tokens.push_back({.type = TokenType::DIVISION});
                continue;
            } else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::EQUALS});
                if (tokens.size() < 2 || tokens.at(tokens.size() - 2).type != TokenType::IDENT || (tokens.size() >= 3 && tokens.at(tokens.size() - 3).type != TokenType::TYPE_DEF && tokens.at(tokens.size() - 3).type != TokenType::LET  && !native_types.contains(tokens.at(tokens.size() - 2).type))) {
                    print("[Error] Invalid assignment.");
                    // let x: string = "";
                    exit(1);
                }
                continue;
            } else if (isspace(peek().value())) {
                consume();
                continue;
            } else {
                std::println(stderr,"Error -> buffer: {} | index: {}",buffer,index);
                exit(1);
            }
        }
        index = 0;
        return tokens;
    }

};
