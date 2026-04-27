# Stack → SSA Value Mapping Explanation

## Executive Summary

This document explains the **core insight** of the assignment: how a stack-based language maps directly to LLVM's SSA (Static Single Assignment) values.

---

## Part 1: Understanding SSA (Static Single Assignment)

### What is SSA?

SSA is LLVM's internal representation where:
- **Each value is assigned exactly once** (hence "Static Single Assignment")
- Each assignment creates a new temporary variable (prefixed with `%`)
- Values never change after creation (immutable)
- Every operation produces a new value

### Example:

```llvm
; Traditional code (not SSA):
x = 5
x = x + 3    ; x is reassigned - NOT allowed in SSA

; SSA equivalent:
%x1 = 5      ; First assignment of x
%x2 = add i32 %x1, 3  ; DIFFERENT variable %x2 (new assignment)
```

---

## Part 2: Stack-Based Language to SSA Mapping

### The Key Insight

In our stack-based language:
- **Each stack value becomes an SSA value**
- **Stack operations become IR instructions**
- **No explicit variables needed at IR level**

### Example 1: Simple Addition

#### Stack Language:
```
5 3 +
```

#### Execution Model (Stack):
```
Step 1: Push 5
  Stack: [5]

Step 2: Push 3
  Stack: [5, 3]

Step 3: Add (pop 2, push result)
  Stack: [8]

Step 4: Return
  Return: 8
```

#### LLVM IR Generated:
```llvm
define i32 @main() {
entry:
  %const_5 = 5          ; Constant 5 (SSA value 1)
  %const_3 = 3          ; Constant 3 (SSA value 2)
  %add_result = add i32 %const_5, %const_3  ; (SSA value 3)
  ret i32 %add_result   ; Return value 3
}
```

#### Code Mapping:

| Stack Operation | C++ Code | SSA Value | LLVM IR |
|-----------------|----------|-----------|---------|
| Push 5 | `emitPushConstant(5)` | `%const_5` | `(constant)` |
| Push 3 | `emitPushConstant(3)` | `%const_3` | `(constant)` |
| Add | `emitAdd()` | `%add_result` | `add i32 %const_5, %const_3` |
| Return | `emitReturn()` | (return 8) | `ret i32 %add_result` |

---

## Part 3: Complex Example with Multiple Operations

### Stack Language:
```
10 5 - 2 *
```
Result: (10 - 5) * 2 = 10

### Execution (Stack Trace):
```
Step 1: Push 10
  Stack: [10]
  SSA values: {%v1 = 10}

Step 2: Push 5
  Stack: [10, 5]
  SSA values: {%v1 = 10, %v2 = 5}

Step 3: Subtract
  Pop %v2, pop %v1
  %v3 = sub i32 %v1, %v2  (10 - 5 = 5)
  Stack: [5]
  SSA values: {%v1 = 10, %v2 = 5, %v3 = 5}

Step 4: Push 2
  Stack: [5, 2]
  SSA values: {..., %v4 = 2}

Step 5: Multiply
  Pop %v4, pop %v3
  %v5 = mul i32 %v3, %v4  (5 * 2 = 10)
  Stack: [10]
  SSA values: {..., %v5 = 10}

Step 6: Return
  Return %v5 (10)
```

### LLVM IR:
```llvm
define i32 @main() {
entry:
  %sub_result = sub i32 10, 5
  %mul_result = mul i32 %sub_result, 2
  ret i32 %mul_result
}
```

### C++ Code Flow:

