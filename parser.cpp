#include "compiler.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include <iostream>
#include <stack>
#include <map>

// Forward declarations for codegen functions
extern void initializeCodegen();
extern llvm::Function* createMainFunction();
extern void emitReturn();
extern void emitPushConstant(int value);
extern llvm::Value* getVariable(const std::string& name);
extern void setVariable(const std::string& name, llvm::Value* value);
extern void emitAdd();
extern void emitSub();
extern void emitMul();
extern void emitDiv();
extern void emitGreater();
extern void printIR();
extern llvm::Module* getModule();
extern llvm::IRBuilder<>* getBuilder();
extern std::stack<llvm::Value*>* getValueStack();
extern std::map<std::string, llvm::Value*>* getVariablesMap();

// Parser constructor
Parser::Parser(const std::vector<Token>& tokens) 
    : tokens(tokens), currentIndex(0) {}

// Get current token
Token Parser::currentToken() {
    if (currentIndex < tokens.size()) {
        return tokens[currentIndex];
    }
    return {TokenType::EOF_TOKEN, ""};
}

// Peek at next token without consuming
Token Parser::peekToken() {
    if (currentIndex + 1 < tokens.size()) {
        return tokens[currentIndex + 1];
    }
    return {TokenType::EOF_TOKEN, ""};
}

// Move to next token
void Parser::advance() {
    if (currentIndex < tokens.size()) {
        currentIndex++;
    }
}

// Parse assignment: x 10 =
void Parser::parseAssignment() {
    auto valueStack = getValueStack();
    auto varMap = getVariablesMap();
    
    if (valueStack->size() < 2) {
        std::cerr << "Error: Not enough values for assignment\n";
        return;
    }
    
    llvm::Value* value = valueStack->top(); 
    valueStack->pop();
    
    // The identifier should still be on stack or we need to track it differently
    // For now, we'll expect: identifier value =
    // We pop value, but identifier is already processed as a string
    
    // This is a simplified version - in real implementation would need to track identifier
    std::cerr << "Assignment parsing simplified for this version\n";
}

// Parse if-else block
void Parser::parseIfElse() {
    auto builder = getBuilder();
    auto valueStack = getValueStack();
    
    // Pop condition from stack
    if (valueStack->empty()) {
        std::cerr << "Error: No condition for if statement\n";
        return;
    }
    
    llvm::Value* cond = valueStack->top();
    valueStack->pop();
    
    // Get the current function
    llvm::BasicBlock* currentBlock = builder->GetInsertBlock();
    llvm::Function* func = currentBlock->getParent();
    
    // Create basic blocks for if/else
    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(
        builder->getContext(), "then", func
    );
    llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(
        builder->getContext(), "else", func
    );
    llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(
        builder->getContext(), "end_if", func
    );
    
    // Create conditional branch
    builder->CreateCondBr(cond, thenBlock, elseBlock);
    
    // Process then block
    builder->SetInsertPoint(thenBlock);
    
    // Skip ahead to process then body (simplified - in real parser would recursively parse)
    advance(); // skip 'if'
    while (currentIndex < tokens.size() && 
           tokens[currentIndex].type != TokenType::ELSE && 
           tokens[currentIndex].type != TokenType::END) {
        parseExpression();
    }
    
    // Jump to end if no explicit return
    if (builder->GetInsertBlock()->getTerminator() == nullptr) {
        builder->CreateBr(endBlock);
    }
    
    // Process else block
    builder->SetInsertPoint(elseBlock);
    
    if (currentToken().type == TokenType::ELSE) {
        advance(); // skip 'else'
        while (currentIndex < tokens.size() && 
               tokens[currentIndex].type != TokenType::END) {
            parseExpression();
        }
    }
    
    // Jump to end
    if (builder->GetInsertBlock()->getTerminator() == nullptr) {
        builder->CreateBr(endBlock);
    }
    
    // Continue at end block
    builder->SetInsertPoint(endBlock);
    
    if (currentToken().type == TokenType::END) {
        advance(); // skip 'end'
    }
}

// Parse expressions: numbers, operations, variables
void Parser::parseExpression() {
    Token token = currentToken();
    auto valueStack = getValueStack();
    auto varMap = getVariablesMap();
    auto builder = getBuilder();
    
    switch (token.type) {
        case TokenType::NUMBER: {
            int value = std::stoi(token.value);
            emitPushConstant(value);
            advance();
            break;
        }
        
        case TokenType::IDENTIFIER: {
            std::string varName = token.value;
            advance();
            
            // Check if next token is assignment
            if (currentToken().type == TokenType::ASSIGN) {
                advance(); // skip '='
                if (!valueStack->empty()) {
                    llvm::Value* value = valueStack->top();
                    valueStack->pop();
                    setVariable(varName, value);
                }
            } else {
                // Load variable value
                llvm::Value* val = getVariable(varName);
                valueStack->push(val);
            }
            break;
        }
        
        case TokenType::PLUS: {
            emitAdd();
            advance();
            break;
        }
        
        case TokenType::MINUS: {
            emitSub();
            advance();
            break;
        }
        
        case TokenType::MUL: {
            emitMul();
            advance();
            break;
        }
        
        case TokenType::DIV: {
            emitDiv();
            advance();
            break;
        }
        
        case TokenType::GREATER: {
            emitGreater();
            advance();
            break;
        }
        
        case TokenType::IF: {
            parseIfElse();
            break;
        }
        
        case TokenType::ELSE:
        case TokenType::END:
        case TokenType::EOF_TOKEN: {
            // End of current block
            break;
        }
        
        default:
            advance();
            break;
    }
}

// Main parse function
void Parser::parse() {
    // Initialize LLVM components
    initializeCodegen();
    
    // Create main function
    llvm::Function* mainFunc = createMainFunction();
    
    // Parse all tokens
    while (currentIndex < tokens.size() && 
           currentToken().type != TokenType::EOF_TOKEN) {
        parseExpression();
    }
    
    // Emit return statement
    emitReturn();
    
    // Print generated IR
    printIR();
}
