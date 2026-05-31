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
// Push constant to stack (int or float based on value type)
// ============================================================
void emitPushConstant(int value) {
    llvm::Value* constVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value);
    valueStack.push(constVal);
}

void emitPushFloatConstant(double value) {
    llvm::Value* constVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), value);
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
// Arithmetic operations (support both int and double)
// ============================================================
void emitAdd() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in ADD\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    
    llvm::Value* result;
    if (left->getType() == llvm::Type::getDoubleTy(context) || 
        right->getType() == llvm::Type::getDoubleTy(context)) {
        // Float addition
        if (left->getType() != llvm::Type::getDoubleTy(context))
            left = builder->CreateSIToFP(left, llvm::Type::getDoubleTy(context), "i2f_left");
        if (right->getType() != llvm::Type::getDoubleTy(context))
            right = builder->CreateSIToFP(right, llvm::Type::getDoubleTy(context), "i2f_right");
        result = builder->CreateFAdd(left, right, "fadd_result");
    } else {
        // Integer addition
        result = builder->CreateAdd(left, right, "add_result");
    }
    valueStack.push(result);
}

void emitSub() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in SUB\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    
    llvm::Value* result;
    if (left->getType() == llvm::Type::getDoubleTy(context) || 
        right->getType() == llvm::Type::getDoubleTy(context)) {
        if (left->getType() != llvm::Type::getDoubleTy(context))
            left = builder->CreateSIToFP(left, llvm::Type::getDoubleTy(context), "i2f_left");
        if (right->getType() != llvm::Type::getDoubleTy(context))
            right = builder->CreateSIToFP(right, llvm::Type::getDoubleTy(context), "i2f_right");
        result = builder->CreateFSub(left, right, "fsub_result");
    } else {
        result = builder->CreateSub(left, right, "sub_result");
    }
    valueStack.push(result);
}

void emitMul() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in MUL\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    
    llvm::Value* result;
    if (left->getType() == llvm::Type::getDoubleTy(context) || 
        right->getType() == llvm::Type::getDoubleTy(context)) {
        if (left->getType() != llvm::Type::getDoubleTy(context))
            left = builder->CreateSIToFP(left, llvm::Type::getDoubleTy(context), "i2f_left");
        if (right->getType() != llvm::Type::getDoubleTy(context))
            right = builder->CreateSIToFP(right, llvm::Type::getDoubleTy(context), "i2f_right");
        result = builder->CreateFMul(left, right, "fmul_result");
    } else {
        result = builder->CreateMul(left, right, "mul_result");
    }
    valueStack.push(result);
}

void emitDiv() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in DIV\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    
    llvm::Value* result;
    if (left->getType() == llvm::Type::getDoubleTy(context) || 
        right->getType() == llvm::Type::getDoubleTy(context)) {
        if (left->getType() != llvm::Type::getDoubleTy(context))
            left = builder->CreateSIToFP(left, llvm::Type::getDoubleTy(context), "i2f_left");
        if (right->getType() != llvm::Type::getDoubleTy(context))
            right = builder->CreateSIToFP(right, llvm::Type::getDoubleTy(context), "i2f_right");
        result = builder->CreateFDiv(left, right, "fdiv_result");
    } else {
        result = builder->CreateSDiv(left, right, "div_result");
    }
    valueStack.push(result);
}

void emitMod() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in MOD\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    // Modulo only works on integers
    if (left->getType() != llvm::Type::getInt32Ty(context))
        left = builder->CreateFPToSI(left, llvm::Type::getInt32Ty(context), "f2i_left");
    if (right->getType() != llvm::Type::getInt32Ty(context))
        right = builder->CreateFPToSI(right, llvm::Type::getInt32Ty(context), "f2i_right");
    llvm::Value* result = builder->CreateSRem(left, right, "mod_result");
    valueStack.push(result);
}

// ============================================================
// Comparison operations (support both int and double)
// ============================================================
void emitGreater() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in GREATER\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    
    llvm::Value* cmp;
    if (left->getType() == llvm::Type::getDoubleTy(context) || 
        right->getType() == llvm::Type::getDoubleTy(context)) {
        if (left->getType() != llvm::Type::getDoubleTy(context))
            left = builder->CreateSIToFP(left, llvm::Type::getDoubleTy(context), "i2f_left");
        if (right->getType() != llvm::Type::getDoubleTy(context))
            right = builder->CreateSIToFP(right, llvm::Type::getDoubleTy(context), "i2f_right");
        cmp = builder->CreateFCmpOGT(left, right, "fcmp_gt");
    } else {
        cmp = builder->CreateICmpSGT(left, right, "cmp_gt");
    }
    valueStack.push(cmp);
}

void emitLess() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in LESS\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    
    llvm::Value* cmp;
    if (left->getType() == llvm::Type::getDoubleTy(context) || 
        right->getType() == llvm::Type::getDoubleTy(context)) {
        if (left->getType() != llvm::Type::getDoubleTy(context))
            left = builder->CreateSIToFP(left, llvm::Type::getDoubleTy(context), "i2f_left");
        if (right->getType() != llvm::Type::getDoubleTy(context))
            right = builder->CreateSIToFP(right, llvm::Type::getDoubleTy(context), "i2f_right");
        cmp = builder->CreateFCmpOLT(left, right, "fcmp_lt");
    } else {
        cmp = builder->CreateICmpSLT(left, right, "cmp_lt");
    }
    valueStack.push(cmp);
}

