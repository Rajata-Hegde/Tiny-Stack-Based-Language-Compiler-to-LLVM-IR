#!/bin/bash

# ============================================================
# Test script for Enhanced Stack Language Compiler
# Runs all sample programs and error test cases
# ============================================================

set -e

COMPILER="./compiler"
PASS=0
FAIL=0
TOTAL=0

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

run_test() {
    local name="$1"
    local file="$2"
    local expect_fail="$3"  # "yes" if we expect validation failure
    TOTAL=$((TOTAL + 1))

    echo ""
    echo "──────────────────────────────────────────"
    echo -e "${YELLOW}Test: ${name}${NC}"
    echo "  File: ${file}"
    echo "──────────────────────────────────────────"

    if [ "$expect_fail" == "yes" ]; then
        # We expect the compiler to reject this program
        if $COMPILER "$file" > /dev/null 2>&1; then
            echo -e "  ${RED}✗ FAIL${NC} — expected validation error, but compiler succeeded"
            FAIL=$((FAIL + 1))
        else
            echo -e "  ${GREEN}✓ PASS${NC} — correctly rejected invalid program"
            PASS=$((PASS + 1))
        fi
    else
        # We expect the compiler to succeed
        if $COMPILER "$file" > "${file%.sl}.ll" 2>/dev/null; then
            echo -e "  ${GREEN}✓ PASS${NC} — IR generated successfully"

            # Try to compile with clang if available
            if command -v clang &> /dev/null; then
                if clang "${file%.sl}.ll" -o "${file%.sl}.bin" 2>/dev/null; then
                    echo -e "  ${GREEN}✓ PASS${NC} — clang compiled IR to native binary"
                else
                    echo -e "  ${YELLOW}⚠ WARN${NC} — clang failed to compile IR"
                fi
            fi

            PASS=$((PASS + 1))
        else
            echo -e "  ${RED}✗ FAIL${NC} — compiler returned error"
            FAIL=$((FAIL + 1))
        fi
    fi
}

echo "=============================================="
echo "  Stack Language Compiler — Test Suite"
echo "=============================================="

# ----------------------------------------------------------
# Valid programs
# ----------------------------------------------------------
run_test "Simple addition (5+3)"               program1.sl no
run_test "Arithmetic chain (10*5/2)"            program2.sl no
run_test "Variables (x=10, x*2+5)"              program3.sl no
run_test "If-else control flow"                 program4.sl no
run_test "Print instruction"                    program5.sl no
run_test "Input + dup + print"                  program6.sl no
run_test "Stack ops: swap, dup, neg"            program7.sl no
run_test "Comparisons and modulo"               program8.sl no
run_test "Complex: mini calculator"             program9.sl no

# ----------------------------------------------------------
# Invalid programs (should be rejected)
# ----------------------------------------------------------
run_test "Error: stack underflow"               error1.sl yes
run_test "Error: unbalanced if/end"             error2.sl yes
run_test "Error: undefined variable (warning)"  error3.sl no  # warnings don't block

# ----------------------------------------------------------
# Summary
# ----------------------------------------------------------
echo ""
echo "=============================================="
echo "  Results: ${PASS}/${TOTAL} passed, ${FAIL} failed"
echo "=============================================="

if [ $FAIL -eq 0 ]; then
    echo -e "  ${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "  ${RED}Some tests failed.${NC}"
    exit 1
fi
