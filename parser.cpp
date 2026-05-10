#include "compiler.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include <iostream>
#include <stack>
#include <map>

// ============================================================
// Forward declarations for codegen functions
// ============================================================
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
extern void emitMod();
extern void emitGreater();
extern void emitLess();
extern void emitEqual();
extern void emitNotEqual();
extern void emitDup();
extern void emitSwap();
extern void emitNeg();
extern void emitPrint();
extern void emitInput();
extern void printIR();
extern bool verifyIR();
extern llvm::Module* getModule();
extern llvm::IRBuilder<>* getBuilder();
extern std::stack<llvm::Value*>* getValueStack();
extern std::map<std::string, llvm::Value*>* getVariablesMap();
extern llvm::LLVMContext* getLLVMContext();

// ============================================================
// Parser constructor
// ============================================================
Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), currentIndex(0), hasErrors(false) {}

// Get current token
Token Parser::currentToken() {
    if (currentIndex < tokens.size()) {
        return tokens[currentIndex];
    }
    return Token(TokenType::EOF_TOKEN, "", 0, 0);
}

// Peek at next token without consuming
Token Parser::peekToken() {
    if (currentIndex + 1 < tokens.size()) {
        return tokens[currentIndex + 1];
    }
    return Token(TokenType::EOF_TOKEN, "", 0, 0);
}

// Move to next token
void Parser::advance() {
    if (currentIndex < tokens.size()) {
        currentIndex++;
    }
}

// Report an error with source location
void Parser::reportError(const Token& tok, const std::string& msg) {
    diags.push_back({
        Diagnostic::ERROR, tok.line, tok.column,
        msg + " (got '" + tok.value + "', token type: " + tokenTypeName(tok.type) + ")"
    });
    hasErrors = true;
}

// Error recovery: skip tokens until a known instruction boundary
void Parser::recoverToNext() {
    while (currentIndex < tokens.size()) {
        TokenType t = tokens[currentIndex].type;
        // Stop at tokens that can start a new instruction
        if (t == TokenType::NUMBER || t == TokenType::IDENTIFIER ||
            t == TokenType::IF || t == TokenType::PRINT ||
            t == TokenType::INPUT || t == TokenType::DUP ||
            t == TokenType::SWAP || t == TokenType::NEG ||
            t == TokenType::EOF_TOKEN) {
            break;
        }
        currentIndex++;
    }
}

const std::vector<Diagnostic>& Parser::diagnostics() const {
    return diags;
}

// ============================================================
// Static variable to track last identifier for assignment
// ============================================================
static std::string lastIdentifier = "";

// ============================================================
// Parse if-else block with proper PHI merging
// ============================================================
void Parser::parseIfElse() {
    auto builder    = getBuilder();
    auto valueStack = getValueStack();

    // Pop condition from stack
    if (valueStack->empty()) {
        reportError(currentToken(), "No condition value on stack for 'if'");
        recoverToNext();
        return;
    }

    llvm::Value* cond = valueStack->top();
    valueStack->pop();

    // Get the current function
    llvm::BasicBlock* currentBlock = builder->GetInsertBlock();
    llvm::Function* func = currentBlock->getParent();

    // Create basic blocks for if/else/end
    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(
        builder->getContext(), "then", func
    );
    llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(
        builder->getContext(), "else", func
    );
    llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(
        builder->getContext(), "end_if", func
    );

    // Create conditional branch from entry to then/else
    builder->CreateCondBr(cond, thenBlock, elseBlock);

    // ===== THEN BLOCK =====
    builder->SetInsertPoint(thenBlock);
    advance(); // skip 'if'

    // Parse all operations in then block until we hit else/end
    while (currentIndex < tokens.size() &&
           tokens[currentIndex].type != TokenType::ELSE &&
           tokens[currentIndex].type != TokenType::END &&
           tokens[currentIndex].type != TokenType::EOF_TOKEN) {
        parseExpression();
    }

    // Capture the result from then block
    llvm::Value* thenResult = nullptr;
    if (!valueStack->empty()) {
        thenResult = valueStack->top();
        valueStack->pop();
    }

    // Get the block we're currently in (might have changed during parsing)
    llvm::BasicBlock* thenBlockEnd = builder->GetInsertBlock();

    // Branch to end if no terminator exists
    if (thenBlockEnd->getTerminator() == nullptr) {
        builder->CreateBr(endBlock);
    }

    // ===== ELSE BLOCK =====
    builder->SetInsertPoint(elseBlock);

    llvm::Value* elseResult = nullptr;
    if (currentToken().type == TokenType::ELSE) {
        advance(); // skip 'else'

        // Parse all operations in else block until we hit end
        while (currentIndex < tokens.size() &&
               tokens[currentIndex].type != TokenType::END &&
               tokens[currentIndex].type != TokenType::EOF_TOKEN) {
            parseExpression();
        }

        // Capture the result from else block
        if (!valueStack->empty()) {
            elseResult = valueStack->top();
            valueStack->pop();
        }
    }

    // Get the block we're currently in
    llvm::BasicBlock* elseBlockEnd = builder->GetInsertBlock();

    // Branch to end if no terminator exists
    if (elseBlockEnd->getTerminator() == nullptr) {
        builder->CreateBr(endBlock);
    }

    // ===== END BLOCK (PHI MERGE) =====
    builder->SetInsertPoint(endBlock);

    // Create phi node to merge results from then and else
    if (thenResult != nullptr && elseResult != nullptr) {
        // Both branches have results — merge with phi
        llvm::Type* resultType = thenResult->getType();
        llvm::PHINode* phiNode = builder->CreatePHI(resultType, 2, "if_result");
        phiNode->addIncoming(thenResult, thenBlockEnd);
        phiNode->addIncoming(elseResult, elseBlockEnd);
        valueStack->push(phiNode);
    } else if (thenResult != nullptr) {
        // Only then has result
        valueStack->push(thenResult);
    } else if (elseResult != nullptr) {
        // Only else has result
        valueStack->push(elseResult);
    }

    // Skip 'end' token
    if (currentToken().type == TokenType::END) {
        advance();
    } else {
        reportError(currentToken(), "Expected 'end' to close 'if' block");
    }
}

