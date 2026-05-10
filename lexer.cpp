#include "compiler.h"
#include <sstream>
#include <cctype>
#include <iostream>

// ============================================================
// Lexer implementation — enhanced with line/column tracking
// and recognition of new keywords & operators
// ============================================================

// Constructor: store input string, initialise position
Lexer::Lexer(const std::string& input)
    : input(input), line(1), column(1) {}

// Determine if a string is a keyword, operator, or identifier
TokenType Lexer::identifyKeywordOrOperator(const std::string& str) {
    // Single-char operators
    if (str == "+")  return TokenType::PLUS;
    if (str == "-")  return TokenType::MINUS;
    if (str == "*")  return TokenType::MUL;
    if (str == "/")  return TokenType::DIV;
    if (str == "%")  return TokenType::MOD;
    if (str == "=")  return TokenType::ASSIGN;
    if (str == ">")  return TokenType::GREATER;
    if (str == "<")  return TokenType::LESS;

    // Multi-char operators
    if (str == "==") return TokenType::EQUAL;
    if (str == "!=") return TokenType::NOT_EQUAL;

    // Keywords — control flow
    if (str == "if")    return TokenType::IF;
    if (str == "else")  return TokenType::ELSE;
    if (str == "end")   return TokenType::END;

    // Keywords — I/O
    if (str == "print") return TokenType::PRINT;
    if (str == "input") return TokenType::INPUT;

    // Keywords — stack manipulation
    if (str == "dup")   return TokenType::DUP;
    if (str == "swap")  return TokenType::SWAP;
    if (str == "neg")   return TokenType::NEG;

    // Anything else is an identifier (variable name)
    return TokenType::IDENTIFIER;
}

// Tokenize input: scan character-by-character, track line & column
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    size_t pos = 0;
    int curLine = 1;
    int curCol  = 1;

    while (pos < input.size()) {
        char ch = input[pos];

        // Skip whitespace, tracking line/column
        if (ch == '\n') {
            curLine++;
            curCol = 1;
            pos++;
            continue;
        }
        if (ch == '\r') {
            pos++;
            continue;  // handled with \n
        }
        if (std::isspace(ch)) {
            curCol++;
            pos++;
            continue;
        }

        // Record start position of this token
        int tokLine = curLine;
        int tokCol  = curCol;

        // --------------------------------------------------
        // Multi-char operators: ==, !=
        // --------------------------------------------------
        if (ch == '=' && pos + 1 < input.size() && input[pos + 1] == '=') {
            tokens.push_back(Token(TokenType::EQUAL, "==", tokLine, tokCol));
            pos += 2;
            curCol += 2;
            continue;
        }
        if (ch == '!' && pos + 1 < input.size() && input[pos + 1] == '=') {
            tokens.push_back(Token(TokenType::NOT_EQUAL, "!=", tokLine, tokCol));
            pos += 2;
            curCol += 2;
            continue;
        }

        // --------------------------------------------------
        // Single-char operators
        // --------------------------------------------------
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
            ch == '%' || ch == '=' || ch == '>' || ch == '<') {
            std::string s(1, ch);
            tokens.push_back(Token(identifyKeywordOrOperator(s), s, tokLine, tokCol));
            pos++;
            curCol++;
            continue;
        }

        // --------------------------------------------------
        // Numbers (integer literals, possibly negative via NEG)
        // --------------------------------------------------
        if (std::isdigit(ch)) {
            std::string numStr;
            while (pos < input.size() && std::isdigit(input[pos])) {
                numStr += input[pos];
                pos++;
                curCol++;
            }
            tokens.push_back(Token(TokenType::NUMBER, numStr, tokLine, tokCol));
            continue;
        }

        // --------------------------------------------------
        // Identifiers and keywords
        // --------------------------------------------------
        if (std::isalpha(ch) || ch == '_') {
            std::string word;
            while (pos < input.size() && (std::isalnum(input[pos]) || input[pos] == '_')) {
                word += input[pos];
                pos++;
                curCol++;
            }
            tokens.push_back(Token(identifyKeywordOrOperator(word), word, tokLine, tokCol));
            continue;
        }

        // --------------------------------------------------
        // Unknown character — emit error token and skip
        // --------------------------------------------------
        std::cerr << "warning [" << curLine << ":" << curCol
                  << "]: Unrecognized character '" << ch << "', skipping.\n";
        pos++;
        curCol++;
    }

    // Add EOF token
    tokens.push_back(Token(TokenType::EOF_TOKEN, "", curLine, curCol));

    return tokens;
}
