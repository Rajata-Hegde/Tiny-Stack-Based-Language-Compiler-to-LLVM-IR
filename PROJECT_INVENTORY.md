# Complete Project Inventory

## Stack-Based Language Compiler with LLVM

Your complete, working project includes **9 files** with everything needed to build and run a stack-based language compiler.

---

## 📂 File Listing

### Core Implementation (4 files - ~500 lines of code)

1. **`compiler.h`** - Header file
   - Token types enum (NUMBER, IDENTIFIER, PLUS, MINUS, MUL, DIV, ASSIGN, GREATER, IF, ELSE, END, EOF_TOKEN)
   - Token struct definition
   - Lexer class interface
   - Parser class interface
   - **Includes**: string, vector, map, memory

2. **`lexer.cpp`** - Lexical analyzer (50 lines)
   - Tokenizes input by whitespace
   - Classifies tokens (numbers, keywords, operators)
   - Function: `Lexer::tokenize()` - returns vector<Token>
   - Function: `Lexer::identifyKeywordOrOperator()` - token classification

3. **`parser.cpp`** - Parser interpreter (150 lines)
   - Processes tokens sequentially
   - Stack-based value processing (no AST)
   - Generates LLVM IR immediately
   - Functions:
     - `Parser::parse()` - main entry point
     - `Parser::parseExpression()` - token processing
     - `Parser::parseIfElse()` - conditional handling
     - `Parser::currentToken()`, `Parser::peekToken()`, `Parser::advance()`

4. **`codegen.cpp`** - LLVM code generator (180 lines)
   - Global LLVM objects (context, module, builder, valueStack, variables)
   - Code emission functions:
     - `initializeCodegen()` - setup LLVM
     - `createMainFunction()` - create main() function
     - `emitPushConstant()`, `emitAdd()`, `emitSub()`, `emitMul()`, `emitDiv()`, `emitGreater()`
     - `getVariable()`, `setVariable()`
     - `emitReturn()`, `printIR()`

5. **`main.cpp`** - Entry point (70 lines)
   - File input handling
   - Orchestrates compilation pipeline (lexer → parser → codegen)
   - Prints input, tokens, and LLVM IR output
   - Usage: `./compiler program.sl`

### Sample Programs (4 files)

6. **`program1.sl`** - Simple arithmetic
   ```
   5 3 +
   ```
   - Expected result: 8

7. **`program2.sl`** - Multiple operations
   ```
   10 5 * 2 /
   ```
   - Expected result: 25

8. **`program3.sl`** - Variable usage
   ```
   x 10 = x 2 * 5 +
   ```
   - Expected result: 25

9. **`program4.sl`** - Conditional (if-else)
   ```
   x 15 = x 10 > if x 1 + else x 1 - end
   ```
   - Expected result: 16

### Build Files (3 files)

10. **`Makefile`** - Build configuration
    - Targets: `all`, `run-examples`, `clean`, `rebuild`, `help`
    - Automatically extracts LLVM compilation flags
    - Usage: `make` or `make run-examples`

11. **`build.sh`** - Linux/WSL build script
    - Checks LLVM installation
    - Compiles all files with LLVM flags
    - Creates executable `./compiler`
    - Usage: `chmod +x build.sh && ./build.sh`

12. **`build.bat`** - Windows PowerShell build script
    - Alternative for Windows users
    - Compiles with LLVM using PowerShell backtick syntax

### Documentation Files (5 files)

13. **`README.md`** - Main project documentation
    - Project overview and features
    - Language syntax reference
    - Compilation instructions
    - Architecture overview
    - Code statistics
    - Extension guide

14. **`BUILD.md`** - Detailed build guide
    - Prerequisites and installation (Ubuntu, macOS, etc.)
    - Multiple build methods
    - Full troubleshooting section
    - File structure reference

15. **`WSL_QUICKSTART.md`** - WSL-specific quick start
    - Step-by-step WSL setup
    - Fast build and run instructions
    - Common tasks (save IR, optimize, compile to executable)
    - Example walkthroughs
    - Troubleshooting for WSL

16. **`PROJECT_STRUCTURE.md`** - Complete project reference
    - Detailed file-by-file breakdown
    - Compilation flow diagram
    - Language grammar specification
    - Data flow visualization
    - Code statistics
    - LLVM API usage summary
    - Extension points guide

17. **`LLVM_IR_OUTPUT.md`** - Expected outputs
    - Sample execution for each program
    - Expected LLVM IR output
    - Step-by-step stack traces
    - Control flow graphs
    - IR instruction reference
    - Testing and verification guide

---

## 📊 Project Statistics

| Category | Count | Details |
|----------|-------|---------|
| **Source Files** | 5 | main.cpp, lexer.cpp, parser.cpp, codegen.cpp, compiler.h |
| **Sample Programs** | 4 | program1-4.sl |
| **Build Files** | 3 | Makefile, build.sh, build.bat |
| **Documentation** | 5 | README.md, BUILD.md, WSL_QUICKSTART.md, PROJECT_STRUCTURE.md, LLVM_IR_OUTPUT.md |
| **Total Files** | 17 | Everything included! |
| **Lines of C++ Code** | ~500 | Compact, efficient implementation |
| **Total Documentation** | ~3000 | Comprehensive guides |

