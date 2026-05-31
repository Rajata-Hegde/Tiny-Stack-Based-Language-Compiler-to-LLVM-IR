#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>

// ============================================================
// Token types for the enhanced stack-based language
// ============================================================
enum class TokenType {
    // Literals & Identifiers
    NUMBER,      // Integer or float literals (e.g., 42, 3.14)
    FLOAT,       // Floating-point literal
    IDENTIFIER,  // Variable names (e.g., x, myVar)

    // Arithmetic operators
    PLUS,        // +
    MINUS,       // -
    MUL,         // *
    DIV,         // /
    MOD,         // %

    // Assignment
    ASSIGN,      // =

    // Comparison operators
    GREATER,     // >
    LESS,        // <
    EQUAL,       // ==
    NOT_EQUAL,   // !=

    // Control flow
    IF,          // if
    ELSE,        // else
    END,         // end
    FOR,         // for loop
    REPEAT,      // repeat count times
    BREAK,       // break out of loop
    CONTINUE,    // continue to next iteration

    // I/O instructions
    PRINT,       // print  — pop TOS and print it
    INPUT,       // input  — read value from stdin, push onto stack
    PRINTLN,     // println — print with newline

    // Stack manipulation
    DUP,         // dup    — duplicate top of stack
    SWAP,        // swap   — swap the top two stack values
    NEG,         // neg    — negate top of stack
    DROP,        // drop   — discard top of stack

    // Type conversion
    F2I,         // float to int
    I2F,         // int to float

    // End of file
    EOF_TOKEN    // End of file sentinel
};

// ============================================================
// Token structure — now carries source location information
// ============================================================
struct Token {
    TokenType type;
    std::string value;
    int line;       // 1-indexed line number in source
    int column;     // 1-indexed column number in source

    Token() : type(TokenType::EOF_TOKEN), value(""), line(0), column(0) {}
    Token(TokenType t, const std::string& v, int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
};

// ============================================================
// Helper: human-readable token type name
// ============================================================
inline std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::NUMBER:    return "NUMBER";
        case TokenType::FLOAT:     return "FLOAT";
        case TokenType::IDENTIFIER:return "IDENTIFIER";
        case TokenType::PLUS:      return "PLUS (+)";
        case TokenType::MINUS:     return "MINUS (-)";
        case TokenType::MUL:       return "MUL (*)";
        case TokenType::DIV:       return "DIV (/)";
        case TokenType::MOD:       return "MOD (%)";
        case TokenType::ASSIGN:    return "ASSIGN (=)";
        case TokenType::GREATER:   return "GREATER (>)";
        case TokenType::LESS:      return "LESS (<)";
        case TokenType::EQUAL:     return "EQUAL (==)";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL (!=)";
        case TokenType::IF:        return "IF";
        case TokenType::ELSE:      return "ELSE";
        case TokenType::END:       return "END";
        case TokenType::FOR:       return "FOR";
        case TokenType::REPEAT:    return "REPEAT";
        case TokenType::BREAK:     return "BREAK";
        case TokenType::CONTINUE:  return "CONTINUE";
        case TokenType::PRINT:     return "PRINT";
        case TokenType::INPUT:     return "INPUT";
        case TokenType::PRINTLN:   return "PRINTLN";
        case TokenType::DUP:       return "DUP";
        case TokenType::SWAP:      return "SWAP";
        case TokenType::NEG:       return "NEG";
        case TokenType::DROP:      return "DROP";
        case TokenType::F2I:       return "F2I";
        case TokenType::I2F:       return "I2F";
        case TokenType::EOF_TOKEN: return "EOF";
        default:                   return "UNKNOWN";
    }
}

// ============================================================
// Lexer class: converts input string into tokens
// ============================================================
class Lexer {
public:
    explicit Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    std::string input;
    int line;
    int column;
    TokenType identifyKeywordOrOperator(const std::string& str);
};

// ============================================================
// Diagnostic — a single compiler error or warning
// ============================================================
struct Diagnostic {
    enum Level { ERROR, WARNING, NOTE };
    Level level;
    int line;
    int column;
    std::string message;

    std::string toString() const {
        std::string prefix;
        switch (level) {
            case ERROR:   prefix = "error"; break;
            case WARNING: prefix = "warning"; break;
            case NOTE:    prefix = "note"; break;
        }
        std::ostringstream oss;
        oss << prefix << " [" << line << ":" << column << "]: " << message;
        return oss.str();
    }
};

// ============================================================
// Validator class: pre-codegen semantic checks
// ============================================================
class Validator {
public:
    explicit Validator(const std::vector<Token>& tokens);
    bool validate();                              // returns true if valid
    const std::vector<Diagnostic>& diagnostics() const;

private:
    std::vector<Token> tokens;
    std::vector<Diagnostic> diags;
    size_t idx;

    Token current();
    void advance();

    void checkStackUnderflow(int required, const std::string& op, const Token& tok);
    int simulatedStackSize;  // tracks simulated stack depth

    void validateProgram();
    void validateInstruction();
    int ifDepth;             // tracks nested if/else/end balance
    int loopDepth;           // tracks nested for/repeat/end balance
};

// ============================================================
// Parser class: processes tokens and generates LLVM IR
//   Now supports error recovery and richer diagnostics
// ============================================================
class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    bool parse();  // returns false if errors encountered
    const std::vector<Diagnostic>& diagnostics() const;

private:
    std::vector<Token> tokens;
    size_t currentIndex;
    std::vector<Diagnostic> diags;
    bool hasErrors;

    Token currentToken();
    Token peekToken();
    void advance();
    void reportError(const Token& tok, const std::string& msg);
    void recoverToNext();  // skip tokens until a known instruction boundary

    void parseExpression();
    void parseIfElse();
    void parseForLoop();
    void parseRepeat();
};

#endif // COMPILER_H
