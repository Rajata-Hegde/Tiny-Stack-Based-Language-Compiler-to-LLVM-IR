#include "compiler.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <stack>

// ============================================================
// Global LLVM state
// ============================================================
static llvm::LLVMContext context;
static std::unique_ptr<llvm::Module> module;
static std::unique_ptr<llvm::IRBuilder<>> builder;
static std::stack<llvm::Value*> valueStack;
static std::map<std::string, llvm::Value*> variables;

// External function references for I/O
static llvm::Function* printfFunc  = nullptr;
static llvm::Function* scanfFunc   = nullptr;

// ============================================================
// Initialize LLVM module and builder
// ============================================================
void initializeCodegen() {
    module = std::make_unique<llvm::Module>("stack_lang", context);
    builder = std::make_unique<llvm::IRBuilder<>>(context);
}

// ============================================================
// Declare external C functions: printf, scanf
// ============================================================
void declarePrintf() {
    if (printfFunc) return;  // already declared

    // int printf(const char* fmt, ...)
    llvm::Type* ptrTy = llvm::PointerType::getUnqual(context);
    std::vector<llvm::Type*> printfArgs = { ptrTy };
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context),
        printfArgs,
        true  // variadic
    );
    printfFunc = llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        module.get()
    );
}

void declareScanf() {
    if (scanfFunc) return;  // already declared

    // int scanf(const char* fmt, ...)
    llvm::Type* ptrTy = llvm::PointerType::getUnqual(context);
    std::vector<llvm::Type*> scanfArgs = { ptrTy };
    llvm::FunctionType* scanfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context),
        scanfArgs,
        true  // variadic
    );
    scanfFunc = llvm::Function::Create(
        scanfType,
        llvm::Function::ExternalLinkage,
        "scanf",
        module.get()
    );
}

// ============================================================
// Create main function: int main()
// ============================================================
llvm::Function* createMainFunction() {
    // Ensure I/O functions are declared before any code generation
    declarePrintf();
    declareScanf();

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

// ============================================================
// Emit final return statement
// ============================================================
void emitReturn() {
    llvm::Value* retVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);

    if (!valueStack.empty()) {
        llvm::Value* topVal = valueStack.top();
        valueStack.pop();

        // Cast to i32 if needed and return it
        if (topVal->getType() != llvm::Type::getInt32Ty(context)) {
            retVal = builder->CreateSExtOrTrunc(topVal, llvm::Type::getInt32Ty(context), "ret_cast");
        } else {
            retVal = topVal;
        }
    }

    builder->CreateRet(retVal);
}

// ============================================================
// Push constant to stack
// ============================================================
void emitPushConstant(int value) {
    llvm::Value* constVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value);
    valueStack.push(constVal);
}

// ============================================================
// Variable get/set
// ============================================================
llvm::Value* getVariable(const std::string& name) {
    if (variables.find(name) != variables.end()) {
        return variables[name];
    }

    // Undefined variable defaults to 0 (validator should have caught this)
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
}

void setVariable(const std::string& name, llvm::Value* value) {
    variables[name] = value;
}

// ============================================================
// Arithmetic operations
// ============================================================
void emitAdd() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in ADD\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* result = builder->CreateAdd(left, right, "add_result");
    valueStack.push(result);
}

void emitSub() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in SUB\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* result = builder->CreateSub(left, right, "sub_result");
    valueStack.push(result);
}

void emitMul() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in MUL\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* result = builder->CreateMul(left, right, "mul_result");
    valueStack.push(result);
}

void emitDiv() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in DIV\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* result = builder->CreateSDiv(left, right, "div_result");
    valueStack.push(result);
}

void emitMod() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in MOD\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* result = builder->CreateSRem(left, right, "mod_result");
    valueStack.push(result);
}

// ============================================================
// Comparison operations
// ============================================================
void emitGreater() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in GREATER\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* cmp = builder->CreateICmpSGT(left, right, "cmp_gt");
    valueStack.push(cmp);
}

