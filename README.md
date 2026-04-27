# Stack-Based Language Compiler with LLVM

A minimal, stack-based programming language compiler that generates LLVM Intermediate Representation (IR) using C++ and the LLVM API.

## Project Structure

```
compiler.h        - Header file with token definitions and class declarations
lexer.cpp         - Tokenizer: converts input string to tokens
parser.cpp        - Parser: processes tokens and generates LLVM IR
codegen.cpp       - Code generator: LLVM IR emission functions
main.cpp          - Entry point: orchestrates lexer → parser → IR output
program*.sl       - Sample input programs
```

## Language Syntax

### Basic Operations

The language uses **Reverse Polish Notation (RPN)** on a stack:

```
5 3 +     → Push 5, Push 3, Add → Result: 8
10 5 * 2 / → Push 10, Push 5, Multiply, Push 2, Divide → Result: 25
```

### Supported Operations

| Operation | Syntax | Stack Before | Stack After |
|-----------|--------|--------------|-------------|
| Addition | `+` | `[a, b]` | `[a+b]` |
| Subtraction | `-` | `[a, b]` | `[a-b]` |
| Multiplication | `*` | `[a, b]` | `[a*b]` |
| Division | `/` | `[a, b]` | `[a/b]` |
| Greater Than | `>` | `[a, b]` | `[a>b ? 1 : 0]` |

### Variable Assignment

```
x 10 =      → Assign 10 to variable x
x 2 * 5 +   → Load x, multiply by 2, add 5
```

### Conditionals

```
x 15 =
x 10 > if
  x 1 +
else
  x 1 -
end
```

- Condition on stack
- `if` starts true branch
- `else` starts false branch (optional)
- `end` terminates conditional

## Token Types

| Token | Type | Example |
|-------|------|---------|
| Number | `NUMBER` | `5`, `42`, `100` |
| Identifier | `IDENTIFIER` | `x`, `y`, `count` |
| Operator | `PLUS`, `MINUS`, `MUL`, `DIV` | `+`, `-`, `*`, `/` |
| Comparison | `GREATER` | `>` |
| Assignment | `ASSIGN` | `=` |
| Control | `IF`, `ELSE`, `END` | `if`, `else`, `end` |

## Compilation

### Prerequisites

- G++ compiler (or any C++ compiler)
- LLVM development libraries (version 10+)
- llvm-config utility

### Compile Command

```bash
# Windows (PowerShell)
g++ *.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core` -o compiler.exe

# Linux/Mac
g++ *.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core` -o compiler
```

### Troubleshooting Compilation

If `llvm-config` is not found:
1. Install LLVM dev packages:
   - Ubuntu/Debian: `sudo apt-get install llvm-dev`
   - macOS: `brew install llvm`
   - Windows: Download from https://llvm.org/

2. Add LLVM to PATH or use full path:
   ```bash
   g++ *.cpp `/usr/lib/llvm-14/bin/llvm-config --cxxflags --ldflags --system-libs --libs core` -o compiler
   ```

## Usage

### Running the Compiler

```bash
# Run on a program file
./compiler.exe program1.sl

# Output includes:
# 1. Input program
# 2. Tokenized output
# 3. LLVM IR (.ll format)
```

### Example Execution

```bash
./compiler.exe program1.sl
```

**Input** (`program1.sl`):
```
5 3 +
```

**Output**:
```
==== Input Program ====
5 3 +

==== Tokens ====
Token: NUMBER = "5"
Token: NUMBER = "3"
Token: PLUS = "+"
Token: EOF = ""

==== LLVM IR Output ====
; ModuleID = 'stack_lang'
source_filename = "stack_lang"

define i32 @main() {
entry:
  %add_result = add i32 5, 3
  ret i32 %add_result
}
```

## Sample Programs

### Program 1: Simple Addition (`program1.sl`)
```
5 3 +
```
**Expected Result**: Returns 8

