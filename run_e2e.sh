#!/bin/bash
cd '/mnt/d/git repos/Tiny-Stack-Based-Language-Compiler-to-LLVM-IR'

echo "===== End-to-End: Compile IR -> Native Binary -> Execute ====="
echo ""

# Test program5: 5 3 + print  (should print 8)
echo "--- program5.sl: '5 3 + print' ---"
./compiler program5.sl > program5.ll 2>/dev/null
clang program5.ll -o program5.bin 2>&1
echo -n "Output: "
./program5.bin
echo ""

# Test program7: swap, dup, neg  (should print 10, 20, 42, 42, -7)
echo "--- program7.sl: 'swap, dup, neg' ---"
./compiler program7.sl > program7.ll 2>/dev/null
clang program7.ll -o program7.bin 2>&1
echo "Output:"
./program7.bin
echo ""

# Test program8: comparisons  (should print 1, 1, 1, 1, 1)
echo "--- program8.sl: 'comparisons + modulo' ---"
./compiler program8.sl > program8.ll 2>/dev/null
clang program8.ll -o program8.bin 2>&1
echo "Output:"
./program8.bin
echo ""

# Test program1: 5 3 +  (exit code should be 8)
echo "--- program1.sl: '5 3 +' (return via exit code) ---"
./compiler program1.sl > program1.ll 2>/dev/null
clang program1.ll -o program1.bin 2>&1
./program1.bin
echo "Exit code: $?"
echo ""

# Test program3: variables
echo "--- program3.sl: 'x 10 = x 2 * 5 +' (exit code should be 25) ---"
./compiler program3.sl > program3.ll 2>/dev/null
clang program3.ll -o program3.bin 2>&1
./program3.bin
echo "Exit code: $?"
echo ""

# Show the generated IR for program5 as a sample
echo "===== Sample Generated LLVM IR (program5.sl) ====="
cat program5.ll