```cpp
// Step 1-2: Push constants
emitPushConstant(10);     // valueStack.push(%v1)
emitPushConstant(5);      // valueStack.push(%v2)

// Step 3: Subtract
emitSub();  // This does:
  //   llvm::Value* right = valueStack.top(); valueStack.pop();  // Get %v2
  //   llvm::Value* left = valueStack.top(); valueStack.pop();   // Get %v1
  //   llvm::Value* result = builder->CreateSub(left, right);    // %v3
  //   valueStack.push(result);                                  // Push %v3

// Step 4: Push 2
emitPushConstant(2);      // valueStack.push(%v4)

// Step 5: Multiply
emitMul();  // This does:
  //   llvm::Value* right = valueStack.top(); valueStack.pop();  // Get %v4
  //   llvm::Value* left = valueStack.top(); valueStack.pop();   // Get %v3
  //   llvm::Value* result = builder->CreateMul(left, right);    // %v5
  //   valueStack.push(result);                                  // Push %v5

// Step 6: Return
emitReturn();  // Pop %v5 and return it
```

---

## Part 4: Variables and the Symbol Table

### How Variables Work

Variables are stored in a `map<string, llvm::Value*>` that maps names to SSA values.

### Example: Variable Assignment

#### Stack Language:
```
x 10 =
x 2 *
```

#### Execution (Stack + Symbol Table):

```
Step 1: Push 10
  Stack: [10]
  Symbols: {}
  SSA: %v1 = 10

Step 2: Variable name 'x' and assignment
  ASSIGN operation:
  Pop %v1 from stack
  Store in symbol table: x → %v1
  Stack: []
  Symbols: {x: %v1}

Step 3: Load variable 'x'
  Get value from symbols: x → %v1
  Push onto stack: %v1
  Stack: [%v1]
  Symbols: {x: %v1}

Step 4: Push 2
  Stack: [%v1, 2]
  SSA: %v2 = 2

Step 5: Multiply
  Pop %v2, pop %v1
  %v3 = mul i32 %v1, %v2
  Stack: [%v3]
  Symbols: {x: %v1}

Step 6: Return
  Return %v3 (20)
```

### LLVM IR:
```llvm
define i32 @main() {
entry:
  %mul_result = mul i32 10, 2
  ret i32 %mul_result
}
```

### C++ Code:

```cpp
// x 10 =
emitPushConstant(10);           // Push %v1
// Next token is 'x' (identifier)
// Next token is '=' (assign)
setVariable("x", valueStack.top());  // x → %v1
valueStack.pop();

// x 2 *
// Load x
llvm::Value* x_val = getVariable("x");  // Get %v1
valueStack.push(x_val);                 // Push %v1

emitPushConstant(2);            // Push %v2
emitMul();                       // Pop, pop, CreateMul → %v3

emitReturn();                    // Return %v3
```

---

## Part 5: Control Flow - If-Else with SSA

### The Challenge

Control flow creates multiple code paths. SSA requires special handling: **Phi nodes** merge values from different branches.

### Example: Conditional

#### Stack Language:
```
x 15 =
x 10 >
if
  x 1 +
else
  x 1 -
end
```

#### Execution (Stack Trace):

```
Step 1: x = 15
  Symbols: {x: %v1 (= 15)}

Step 2: x 10 >
  Load x → %v1
  Push 10 → %v2
  Compare: %v3 = icmp sgt i32 %v1, %v2  (15 > 10 = true/1)

Step 3: if (conditional branch)
  Condition: %v3 (true)
  Create 3 BasicBlocks:
    - then_block: for true branch
    - else_block: for false branch
    - end_if_block: merge point

  Branch on condition:
    br i1 %v3, label %then_block, label %else_block

Step 4: then_block (x + 1 = 16)
  Load x → %v1
  Push 1 → %v4
  Add: %v5 = add i32 %v1, %v4 (15 + 1 = 16)
  Branch to end: br label %end_if_block

Step 5: else_block (x - 1 = 14)
  Load x → %v1
  Push 1 → %v4
  Sub: %v6 = sub i32 %v1, %v4 (15 - 1 = 14)
  Branch to end: br label %end_if_block

Step 6: end_if_block (merge)
  Phi node: %v7 = phi i32 [%v5, %then_block], [%v6, %else_block]
  If condition was true: %v7 = %v5 (16)
  If condition was false: %v7 = %v6 (14)

Step 7: Return
  Return %v7
```

