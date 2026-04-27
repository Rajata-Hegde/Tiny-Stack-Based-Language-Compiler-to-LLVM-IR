# Complete Project Structure - Stack Language Compiler

## Overview

This is a complete, working stack-based language compiler written in C++ that generates LLVM IR. It consists of 4 main source files plus supporting documentation.

## Project Files

### Core Implementation Files

#### 1. `compiler.h` - Header File
**Purpose**: Defines all types, enums, and class declarations

**Contains**:
- `TokenType` enum - All token types (NUMBER, IDENTIFIER, PLUS, MINUS, MUL, DIV, ASSIGN, GREATER, IF, ELSE, END, EOF_TOKEN)
- `Token` struct - Token type + value pair
- `Lexer` class - Tokenization interface
- `Parser` class - Parsing interface

**Size**: ~50 lines

#### 2. `lexer.cpp` - Lexical Analyzer
**Purpose**: Converts input string into tokens

**Key Functions**:
- `Lexer::Lexer()` - Constructor
- `Lexer::tokenize()` - Main tokenization function
- `Lexer::identifyKeywordOrOperator()` - Token classification

**Algorithm**:
1. Split input by whitespace
2. For each word:
   - Check if it's a number → NUMBER token
   - Check if it's a keyword/operator → appropriate token type
   - Otherwise → IDENTIFIER token
3. Return vector of tokens

**Size**: ~50 lines

#### 3. `parser.cpp` - Parser/Interpreter
**Purpose**: Processes tokens sequentially and generates LLVM IR

**Key Functions**:
- `Parser::Parser()` - Constructor
- `Parser::parse()` - Main parsing entry point
- `Parser::parseExpression()` - Process individual tokens
- `Parser::parseIfElse()` - Handle conditional blocks
- `Parser::currentToken()` - Get current token
- `Parser::advance()` - Move to next token

**Algorithm**:
1. For each token:
   - NUMBER: Push constant to value stack
   - IDENTIFIER: Load/store variable
   - OPERATOR (+, -, *, /): Pop 2 values, apply operation, push result
   - COMPARISON (>): Pop 2 values, compare, push boolean
   - IF: Create conditional basic blocks and branch
   - ELSE/END: Handle conditional block merging

**Size**: ~150 lines

#### 4. `codegen.cpp` - Code Generator
**Purpose**: LLVM IR emission functions

**Global State**:
- `llvm::LLVMContext context` - LLVM runtime context
- `std::unique_ptr<llvm::Module> module` - Generated IR module
- `std::unique_ptr<llvm::IRBuilder<>> builder` - IR builder
- `std::stack<llvm::Value*> valueStack` - Runtime value stack
- `std::map<std::string, llvm::Value*> variables` - Symbol table

**Key Functions**:
- `initializeCodegen()` - Initialize LLVM context/module/builder
- `createMainFunction()` - Create main() function
- `emitPushConstant(int)` - Push constant to stack
- `emitAdd()` - Generate add instruction
- `emitSub()` - Generate subtract instruction
- `emitMul()` - Generate multiply instruction
- `emitDiv()` - Generate signed divide instruction
- `emitGreater()` - Generate comparison instruction
- `getVariable()` - Look up variable value
- `setVariable()` - Store variable value
- `emitReturn()` - Generate final return statement
- `printIR()` - Output LLVM IR module

**Size**: ~180 lines

#### 5. `main.cpp` - Entry Point
**Purpose**: Orchestrate the compilation pipeline

**Key Functions**:
- `readFile()` - Read program from file
- `main()` - Entry point

**Pipeline**:
1. Read input file (argument 1)
2. Print input program
3. Run lexer → get tokens
4. Print tokens
5. Create parser → run parse()
6. Parse() calls codegen functions and prints IR

**Size**: ~70 lines

### Sample Programs

#### `program1.sl` - Simple Addition
```
5 3 +
```
- Returns: 8

#### `program2.sl` - Multiple Operations
```
10 5 * 2 /
```
- Returns: 25

#### `program3.sl` - Variables
```
x 10 = x 2 * 5 +
```
- Returns: 25 (assign 10 to x, calculate x*2+5)

