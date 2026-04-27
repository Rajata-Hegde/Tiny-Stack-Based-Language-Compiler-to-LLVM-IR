#include "compiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Function to read input from file
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Main entry point
int main(int argc, char* argv[]) {
    std::string input;
    
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        std::cerr << "Example: " << argv[0] << " program.sl\n";
        return 1;
    }
    
    // Read input from file
    input = readFile(argv[1]);
    if (input.empty()) {
        return 1;
    }
    
    std::cout << "==== Input Program ====\n";
    std::cout << input << "\n";
    std::cout << "\n==== Tokens ====\n";
    
    // Step 1: Tokenize (Lexer)
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();
    
    for (const auto& token : tokens) {
        std::cout << "Token: ";
        switch (token.type) {
            case TokenType::NUMBER: std::cout << "NUMBER"; break;
            case TokenType::IDENTIFIER: std::cout << "IDENTIFIER"; break;
            case TokenType::PLUS: std::cout << "PLUS"; break;
            case TokenType::MINUS: std::cout << "MINUS"; break;
            case TokenType::MUL: std::cout << "MUL"; break;
            case TokenType::DIV: std::cout << "DIV"; break;
            case TokenType::ASSIGN: std::cout << "ASSIGN"; break;
            case TokenType::GREATER: std::cout << "GREATER"; break;
            case TokenType::IF: std::cout << "IF"; break;
            case TokenType::ELSE: std::cout << "ELSE"; break;
            case TokenType::END: std::cout << "END"; break;
            case TokenType::EOF_TOKEN: std::cout << "EOF"; break;
        }
        std::cout << " = \"" << token.value << "\"\n";
    }
    
    std::cout << "\n==== LLVM IR Output ====\n";
    
    // Step 2 & 3: Parse and Generate LLVM IR (Parser + CodeGen)
    Parser parser(tokens);
    parser.parse();
    
    return 0;
}
