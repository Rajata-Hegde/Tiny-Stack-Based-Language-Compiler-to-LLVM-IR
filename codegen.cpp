#include "compiler.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <stack>

// Global LLVM context and module
static llvm::LLVMContext context;
static std::unique_ptr<llvm::Module> module;
static std::unique_ptr<llvm::IRBuilder<>> builder;
static std::stack<llvm::Value*> valueStack;
static std::map<std::string, llvm::Value*> variables;

// Initialize LLVM module and builder
void initializeCodegen() {
    module = std::make_unique<llvm::Module>("stack_lang", context);
    builder = std::make_unique<llvm::IRBuilder<>>(context);
}

// Create main function
llvm::Function* createMainFunction() {
    llvm::FunctionType* mainType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context), 
        false
    );
    
    llvm::Function* mainFunc = llvm::Function::Create(
        mainType,
        llvm::Function::ExternalLinkage,
        "main",
        module.get()
    );
    
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder->SetInsertPoint(entry);
    
    return mainFunc;
}

// Emit final return statement
void emitReturn() {
    llvm::Value* retVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    
    if (!valueStack.empty()) {
        llvm::Value* topVal = valueStack.top();
        valueStack.pop();
        
        // Cast to i32 if needed and return it
        if (topVal->getType() != llvm::Type::getInt32Ty(context)) {
            retVal = builder->CreateSExtOrTrunc(topVal, llvm::Type::getInt32Ty(context));
        } else {
            retVal = topVal;
        }
    }
    
    builder->CreateRet(retVal);
}

// Push constant to stack
void emitPushConstant(int value) {
    llvm::Value* constVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value);
    valueStack.push(constVal);
}

// Get variable value (or allocate new one)
llvm::Value* getVariable(const std::string& name) {
    if (variables.find(name) != variables.end()) {
        return variables[name];
    }
    
    // Create a new i32 constant for uninitialized variables (default to 0)
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
}

// Set variable value
void setVariable(const std::string& name, llvm::Value* value) {
    variables[name] = value;
}

// Perform addition
void emitAdd() {
    if (valueStack.size() < 2) {
        std::cerr << "Error: Not enough values on stack for addition\n";
        return;
    }
    
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left = valueStack.top(); valueStack.pop();
    
    llvm::Value* result = builder->CreateAdd(left, right, "add_result");
    valueStack.push(result);
}

// Perform subtraction
void emitSub() {
    if (valueStack.size() < 2) {
        std::cerr << "Error: Not enough values on stack for subtraction\n";
        return;
    }
    
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left = valueStack.top(); valueStack.pop();
    
    llvm::Value* result = builder->CreateSub(left, right, "sub_result");
    valueStack.push(result);
}

// Perform multiplication
void emitMul() {
    if (valueStack.size() < 2) {
        std::cerr << "Error: Not enough values on stack for multiplication\n";
        return;
    }
    
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left = valueStack.top(); valueStack.pop();
    
    llvm::Value* result = builder->CreateMul(left, right, "mul_result");
    valueStack.push(result);
}

// Perform division
void emitDiv() {
    if (valueStack.size() < 2) {
        std::cerr << "Error: Not enough values on stack for division\n";
        return;
    }
    
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left = valueStack.top(); valueStack.pop();
    
    llvm::Value* result = builder->CreateSDiv(left, right, "div_result");
    valueStack.push(result);
}

// Greater than comparison
void emitGreater() {
    if (valueStack.size() < 2) {
        std::cerr << "Error: Not enough values on stack for comparison\n";
        return;
    }
    
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left = valueStack.top(); valueStack.pop();
    
    llvm::Value* cmp = builder->CreateICmpSGT(left, right, "cmp_gt");
    valueStack.push(cmp);
}

// Print the generated LLVM IR
void printIR() {
    module->print(llvm::outs(), nullptr);
}

// Get current LLVM module
llvm::Module* getModule() {
    return module.get();
}

// Get current IRBuilder
llvm::IRBuilder<>* getBuilder() {
    return builder.get();
}

// Get value stack
std::stack<llvm::Value*>* getValueStack() {
    return &valueStack;
}

// Get variables map
std::map<std::string, llvm::Value*>* getVariablesMap() {
    return &variables;
}
