#!/bin/bash

# ============================================================
# Build script for Enhanced Stack-Based Language Compiler
# ============================================================

set -e  # Exit on any error

echo "=============================================="
echo "Stack Language Compiler - Enhanced Build Script"
echo "=============================================="
echo ""

# Check if llvm-config is available
if ! command -v llvm-config &> /dev/null; then
    echo "ERROR: llvm-config not found!"
    echo "Please install LLVM development files:"
    echo "  Ubuntu/Debian: sudo apt install -y llvm-dev"
    echo "  macOS: brew install llvm"
    exit 1
fi

echo "Detected LLVM version: $(llvm-config --version)"
echo ""

# Get LLVM compilation flags
LLVM_CXXFLAGS=$(llvm-config --cxxflags)
LLVM_LDFLAGS=$(llvm-config --ldflags)
LLVM_SYSTEM_LIBS=$(llvm-config --system-libs)
LLVM_LIBS=$(llvm-config --libs core)

echo "Compiling source files..."
echo "  - main.cpp"
echo "  - lexer.cpp"
echo "  - parser.cpp"
echo "  - codegen.cpp"
echo "  - validator.cpp"
echo ""

# Compile with LLVM support
g++ -fPIC \
    main.cpp lexer.cpp parser.cpp codegen.cpp validator.cpp \
    ${LLVM_CXXFLAGS} \
    ${LLVM_LDFLAGS} \
    ${LLVM_SYSTEM_LIBS} \
    ${LLVM_LIBS} \
    -o compiler

if [ -f compiler ]; then
    echo "✓ Build successful!"
    echo "  Executable: ./compiler"
    echo ""
    echo "Usage:"
    echo "  ./compiler <input_file.sl>              # Compile and print IR"
    echo "  ./compiler --validate-only <file.sl>    # Validate only"
    echo "  ./compiler --help                       # Show all options"
    echo ""
    echo "Run + Execute workflow:"
    echo "  ./compiler program.sl > output.ll       # Save IR to file"
    echo "  clang output.ll -o program              # Compile IR to native"
    echo "  ./program                               # Run the program"
    echo ""
    echo "Example programs:"
    echo "  ./compiler program1.sl    # Simple addition"
    echo "  ./compiler program2.sl    # Arithmetic chain"
    echo "  ./compiler program3.sl    # Variables"
    echo "  ./compiler program4.sl    # If-else control flow"
    echo "  ./compiler program5.sl    # I/O: print"
    echo "  ./compiler program6.sl    # I/O: input + print"
    echo "  ./compiler program7.sl    # Stack ops: dup, swap, neg"
    echo "  ./compiler program8.sl    # Comparisons"
    echo "  ./compiler program9.sl    # Complex: calculator"
    echo ""
    echo "Error test programs:"
    echo "  ./compiler error1.sl      # Stack underflow"
    echo "  ./compiler error2.sl      # Unbalanced if/end"
    echo "  ./compiler error3.sl      # Undefined variable"
else
    echo "✗ Build failed!"
    exit 1
fi