void emitEqual() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in EQUAL\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    
    llvm::Value* cmp;
    if (left->getType() == llvm::Type::getDoubleTy(context) || 
        right->getType() == llvm::Type::getDoubleTy(context)) {
        if (left->getType() != llvm::Type::getDoubleTy(context))
            left = builder->CreateSIToFP(left, llvm::Type::getDoubleTy(context), "i2f_left");
        if (right->getType() != llvm::Type::getDoubleTy(context))
            right = builder->CreateSIToFP(right, llvm::Type::getDoubleTy(context), "i2f_right");
        cmp = builder->CreateFCmpOEQ(left, right, "fcmp_eq");
    } else {
        cmp = builder->CreateICmpEQ(left, right, "cmp_eq");
    }
    valueStack.push(cmp);
}

void emitNotEqual() {
    if (valueStack.size() < 2) {
        std::cerr << "codegen error: stack underflow in NOT_EQUAL\n";
        return;
    }
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left  = valueStack.top(); valueStack.pop();
    
    llvm::Value* cmp;
    if (left->getType() == llvm::Type::getDoubleTy(context) || 
        right->getType() == llvm::Type::getDoubleTy(context)) {
        if (left->getType() != llvm::Type::getDoubleTy(context))
            left = builder->CreateSIToFP(left, llvm::Type::getDoubleTy(context), "i2f_left");
        if (right->getType() != llvm::Type::getDoubleTy(context))
            right = builder->CreateSIToFP(right, llvm::Type::getDoubleTy(context), "i2f_right");
        cmp = builder->CreateFCmpONE(left, right, "fcmp_ne");
    } else {
        cmp = builder->CreateICmpNE(left, right, "cmp_ne");
    }
    valueStack.push(cmp);
}

// ============================================================
// Stack manipulation (including new operations)
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
    llvm::Value* result;
    if (top->getType() == llvm::Type::getDoubleTy(context)) {
        result = builder->CreateFNeg(top, "fneg_result");
    } else {
        result = builder->CreateNeg(top, "neg_result");
    }
    valueStack.push(result);
}

void emitDrop() {
    if (valueStack.empty()) {
        std::cerr << "codegen error: stack underflow in DROP\n";
        return;
    }
    valueStack.pop();
}

// ============================================================
// Type conversion operations
// ============================================================
void emitF2I() {
    if (valueStack.empty()) {
        std::cerr << "codegen error: stack underflow in F2I\n";
        return;
    }
    llvm::Value* val = valueStack.top(); valueStack.pop();
    if (val->getType() == llvm::Type::getDoubleTy(context)) {
        llvm::Value* result = builder->CreateFPToSI(val, llvm::Type::getInt32Ty(context), "f2i_result");
        valueStack.push(result);
    } else {
        // Already an integer
        valueStack.push(val);
    }
}

void emitI2F() {
    if (valueStack.empty()) {
        std::cerr << "codegen error: stack underflow in I2F\n";
        return;
    }
    llvm::Value* val = valueStack.top(); valueStack.pop();
    if (val->getType() != llvm::Type::getDoubleTy(context)) {
        llvm::Value* result = builder->CreateSIToFP(val, llvm::Type::getDoubleTy(context), "i2f_result");
        valueStack.push(result);
    } else {
        // Already a float
        valueStack.push(val);
    }
}

// ============================================================
// I/O operations (enhanced to support floats)
// ============================================================

// print — pop TOS and print it (as integer or float followed by newline)
void emitPrint() {
    if (valueStack.empty()) {
        std::cerr << "codegen error: stack underflow in PRINT\n";
        return;
    }

    llvm::Value* val = valueStack.top(); valueStack.pop();

    if (val->getType() == llvm::Type::getDoubleTy(context)) {
        // Print as float: "%.6f\n"
        llvm::Value* fmtStr = builder->CreateGlobalStringPtr("%.6f\n", "fmt_print_float");
        builder->CreateCall(printfFunc, { fmtStr, val }, "printf_float_call");
    } else if (val->getType() == llvm::Type::getInt1Ty(context)) {
        // Cast boolean to int and print
        val = builder->CreateZExt(val, llvm::Type::getInt32Ty(context), "bool_to_int");
        llvm::Value* fmtStr = builder->CreateGlobalStringPtr("%d\n", "fmt_print_int");
        builder->CreateCall(printfFunc, { fmtStr, val }, "printf_int_call");
    } else {
        // Print as integer
        if (val->getType() != llvm::Type::getInt32Ty(context)) {
            val = builder->CreateFPToSI(val, llvm::Type::getInt32Ty(context), "f2i_print");
        }
        llvm::Value* fmtStr = builder->CreateGlobalStringPtr("%d\n", "fmt_print_int");
        builder->CreateCall(printfFunc, { fmtStr, val }, "printf_int_call");
    }
}

// println — same as print (newline already included)
void emitPrintln() {
    emitPrint();  // Our print already includes newline
}

// input — read a value from stdin, push onto stack
void emitInput() {
    // Read as double for more flexibility
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    llvm::IRBuilder<> entryBuilder(&func->getEntryBlock(),
                                    func->getEntryBlock().begin());
    llvm::AllocaInst* tmpAlloca = entryBuilder.CreateAlloca(
        llvm::Type::getDoubleTy(context), nullptr, "input_tmp"
    );

    // Create format string: "%lf" for double
    llvm::Value* fmtStr = builder->CreateGlobalStringPtr("%lf", "fmt_scan");

    // Call scanf(fmt, &tmp)
    builder->CreateCall(scanfFunc, { fmtStr, tmpAlloca }, "scanf_call");

    // Load the scanned value and push onto stack
    llvm::Value* loaded = builder->CreateLoad(
        llvm::Type::getDoubleTy(context), tmpAlloca, "input_val"
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
