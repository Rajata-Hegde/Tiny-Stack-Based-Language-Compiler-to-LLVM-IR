#include "compiler.h"
#include <sstream>
#include <cctype>
#include <iostream>

// Constructor: store input string
Lexer::Lexer(const std::string& input) : input(input) {}

// Determine if a string is a keyword or operator
TokenType Lexer::identifyKeywordOrOperator(const std::string& str) {
    if (str == "+") return TokenType::PLUS;
    if (str == "-") return TokenType::MINUS;
    if (str == "*") return TokenType::MUL;
    if (str == "/") return TokenType::DIV;
    if (str == "=") return TokenType::ASSIGN;
    if (str == ">") return TokenType::GREATER;
    if (str == "if") return TokenType::IF;
    if (str == "else") return TokenType::ELSE;
    if (str == "end") return TokenType::END;
    
    return TokenType::IDENTIFIER;
}

// Tokenize input: split by spaces and classify each token
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    std::istringstream stream(input);
    std::string word;

    while (stream >> word) {
        Token token;
        
        // Check if the word is a number
        bool isNumber = !word.empty() && std::isdigit(word[0]);
        if (isNumber) {
            for (char c : word) {
                if (!std::isdigit(c)) {
                    isNumber = false;
                    break;
                }
            }
        }
        
        if (isNumber) {
            token.type = TokenType::NUMBER;
            token.value = word;
        } else {
            token.type = identifyKeywordOrOperator(word);
            token.value = word;
        }
        
        tokens.push_back(token);
    }
    
    // Add EOF token
    tokens.push_back({TokenType::EOF_TOKEN, ""});
    
    return tokens;
}