#### `program4.sl` - Conditional
```
x 15 = x 10 > if x 1 + else x 1 - end
```
- Returns: 16 (15 > 10 is true, so 15+1)

### Build Files

#### `build.sh` - Linux/WSL Build Script
- Checks for LLVM installation
- Compiles all source files with LLVM flags
- Creates executable `./compiler`

#### `Makefile` - Build Configuration
- Targets: `all`, `run-examples`, `clean`, `rebuild`
- Automatically extracts LLVM flags using llvm-config

#### `build.bat` - Windows Build Script (PowerShell)
- Alternative for Windows users
- Uses PowerShell backtick for command substitution

### Documentation

#### `README.md` - Main Documentation
- Project overview
- Language syntax reference
- Compilation instructions
- Usage examples
- Architecture overview

#### `BUILD.md` - Detailed Build Guide
- Prerequisites and installation
- Multiple build methods
- Troubleshooting guide
- File structure reference

#### `WSL_QUICKSTART.md` - WSL-Specific Guide
- Step-by-step WSL setup
- Quick build and run
- Common tasks
- Example walkthroughs

## Compilation Flow

```
Input File (.sl)
    ↓
main() reads file
    ↓
Lexer::tokenize()
    ↓ produces vector<Token>
main() prints tokens
    ↓
Parser::parse()
    ├→ initializeCodegen() [creates LLVM context/module/builder]
    ├→ createMainFunction() [creates main() LLVM function]
    ├→ parseExpression() for each token
    │   ├→ NUMBER: emitPushConstant()
    │   ├→ IDENTIFIER: getVariable() / setVariable()
    │   ├→ +: emitAdd()
    │   ├→ -: emitSub()
    │   ├→ *: emitMul()
    │   ├→ /: emitDiv()
    │   ├→ >: emitGreater()
    │   └→ if: parseIfElse() [creates BasicBlocks & branches]
    ├→ emitReturn() [final return from stack]
    └→ printIR() [output LLVM IR module]
    ↓
LLVM IR (.ll format) to stdout
```

## Language Grammar

```
Program := Statement*

Statement := 
    | NUMBER                          # Push constant
    | IDENTIFIER                      # Load variable or label
    | IDENTIFIER '=' Statement+       # Assignment
    | Operator                        # Binary operation (+, -, *, /, >)
    | 'if' Statement+ 'else' Statement+ 'end'  # Conditional
    | 'if' Statement+ 'end'          # Conditional (no else)

Operator := '+' | '-' | '*' | '/' | '>'
```

## Data Flow

```
                      ┌─────────────┐
                      │ Input File  │
                      └──────┬──────┘
                             │
                      ┌──────▼──────┐
                      │   Lexer     │ (lexer.cpp)
                      │  tokenize   │
                      └──────┬──────┘
                             │
                      ┌──────▼──────┐
                      │   Tokens    │ vector<Token>
                      └──────┬──────┘
                             │
                      ┌──────▼──────┐
                      │   Parser    │ (parser.cpp)
                      │    parse    │
                      └──────┬──────┘
                             │
                      ┌──────▼──────────────────┐
                      │   CodeGen Functions    │ (codegen.cpp)
                      │  (emit*, get*, set*)   │
                      └──────┬──────────────────┘
                             │
                ┌────────────┬┴──────────────┬──────────────┐
                │            │              │              │
         ┌──────▼──┐  ┌──────▼──┐  ┌───────▼────┐  ┌─────▼──────┐
         │ Module  │  │ Builder │  │ValueStack  │  │ Variables  │
         │ (LLVM)  │  │(LLVM)   │  │(runtime)   │  │(map)       │
         └────┬────┘  └─────────┘  └────────────┘  └────────────┘
              │
       ┌──────▼──────┐
       │  printIR()  │ (codegen.cpp)
       └──────┬──────┘
              │
       ┌──────▼──────┐
       │ LLVM IR (.ll)│
       │ to stdout    │
       └──────────────┘
```

## Code Statistics