### LLVM IR:
```llvm
define i32 @main() {
entry:
  %cmp_gt = icmp sgt i32 15, 10
  br i1 %cmp_gt, label %then, label %else

then:                                             ; preds = %entry
  %add_result = add i32 15, 1
  br label %end_if

else:                                             ; preds = %entry
  %sub_result = sub i32 15, 1
  br label %end_if

end_if:                                           ; preds = %else, %then
  %phi = phi i32 [%add_result, %then], [%sub_result, %else]
  ret i32 %phi
}
```

### C++ Code Flow:

```cpp
// x 15 =
emitPushConstant(15);
setVariable("x", valueStack.top());
valueStack.pop();

// x 10 >
llvm::Value* x = getVariable("x");
valueStack.push(x);
emitPushConstant(10);
emitGreater();  // Computes %cmp_gt

// if
parseIfElse();  // This does:
  //
  // 1. Pop condition from stack
  llvm::Value* cond = valueStack.top(); valueStack.pop();  // %cmp_gt
  
  // 2. Create BasicBlocks
  llvm::BasicBlock* then_bb = BasicBlock::Create(..., "then", mainFunc);
  llvm::BasicBlock* else_bb = BasicBlock::Create(..., "else", mainFunc);
  llvm::BasicBlock* end_bb = BasicBlock::Create(..., "end_if", mainFunc);
  
  // 3. Branch on condition (from entry block)
  builder->CreateCondBr(cond, then_bb, else_bb);
  
  // 4. Process then block
  builder->SetInsertPoint(then_bb);
  // Execute: x 1 +
  //   This generates %add_result = add i32 15, 1
  builder->CreateBr(end_bb);  // Jump to merge
  
  // 5. Process else block
  builder->SetInsertPoint(else_bb);
  // Execute: x 1 -
  //   This generates %sub_result = sub i32 15, 1
  builder->CreateBr(end_bb);  // Jump to merge
  
  // 6. Merge block
  builder->SetInsertPoint(end_bb);
  // Implicit phi created by LLVM
```

---

## Part 6: Summary - Stack to SSA Mapping

### Core Principles

| Concept | Stack Model | SSA Model |
|---------|-------------|-----------|
| **Value** | Item on stack | SSA value (`%var`) |
| **Push** | Add to stack | Create SSA value |
| **Pop** | Remove from stack | Reference SSA value |
| **Operation** | Stack pops/pushes | IR instruction creates new SSA value |
| **Variable** | Symbol table entry | SSA value stored in map |
| **Control Flow** | Branch | BasicBlocks + phi nodes |

### The Magic: Implicit Optimization

```
Stack language: 10 5 - 2 *
Stack operations: push, push, sub, push, mul

SSA IR (before optimization):
  %v1 = 10
  %v2 = 5
  %v3 = sub i32 %v1, %v2
  %v4 = 2
  %v5 = mul i32 %v3, %v4

SSA IR (after optimization with -O2):
  %v1 = 10       ; Could be folded
  %v1 = sub i32 10, 5
  ; Optimizer might compute: 5 * 2 = 10 at compile time!
```

---

## Part 7: Code Implementation Details

### How Our Compiler Implements This

#### In `codegen.cpp`:

```cpp
// Global stack (each element is an SSA value)
static std::stack<llvm::Value*> valueStack;

// Symbol table (maps variable names to SSA values)
static std::map<std::string, llvm::Value*> variables;

// Push: creates and stores SSA value
void emitPushConstant(int value) {
    llvm::Value* constVal = llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(context), 
        value
    );
    valueStack.push(constVal);  // Push SSA value
}

// Add: pops two SSA values, creates new one
void emitAdd() {
    llvm::Value* right = valueStack.top(); valueStack.pop();
    llvm::Value* left = valueStack.top(); valueStack.pop();
    
    // CreateAdd generates IR: %add_result = add i32 %left, %right
    llvm::Value* result = builder->CreateAdd(left, right, "add_result");
    
    valueStack.push(result);  // Push new SSA value
}

// Variable storage
void setVariable(const std::string& name, llvm::Value* value) {
    variables[name] = value;  // Store SSA value
}

llvm::Value* getVariable(const std::string& name) {
    return variables[name];   // Retrieve SSA value
}
```