**LLVM IR**:
```llvm
define i32 @main() {
entry:
  %add_result = add i32 5, 3
  ret i32 %add_result
}
```

### Program 2: Mixed Operations (`program2.sl`)
```
10 5 * 2 /
```
**Expected Result**: Returns 25 (10 * 5 / 2 = 25)

**LLVM IR**:
```llvm
define i32 @main() {
entry:
  %mul_result = mul i32 10, 5
  %div_result = sdiv i32 %mul_result, 2
  ret i32 %div_result
}
```

### Program 3: Variable Usage (`program3.sl`)
```
x 10 = x 2 * 5 +
```
**Expected Result**: Returns 25 (x=10, x*2+5 = 25)

**LLVM IR**:
```llvm
define i32 @main() {
entry:
  %mul_result = mul i32 10, 2
  %add_result = add i32 %mul_result, 5
  ret i32 %add_result
}
```

### Program 4: Conditional (`program4.sl`)
```
x 15 = x 10 > if x 1 + else x 1 - end
```
**Expected Result**: Returns 16 (15 > 10 is true, so 15 + 1 = 16)

**LLVM IR**:
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
  ret i32 %sub_result
}
```

## Architecture

### Lexer (`lexer.cpp`)
- Splits input by whitespace
- Identifies token types (numbers, operators, keywords)
- Returns vector of tokens with type and value

### Parser (`parser.cpp`)
- Processes tokens sequentially
- Maintains expression stack with `std::stack<llvm::Value*>`
- Immediately generates LLVM IR (no AST)
- Handles operators, assignments, and control flow

### Code Generator (`codegen.cpp`)
- Uses LLVM API: `LLVMContext`, `Module`, `IRBuilder`
- Implements arithmetic operations: `CreateAdd`, `CreateSub`, `CreateMul`, `CreateSDiv`
- Handles conditionals with `BasicBlock` and `CreateCondBr`
- Stores variables in `std::map<string, Value*>`

### Main (`main.cpp`)
- Reads program file
- Orchestrates lexer → parser → IR generation
- Prints tokenized output and LLVM IR

## Implementation Details

### Stack-Based Execution Model

```cpp
std::stack<llvm::Value*> valueStack;  // Runtime stack

// Example: 5 3 +
emitPushConstant(5);      // valueStack: [5]
emitPushConstant(3);      // valueStack: [5, 3]
emitAdd();                // valueStack: [8]
```

### LLVM IR Generation

```cpp
// Addition example generates:
%add_result = add i32 5, 3
```

### Variable Storage

```cpp
std::map<std::string, llvm::Value*> variables;

// x 10 =
setVariable("x", ConstantInt::get(Type::getInt32Ty(context), 10));

// x 2 *
llvm::Value* x_val = getVariable("x");  // Returns 10
// ... multiply by 2
```

### Conditional Branching

```cpp
if (condition > 0)
  // then block
else
  // else block
end

// Generates:
// entry:
//   icmp sgt i32 condition, 0
//   br i1 result, label %then, label %else
// then:
//   ...
// else:
//   ...
// end_if:
//   ...
```

## Limitations and Future Enhancements

### Current Limitations
1. No loop constructs (while, for)
2. No function definitions
3. No arrays or complex data structures
4. Limited error handling
5. Variables stored as immediate values (no heap allocation)
6. No floating-point support

### Potential Enhancements
1. Add `while` loop support
2. Function definitions and calls
3. Array/list operations
4. Better error messages with line numbers
5. Floating-point arithmetic
6. Memory allocation with `malloc`/`free`
7. Optimization passes
8. Output to `.o` object files or executables

## References

- LLVM Documentation: https://llvm.org/docs/
- LLVM IR Reference: https://llvm.org/docs/LangRef/
- LLVM C++ API: https://llvm.org/doxygen/

## License

Public domain - use freely for educational purposes.
