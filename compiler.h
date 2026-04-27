#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

// Token types for the language
enum class TokenType {
    NUMBER,      // Integer literals
    IDENTIFIER,  // Variable names
    PLUS,        // +
    MINUS,       // -
    MUL,         // *
    DIV,         // /
    ASSIGN,      // =
    GREATER,     // >
    IF,          // if
    ELSE,        // else
    END,         // end
    EOF_TOKEN    // End of file
};

// Token structure
struct Token {
    TokenType type;
    std::string value;
};

// Lexer class: converts input string into tokens
class Lexer {
public:
    explicit Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    std::string input;
    TokenType identifyKeywordOrOperator(const std::string& str);
};

// Parser class: processes tokens and generates LLVM IR
class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    void parse();

private:
    std::vector<Token> tokens;
    size_t currentIndex;
    
    Token currentToken();
    Token peekToken();
    void advance();
    void parseExpression();
    void parseIfElse();
    void parseAssignment();
};

#endif // COMPILER_H