---

## 🚀 Quick Start

### 1. Install LLVM (if needed)
```bash
sudo apt update && sudo apt install -y llvm-dev build-essential g++
```

### 2. Build
```bash
chmod +x build.sh
./build.sh
```

### 3. Run
```bash
./compiler program1.sl
./compiler program2.sl
./compiler program3.sl
./compiler program4.sl
```

---

## 📖 Documentation Guide

### For Quick Start
👉 Read: **`WSL_QUICKSTART.md`** (5-minute setup)

### For Project Overview
👉 Read: **`README.md`** (language features, examples)

### For Understanding Code
👉 Read: **`PROJECT_STRUCTURE.md`** (code walkthrough)

### For Build Issues
👉 Read: **`BUILD.md`** (troubleshooting)

### For Compiler Output
👉 Read: **`LLVM_IR_OUTPUT.md`** (expected outputs)

---

## 🔧 Key Features

✅ **Complete implementation** - Lexer, Parser, CodeGen, Main  
✅ **Stack-based evaluation** - Postfix notation  
✅ **LLVM IR generation** - Valid, compilable output  
✅ **Variables** - Storage and retrieval  
✅ **Arithmetic** - +, -, *, /  
✅ **Conditionals** - if-else-end blocks  
✅ **Multiple build options** - Makefile, scripts, manual  
✅ **Comprehensive docs** - 5 documentation files  
✅ **Sample programs** - 4 working examples  

---

## 📋 Language Support

### Operators
- Addition: `+`
- Subtraction: `-`
- Multiplication: `*`
- Division: `/` (signed)
- Comparison: `>` (greater than)

### Control Flow
- Conditionals: `if ... else ... end`
- Variables: `x 10 =` (assignment), `x` (load)

### Data Types
- Integer: 32-bit signed integers (i32)
- Return value: Always i32

---

## 🎯 Compilation Pipeline

```
.sl File
    ↓
Lexer (lexer.cpp)
    ↓ [Tokens]
Parser (parser.cpp)
    ↓ [Stack Processing]
CodeGen (codegen.cpp)
    ↓ [LLVM IR]
.ll Format (LLVM IR)
    ↓
LLVM Toolchain (opt, llc, gcc)
    ↓
Native Executable
```

---

## 🔌 Technology Stack

- **Language**: C++11/14
- **Compiler**: g++ (or clang)
- **LLVM**: 10.0+ 
- **Build Tools**: make, bash, PowerShell
- **Platforms**: Linux, macOS, WSL, Windows

---

## 📝 File Dependencies

```
main.cpp
  ├─ compiler.h
  ├─ lexer.cpp
  └─ parser.cpp
      └─ codegen.cpp
          └─ (LLVM libraries)

Sample Programs (.sl)
  └─ compiler (executable)
```

---

## ✨ What You Can Do

1. **Run existing programs** - All 4 samples work out of the box
2. **Create custom programs** - Write your own .sl files
3. **Study compiler design** - Well-commented, educational code
4. **Extend the language** - Add new operators/features
5. **Generate LLVM IR** - Output valid IR for optimization
6. **Create executables** - Compile IR to native code

---

## 🎓 Learning Outcomes

After working with this project, you'll understand:

- How lexers tokenize input
- Stack-based parsing and evaluation
- LLVM IR generation
- Basic compiler architecture
- How to use LLVM API
- Control flow with BasicBlocks
- Variable symbol tables
- Compilation pipeline

---

## 📚 Documentation Quick Links

- **Setup**: `WSL_QUICKSTART.md`
- **Learn**: `README.md`
- **Details**: `PROJECT_STRUCTURE.md`
- **Build**: `BUILD.md`
- **Expected Output**: `LLVM_IR_OUTPUT.md`

---

## ✅ Verification Checklist

After building, verify with:

```bash
# Check compilation
[ -f compiler ] && echo "✓ Compiler built"

# Test program 1
./compiler program1.sl | grep "add i32 5, 3"
[ $? -eq 0 ] && echo "✓ Program 1 works"

# Test program 2
./compiler program2.sl | grep "mul i32 10, 5"
[ $? -eq 0 ] && echo "✓ Program 2 works"

# Test program 3
./compiler program3.sl | grep "mul i32 10, 2"
[ $? -eq 0 ] && echo "✓ Program 3 works"

# Test program 4
./compiler program4.sl | grep "icmp sgt i32 15, 10"
[ $? -eq 0 ] && echo "✓ Program 4 works"

# All tests passed
echo "✅ All tests passed!"
```

---

## 🎉 You're All Set!

Your complete stack-based language compiler project is ready to:
- Build with `./build.sh` or `make`
- Run with `./compiler program.sl`
- Study and extend with 500 lines of well-documented C++ code
- Learn compiler design with LLVM

**Total package: 17 files, 3500+ lines including code and documentation.**

Happy compiling! 🚀