void emitLess() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in LESS\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* cmp = builder->CreateICmpSLT(left, right, "cmp_lt");
    valueStack.push(cmp);
}

void emitEqual() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in EQUAL\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* cmp = builder->CreateICmpEQ(left, right, "cmp_eq");
    valueStack.push(cmp);
}

void emitNotEqual() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in NOT_EQUAL\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    llvm::Value* cmp = builder->CreateICmpNE(left, right, "cmp_ne");
    valueStack.push(cmp);
}

// ============================================================
// Stack manipulation
// ============================================================
void emitDup() {
    if (valueStack.empty()) {
        std::cerr << "codegen error: stack underflow in DUP\n";
        return;
    }
    llvm::Value* top = valueStack.top();
    valueStack.push(top);  // push a copy
}

void emitSwap() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in SWAP\n";
        return;
    }
    llvm::Value* a = valueStack.top(); valueStack.pop();
    llvm::Value* b = valueStack.top(); valueStack.pop();
    valueStack.push(a);
    valueStack.push(b);
}

void emitNeg() {
    if (valueStack.empty()) {
        std::cerr << "codegen error: stack underflow in NEG\n";
        return;
    }
    llvm::Value* top = valueStack.top(); valueStack.pop();
    llvm::Value* result = builder->CreateNeg(top, "neg_result");
    valueStack.push(result);
}

// ============================================================
// I/O operations
// ============================================================

// print — pop TOS and print it as an integer followed by newline
void emitPrint() {
    if (valueStack.empty()) {
        std::cerr << "codegen error: stack underflow in PRINT\n";
        return;
    }

    llvm::Value* val = valueStack.top(); valueStack.pop();

    // Cast to i32 if it's a boolean (i1)
    if (val->getType() == llvm::Type::getInt1Ty(context)) {
        val = builder->CreateZExt(val, llvm::Type::getInt32Ty(context), "bool_to_int");
    }

    // Create format string: "%d\n"
    llvm::Value* fmtStr = builder->CreateGlobalStringPtr("%d\n", "fmt_print");

    // Call printf(fmt, val)
    builder->CreateCall(printfFunc, { fmtStr, val }, "printf_call");
}

// input — read an integer from stdin, push onto stack
void emitInput() {
    // Allocate space for the scanned integer on the stack
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    llvm::IRBuilder<> entryBuilder(&func->getEntryBlock(),
                                    func->getEntryBlock().begin());
    llvm::AllocaInst* tmpAlloca = entryBuilder.CreateAlloca(
        llvm::Type::getInt32Ty(context), nullptr, "input_tmp"
    );

    // Create format string: "%d"
    llvm::Value* fmtStr = builder->CreateGlobalStringPtr("%d", "fmt_scan");

    // Call scanf(fmt, &tmp)
    builder->CreateCall(scanfFunc, { fmtStr, tmpAlloca }, "scanf_call");

    // Load the scanned value and push onto stack
    llvm::Value* loaded = builder->CreateLoad(
        llvm::Type::getInt32Ty(context), tmpAlloca, "input_val"
    );
    valueStack.push(loaded);
}

// ============================================================
// Print the generated LLVM IR
// ============================================================
void printIR() {
    module->print(llvm::outs(), nullptr);
}

// ============================================================
// Verify generated IR module
// ============================================================
bool verifyIR() {
    std::string errStr;
    llvm::raw_string_ostream errStream(errStr);
    if (llvm::verifyModule(*module, &errStream)) {
        std::cerr << "\n=== LLVM IR Verification FAILED ===\n";
        std::cerr << errStr << "\n";
        return false;
    }
    return true;
}

// ============================================================
// Accessor functions for the parser
// ============================================================
llvm::Module* getModule() {
    return module.get();
}

llvm::IRBuilder<>* getBuilder() {
    return builder.get();
}

std::stack<llvm::Value*>* getValueStack() {
    return &valueStack;
}

std::map<std::string, llvm::Value*>* getVariablesMap() {
    return &variables;
}

llvm::LLVMContext* getLLVMContext() {
    return &context;
}