| File | Lines | Purpose |
|------|-------|---------|
| compiler.h | 50 | Type definitions |
| lexer.cpp | 50 | Tokenization |
| parser.cpp | 150 | Parsing |
| codegen.cpp | 180 | LLVM IR generation |
| main.cpp | 70 | Entry point |
| **Total** | **500** | Complete compiler |

## LLVM API Usage

### Key LLVM Classes/Functions Used

```cpp
// Context and module setup
llvm::LLVMContext context;
llvm::Module module("name", context);

// Function creation
llvm::FunctionType::get(returnType, paramTypes, false);
llvm::Function::Create(funcType, linkage, name, module);

// IR building
llvm::IRBuilder<> builder(context);
builder.SetInsertPoint(basicBlock);

// Instruction generation
builder.CreateAdd(left, right, "name");
builder.CreateSub(left, right, "name");
builder.CreateMul(left, right, "name");
builder.CreateSDiv(left, right, "name");
builder.CreateICmpSGT(left, right, "name");
builder.CreateCondBr(condition, thenBlock, elseBlock);
builder.CreateRet(value);

// Control flow
llvm::BasicBlock::Create(context, "name", function);

// Constants
llvm::ConstantInt::get(type, value);
```

## Supported Language Features

### Operators
- Addition: `5 3 +` → 8
- Subtraction: `10 3 -` → 7
- Multiplication: `5 3 *` → 15
- Division: `10 3 /` → 3
- Greater than: `5 3 >` → 1 (true)

### Variables
- Assignment: `x 10 =`
- Usage: `x 2 *` (load x, multiply by 2)

### Control Flow
- If-else: `condition if ... else ... end`
- Condition is on stack (non-zero = true)

## Building Instructions

### WSL/Linux
```bash
chmod +x build.sh
./build.sh
```

### macOS
```bash
brew install llvm
./build.sh
```

### Manual
```bash
g++ *.cpp $(llvm-config --cxxflags --ldflags --system-libs --libs core) -o compiler
```

## Running Programs

```bash
./compiler program1.sl
./compiler program2.sl
./compiler program3.sl
./compiler program4.sl
```

## Expected Output Format

```
==== Input Program ====
<program source>

==== Tokens ====
Token: <TYPE> = "<value>"
...

==== LLVM IR Output ====
; ModuleID = 'stack_lang'
source_filename = "stack_lang"

define i32 @main() {
entry:
  <IR instructions>
  ret i32 <result>
}
```

## Extension Points

### Adding New Token Type
1. Add to `TokenType` enum in `compiler.h`
2. Update `Lexer::identifyKeywordOrOperator()` in `lexer.cpp`

### Adding New Operator
1. Add token type
2. Update lexer
3. Add case in `Parser::parseExpression()` in `parser.cpp`
4. Implement `emit*()` function in `codegen.cpp`

### Adding New Language Feature
1. Plan token types needed
2. Update lexer
3. Add parsing logic in parser
4. Implement code generation
5. Test with sample program

## Limitations

- **Single function**: Only generates `main()` function
- **No loops**: While/for loops not supported
- **No function calls**: Cannot define or call functions
- **No dynamic allocation**: No heap memory management
- **No floating point**: Only 32-bit integers
- **Limited error handling**: First error stops compilation
- **Stack-only variables**: Variables stored as immediate values

## Performance Considerations

- **No optimization**: Generated IR is straightforward
- **Direct codegen**: No AST overhead
- **Minimal allocations**: Stack-based execution model
- **Sequential processing**: Single-pass compilation

## Production Use

To use generated IR in production:

```bash
# Optimize the IR
./compiler program.sl > prog.ll
opt -O3 prog.ll -o prog_opt.ll

# Generate native code
llc prog_opt.ll -o prog.s
gcc -c prog.s -o prog.o

# Link with C runtime
gcc prog.o -lc -o prog
./prog
echo $?  # Print exit code
```

## Testing

```bash
# Build and run all examples
make run-examples

# Create custom test
echo "42" > test.sl
./compiler test.sl
```

---

**Total**: Complete, working compiler in ~500 lines of C++ code using LLVM API!
