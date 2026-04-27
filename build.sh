#!/bin/bash

# Build script for Stack-Based Language Compiler
# This script compiles the project using LLVM

set -e  # Exit on any error

echo "======================================"
echo "Stack Language Compiler - Build Script"
echo "======================================"
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
echo ""

# Compile with LLVM support
g++ -fPIC \
    main.cpp lexer.cpp parser.cpp codegen.cpp \
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
    echo "  ./compiler <input_file.sl>"
    echo ""
    echo "Examples:"
    echo "  ./compiler program1.sl"
    echo "  ./compiler program2.sl"
    echo "  ./compiler program3.sl"
    echo "  ./compiler program4.sl"
else
    echo "✗ Build failed!"
    exit 1
fi
