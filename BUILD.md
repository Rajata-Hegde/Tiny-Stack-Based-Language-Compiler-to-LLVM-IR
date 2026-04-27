# Stack-Based Language Compiler - Build Instructions

## Prerequisites

### Install LLVM and necessary tools on WSL

```bash
# Update package manager
sudo apt update

# Install LLVM development libraries (choose one based on your needs)
# For latest stable LLVM (14+)
sudo apt install -y llvm-dev clang libllvm-dev

# Install build tools
sudo apt install -y build-essential g++ make
```

### Verify Installation

```bash
llvm-config --version
llvm-config --cxxflags
llvm-config --ldflags
llvm-config --system-libs
llvm-config --libs core
```

## Build Instructions

### Method 1: Using the Build Script (Recommended)

```bash
# Make the script executable
chmod +x build.sh

# Run the build script
./build.sh
```

### Method 2: Manual Compilation

```bash
# Compile all source files with LLVM
g++ -fPIC main.cpp lexer.cpp parser.cpp codegen.cpp \
    $(llvm-config --cxxflags --ldflags --system-libs --libs core) \
    -o compiler
```

### Method 3: Using a Makefile

Create a `Makefile` in the project directory and run:

```bash
make
```

## Running Programs

Once compiled, run the compiler with a program file:

```bash
./compiler program1.sl
./compiler program2.sl
./compiler program3.sl
./compiler program4.sl
```

## Program Examples

### program1.sl - Simple Addition
```
5 3 +
```
**Output**: Adds 5 and 3, result = 8

### program2.sl - Complex Expression
```
10 5 * 2 /
```
**Output**: (10 * 5) / 2 = 25

### program3.sl - Variables
```
x 10 = x 2 * 5 +
```
**Output**: Assigns 10 to x, then calculates x * 2 + 5 = 25

### program4.sl - If-Else Condition
```
x 15 = x 10 > if x 1 + else x 1 - end
```
**Output**: Assigns 15 to x, checks if x > 10 (true), returns x + 1 = 16

## Expected Output Format

When you run `./compiler program1.sl`, you'll see:

```
==== Input Program ====
5 3 +

==== Tokens ====
Token: NUMBER = "5"
Token: NUMBER = "3"
Token: PLUS = "+"

==== LLVM IR Output ====
; ModuleID = 'stack_lang'
source_filename = "stack_lang"

define i32 @main() {
entry:
  %add_result = add i32 5, 3
  ret i32 %add_result
}
```

## Troubleshooting

### "llvm-config not found"
```bash
# Install LLVM if not already installed
sudo apt install -y llvm-dev

# Or find the correct path
which llvm-config
```

### Compilation Errors
```bash
# Check LLVM installation
llvm-config --version

# Try with verbose output
g++ -v main.cpp lexer.cpp parser.cpp codegen.cpp \
    $(llvm-config --cxxflags --ldflags --system-libs --libs core) \
    -o compiler
```

### Runtime Errors
- Ensure input files exist in the current directory
- Check file permissions: `ls -la *.sl`
- Verify the compiler was built successfully: `./compiler --version` (if implemented)

## File Structure

```
.
├── compiler.h          # Header file with token/lexer/parser definitions
├── main.cpp            # Entry point - reads file and orchestrates compilation
├── lexer.cpp           # Tokenizer - converts input to tokens
├── parser.cpp          # Parser - processes tokens and generates LLVM IR
├── codegen.cpp         # Code generator - LLVM IR emission functions
├── program1.sl         # Sample program 1: simple arithmetic
├── program2.sl         # Sample program 2: multiple operations
├── program3.sl         # Sample program 3: variables
├── program4.sl         # Sample program 4: if-else condition
├── build.sh            # Build script for Linux/WSL
├── Makefile            # Alternative: build using make
└── BUILD.md            # This file
```

## Project Overview

### Architecture

```
Input File (.sl)
    ↓
Lexer (lexer.cpp)
    ↓ [Tokens]
Parser (parser.cpp)
    ↓ [Stack-based processing]
CodeGen (codegen.cpp)
    ↓ [LLVM IR]
Output (LLVM Module)
```

### Language Features

1. **Stack-based arithmetic**: `5 3 +` → pushes 5, pushes 3, adds them
2. **Operations**: `+`, `-`, `*`, `/`
3. **Variables**: `x 10 =` assigns 10 to x; `x` pushes x's value
4. **Comparison**: `>` (greater than)
5. **Control flow**: `if ... else ... end` blocks
6. **Immediate IR generation**: No AST - IR generated on-the-fly

### Technology Stack

- **Language**: C++11/14
- **Compiler**: g++ (any C++ compiler supporting LLVM)
- **LLVM Version**: 10.0+ recommended
- **Platform**: Linux, macOS, WSL (Windows Subsystem for Linux)

## Performance Notes

- The compiler generates valid LLVM IR that can be further optimized
- No optimization passes are run in this basic version
- Generated IR is printed to stdout in `.ll` format
- For production use, pipe output to `opt` for optimization and `llc` for code generation
