# Expected LLVM IR Output Examples

This document shows the exact LLVM IR output you should expect from running each sample program.

## Program 1: Simple Addition

**File**: `program1.sl`
```
5 3 +
```

### Execution
```bash
./compiler program1.sl
```

### Expected Full Output
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

### Explanation

1. **Tokens**: Parser identifies three tokens:
   - NUMBER with value "5"
   - NUMBER with value "3"
   - PLUS operator

2. **IR Generation**:
   - `%add_result = add i32 5, 3` - Add two i32 constants, store result in %add_result
   - `ret i32 %add_result` - Return the addition result (which is 8)

3. **Result**: Function returns **8**

---

## Program 2: Multiple Operations

**File**: `program2.sl`
```
10 5 * 2 /
```

### Execution
```bash
./compiler program2.sl
```

### Expected LLVM IR
```
; ModuleID = 'stack_lang'
source_filename = "stack_lang"

define i32 @main() {
entry:
  %mul_result = mul i32 10, 5
  %div_result = sdiv i32 %mul_result, 2
  ret i32 %div_result
}
```

### Step-by-Step Stack Trace

```
Stack: []
  Push 10    → Stack: [10]
  Push 5     → Stack: [10, 5]
  Multiply   → Stack: [50]         ; IR: %mul_result = mul i32 10, 5
  Push 2     → Stack: [50, 2]
  Divide     → Stack: [25]         ; IR: %div_result = sdiv i32 50, 2
  Return 25
```

### Explanation

1. First instruction multiplies: 10 * 5 = 50
2. Second instruction divides: 50 / 2 = 25
3. Returns **25**

---

## Program 3: Variables

**File**: `program3.sl`
```
x 10 = x 2 * 5 +
```

### Execution
```bash
./compiler program3.sl
```

### Expected LLVM IR
```
; ModuleID = 'stack_lang'
source_filename = "stack_lang"

define i32 @main() {
entry:
  %mul_result = mul i32 10, 2
  %add_result = add i32 %mul_result, 5
  ret i32 %add_result
}
```

### Stack Trace with Variables

```
Variables: {}
Stack: []

x 10 =
  Push 10                    → Stack: [10]
  Assign to x                → Variables: {x: 10}, Stack: []

x 2 *
  Load x                     → Stack: [10]
  Push 2                     → Stack: [10, 2]
  Multiply                   → Stack: [20]

5 +
  Push 5                     → Stack: [20, 5]
  Add                        → Stack: [25]

Return 25
```

### Explanation

1. **Assignment**: `x 10 =` stores 10 in variable x
2. **Multiplication**: Load x (10), multiply by 2 → 20
3. **Addition**: Add 5 to result → 25
4. **Result**: Returns **25**

---

## Program 4: Conditional (If-Else)

**File**: `program4.sl`
```
x 15 = x 10 > if x 1 + else x 1 - end
```

### Execution
```bash
./compiler program4.sl
```

### Expected LLVM IR
```
; ModuleID = 'stack_lang'
source_filename = "stack_lang"

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
  ret i32 %add_result
}
```

### Control Flow Graph

```
                    ┌─────────┐
                    │ entry   │
                    │ x = 15  │
                    │ cmp     │
                    └────┬────┘
                        /\
                       /  \
                      /    \
              true   /      \  false
                    /        \
            ┌──────▼──┐    ┌──▼────────┐
            │ then    │    │ else      │
            │ x + 1   │    │ x - 1     │
            │ = 16    │    │ = 14      │
            └────┬────┘    └──┬────────┘
                 \            /
                  \          /
                   \        /
                    └──┬───┘
                       │
                    ┌──▼──────┐
                    │ end_if  │
                    │ return  │
                    └─────────┘
```

### Stack Trace with Conditional

```
Variables: {}
Stack: []

x 15 =
  Push 15                    → Stack: [15]
  Assign to x                → Variables: {x: 15}, Stack: []

x 10 > (comparison)
  Load x                     → Stack: [15]
  Push 10                    → Stack: [15, 10]
  Compare sgt (15 > 10?)     → Stack: [true]  (in LLVM: i1 1)

if (branch on condition)
  Condition is true          → Enter 'then' block

then:
  Load x                     → Stack: [15]
  Push 1                     → Stack: [15, 1]
  Add                        → Stack: [16]
  Branch to end_if

end_if:
  Return 16
```

### Explanation