#### In `parser.cpp`:

```cpp
// When parsing a token:
switch (token.type) {
    case NUMBER:
        emitPushConstant(stoi(token.value));
        // Creates and pushes SSA value onto valueStack
        break;
    
    case IDENTIFIER:
        if (nextToken == ASSIGN) {
            // Pop SSA value and store in symbol table
            setVariable(token.value, valueStack.top());
            valueStack.pop();
        } else {
            // Load SSA value from symbol table and push
            valueStack.push(getVariable(token.value));
        }
        break;
    
    case PLUS:
        emitAdd();  // Creates new SSA value
        break;
    
    // ... similar for other operations
}
```

---

## Part 8: Verification - Run This!

### Test Program 1: Basic Addition
```bash
./compiler program1.sl
```
**Expected Output:**
```llvm
%add_result = add i32 5, 3
ret i32 %add_result
```
**Stack→SSA: [5] + [3] = [8]**

### Test Program 2: Multiple Operations
```bash
./compiler program2.sl
```
**Expected Output:**
```llvm
%mul_result = mul i32 10, 5
%div_result = sdiv i32 %mul_result, 2
ret i32 %div_result
```
**Stack→SSA: [10, 5] → mul → [50] → [50, 2] → div → [25]**

### Test Program 3: Variables
```bash
./compiler program3.sl
```
**Expected Output:**
```llvm
%mul_result = mul i32 10, 2
%add_result = add i32 %mul_result, 5
ret i32 %add_result
```
**Stack→SSA: x=10, load x→[10], multiply→[20], add→[25]**

### Test Program 4: Conditionals
```bash
./compiler program4.sl
```
**Expected Output:**
```llvm
%cmp_gt = icmp sgt i32 15, 10
br i1 %cmp_gt, label %then, label %else
then:
  %add_result = add i32 15, 1
  br label %end_if
else:
  %sub_result = sub i32 15, 1
  br label %end_if
end_if:
  ret i32 <phi result>
```
**Stack→SSA: Branch on condition, merge paths with phi node**

---

## Part 9: Key Insights

### Why This Matters

1. **Stack operations are ephemeral** - they don't persist in SSA
2. **Each SSA value is immutable** - multiple assignments create multiple values
3. **Variables are bridges** - map external names to internal SSA values
4. **Control flow needs phi nodes** - to select between multiple SSA values
5. **Compiler optimization works on SSA** - because it's explicit and immutable

### The Learning Value

This project teaches:
- How real compilers work internally
- Why SSA is better than traditional code
- How stack machines map to LLVM
- Compiler architecture end-to-end

---

## Part 10: Glossary

| Term | Definition |
|------|-----------|
| **SSA** | Static Single Assignment - each variable assigned exactly once |
| **Value** | An SSA value (result of an operation) |
| **BasicBlock** | Sequence of instructions ending with terminator |
| **Phi Node** | Merges values from multiple control flow paths |
| **IRBuilder** | LLVM API for creating IR instructions |
| **Symbol Table** | Maps names to SSA values |
| **Terminator** | Last instruction in a block (branch/return) |
| **Temporary** | Intermediate SSA value (like stack items) |

---

## Conclusion

Your stack-based compiler perfectly demonstrates the **stack → SSA mapping**:

- **Source program** uses a simple stack model
- **Lexer** tokenizes it
- **Parser** processes tokens and manages a stack
- **CodeGen** emits LLVM IR where each stack item becomes an SSA value
- **LLVM** optimizes and compiles to native code

This is exactly how real interpreters and compilers work!

✅ **Assignment Complete!**
