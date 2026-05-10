#!/bin/bash
cd '/mnt/d/git repos/Tiny-Stack-Based-Language-Compiler-to-LLVM-IR'

PASS=0
FAIL=0

run_valid() {
    echo "=== $1 ==="
    if ./compiler "$1" > /dev/null 2>&1; then
        echo "  PASS"
        PASS=$((PASS + 1))
    else
        echo "  FAIL"
        ./compiler "$1" > /dev/null 2>&2
        FAIL=$((FAIL + 1))
    fi
}

run_error() {
    echo "=== $1 (expect reject) ==="
    if ./compiler "$1" > /dev/null 2>&1; then
        echo "  FAIL (should have been rejected)"
        FAIL=$((FAIL + 1))
    else
        echo "  PASS (correctly rejected)"
        PASS=$((PASS + 1))
    fi
}

echo "======== VALID PROGRAMS ========"
run_valid program1.sl
run_valid program2.sl
run_valid program3.sl
run_valid program4.sl
run_valid program5.sl
run_valid program7.sl
run_valid program8.sl

echo ""
echo "======== ERROR PROGRAMS ========"
run_error error1.sl
run_error error2.sl

echo ""
echo "======== RESULTS ========"
echo "PASS: $PASS  FAIL: $FAIL"