// ============================================================
// Parse a single expression / instruction
// ============================================================
void Parser::parseExpression() {
    Token token = currentToken();
    auto valueStack = getValueStack();
    auto varMap     = getVariablesMap();
    auto builder    = getBuilder();

    switch (token.type) {
        case TokenType::NUMBER: {
            int value = std::stoi(token.value);
            emitPushConstant(value);
            advance();
            break;
        }

        case TokenType::IDENTIFIER: {
            std::string varName = token.value;
            lastIdentifier = varName;  // Store for potential assignment
            advance();

            // Check if next token is assignment
            if (currentToken().type == TokenType::ASSIGN) {
                // Pop value from stack and assign to variable
                if (!valueStack->empty()) {
                    llvm::Value* value = valueStack->top();
                    valueStack->pop();
                    setVariable(varName, value);
                } else {
                    reportError(token, "No value on stack for assignment to '" + varName + "'");
                }
                advance(); // skip '='
            } else {
                // Load variable value
                llvm::Value* val = getVariable(varName);
                valueStack->push(val);
            }
            break;
        }

        case TokenType::PLUS:    { emitAdd();      advance(); break; }
        case TokenType::MINUS:   { emitSub();      advance(); break; }
        case TokenType::MUL:     { emitMul();      advance(); break; }
        case TokenType::DIV:     { emitDiv();      advance(); break; }
        case TokenType::MOD:     { emitMod();      advance(); break; }
        case TokenType::GREATER: { emitGreater();  advance(); break; }
        case TokenType::LESS:    { emitLess();     advance(); break; }
        case TokenType::EQUAL:   { emitEqual();    advance(); break; }
        case TokenType::NOT_EQUAL:{ emitNotEqual(); advance(); break; }

        case TokenType::DUP:     { emitDup();      advance(); break; }
        case TokenType::SWAP:    { emitSwap();     advance(); break; }
        case TokenType::NEG:     { emitNeg();      advance(); break; }

        case TokenType::PRINT:   { emitPrint();    advance(); break; }
        case TokenType::INPUT:   { emitInput();    advance(); break; }

        case TokenType::IF: {
            parseIfElse();
            break;
        }

        case TokenType::ASSIGN: {
            // Bare '=' without identifier context — use lastIdentifier
            if (!lastIdentifier.empty() && !valueStack->empty()) {
                llvm::Value* value = valueStack->top();
                valueStack->pop();
                setVariable(lastIdentifier, value);
            } else {
                reportError(token, "Unexpected '=' — no identifier or value for assignment");
            }
            advance();
            break;
        }

        case TokenType::ELSE:
        case TokenType::END:
        case TokenType::EOF_TOKEN: {
            // End of current block — don't advance
            break;
        }

        default: {
            reportError(token, "Unexpected token");
            advance();
            recoverToNext();
            break;
        }
    }
}

// ============================================================
// Main parse function — orchestrates the full pipeline
// ============================================================
bool Parser::parse() {
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

    // Verify the generated IR
    bool irValid = verifyIR();

    // Print generated IR
    printIR();

    if (!irValid) {
        hasErrors = true;
    }

    return !hasErrors;
}