1. **Assignment**: `x 15 =` stores 15 in x
2. **Comparison**: `x 10 >` compares 15 > 10 → **true**
3. **Conditional Branch**: Since condition is true:
   - Enters `then` block
   - Calculates x + 1 = 16
   - Branches to end_if
4. **Result**: Returns **16**

### Alternative: If Condition is False

If the program was:
```
x 5 = x 10 > if x 1 + else x 1 - end
```

Then:
```
Comparison: 5 > 10 → false
Branch to 'else' block
Calculate: 5 - 1 = 4
Return: 4
```

---

## LLVM IR Instructions Reference

### Arithmetic Instructions

```llvm
%result = add i32 %a, %b      ; Addition
%result = sub i32 %a, %b      ; Subtraction
%result = mul i32 %a, %b      ; Multiplication
%result = sdiv i32 %a, %b     ; Signed integer division
```

### Comparison Instructions

```llvm
%cond = icmp sgt i32 %a, %b   ; Signed greater than (a > b)
%cond = icmp slt i32 %a, %b   ; Signed less than (a < b)
%cond = icmp eq i32 %a, %b    ; Equal
%cond = icmp ne i32 %a, %b    ; Not equal
```

### Control Flow Instructions

```llvm
br label %target              ; Unconditional branch
br i1 %cond, label %then, label %else  ; Conditional branch
ret i32 %value                ; Return value
```

### BasicBlock Structure

```llvm
label_name:                   ; Label
  instructions...
  terminator                  ; Must end with br or ret
```

---

## Testing Generated IR

### Verify Output Correctness

```bash
# Generate IR and save to file
./compiler program1.sl > output.ll

# View the IR
cat output.ll

# Validate IR syntax
llvm-as output.ll -o output.bc  # Assemble to bitcode
# If this succeeds, your IR is valid!
```

### Compile IR to Executable

```bash
# Step 1: Generate IR
./compiler program1.sl > prog.ll

# Step 2: Compile to assembly
llc prog.ll -o prog.s

# Step 3: Assemble to object file
as prog.s -o prog.o

# Step 4: Link to create executable
gcc prog.o -lc -o prog_exec

# Step 5: Run and check exit code
./prog_exec
echo $?  # Print exit code (should be 8 for program1)
```

### Optimize and Compare

```bash
# Original IR
./compiler program1.sl > original.ll

# Optimize
opt -O3 original.ll -o optimized.ll

# Compare
diff original.ll optimized.ll

# Disassemble optimized bitcode
llvm-dis optimized.bc -o optimized.ll
```

---

## Common IR Patterns

### Constant Push
```
Stack: [N]
```
```llvm
; No IR generated - constant folding handles it
```

### Binary Operation
```
Stack: [a, b, OP] → Stack: [result]
```
```llvm
%result = <op> i32 %a, %b
; e.g., %result = add i32 %a, %b
```

### Variable Assignment
```
x 10 = 
```
```llvm
; Variable stored in memory map, no IR instruction
```

### Variable Load
```
x
```
```llvm
; Value retrieved from memory map, becomes SSA value
```

### If-Else Block
```
cond if body1 else body2 end
```
```llvm
br i1 %cond, label %then, label %else
then:
  ; body1 instructions
  br label %end_if
else:
  ; body2 instructions
  br label %end_if
end_if:
  ; continue
```

---

## Debugging Output

### If output doesn't match expected:

1. **Check token stream**: Verify lexer produces correct tokens
   ```
   ==== Tokens ====
   Token: NUMBER = "5"
   Token: NUMBER = "3"
   Token: PLUS = "+"
   ```

2. **Check IR syntax**: Validate with llvm-as
   ```bash
   ./compiler program1.sl > test.ll
   llvm-as test.ll  # Will report syntax errors
   ```

3. **Check result**: Run compiled program
   ```bash
   llc test.ll -o test.s
   gcc test.s -o test_exe
   ./test_exe; echo $?  # Print exit code
   ```

---

## Advanced: Examining Optimization

```bash
# Generate and optimize
./compiler program1.sl > orig.ll
opt -O2 orig.ll -S -o opt.ll

# Compare results
cat orig.ll
echo "---"
cat opt.ll

# Example: O2 optimization might fold constants
# Before: add i32 5, 3
# After:  (constant 8)
```

---

This document provides all expected outputs. Use it to verify your compiler is working correctly!
