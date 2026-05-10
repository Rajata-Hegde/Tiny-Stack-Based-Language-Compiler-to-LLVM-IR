# Tiny Stack-Based Language Compiler to LLVM IR — Enhanced Edition

A compiler for a small stack-based language that generates LLVM IR, featuring I/O operations, rich error diagnostics, pre-codegen validation, and extended instructions.

## Architecture

```
Source (.sl) ──► Lexer ──► Tokens ──► Validator ──► Parser + CodeGen ──► LLVM IR (.ll)
                  │                     │                │
            line/column          stack simulation    error recovery
            tracking             underflow check     diagnostics
                                 var-def check
                                 if/end balance
```

| File           | Purpose                                                |
|----------------|--------------------------------------------------------|
| `compiler.h`   | Token types, Lexer/Parser/Validator/Diagnostic classes |
| `lexer.cpp`    | Char-by-char scanner with line/column tracking         |
| `validator.cpp` | Simulated stack execution for semantic checks         |
| `parser.cpp`   | Recursive-descent parser with LLVM IR emission         |
| `codegen.cpp`  | LLVM IRBuilder wrappers, I/O via printf/scanf          |
| `main.cpp`     | CLI driver with validation + codegen pipeline          |

## Language Reference

### Data Types
- **Integer (i32)** — the only data type; all values are 32-bit signed integers.

### Instructions

| Instruction | Stack Effect         | Description                            |
|-------------|----------------------|----------------------------------------|
| `<number>`  | → n                  | Push integer literal                   |
| `<id>`      | → val                | Push variable value (or begin assign)  |
| `<id> = `   | val →                | Pop TOS and assign to variable         |
| `+`         | a b → (a+b)          | Addition                               |
| `-`         | a b → (a−b)          | Subtraction                            |
| `*`         | a b → (a×b)          | Multiplication                         |
| `/`         | a b → (a÷b)          | Signed integer division                |
| `%`         | a b → (a mod b)      | Modulo (remainder)                     |
| `>`         | a b → (a>b)          | Greater-than comparison (0/1)          |
| `<`         | a b → (a<b)          | Less-than comparison (0/1)             |
| `==`        | a b → (a==b)         | Equality comparison (0/1)              |
| `!=`        | a b → (a≠b)          | Not-equal comparison (0/1)             |
| `neg`       | a → (−a)             | Negate top of stack                    |
| `dup`       | a → a a              | Duplicate top of stack                 |
| `swap`      | a b → b a            | Swap top two values                    |
| `print`     | a →                  | Pop and print value with newline       |
| `input`     | → n                  | Read integer from stdin, push it       |
| `if`        | cond →               | Pop condition; branch to then/else     |
| `else`      | —                    | Start else branch                      |
| `end`       | —                    | Close if/else block                    |

### Assignment Syntax
```
x 10 =           // assign 10 to x
x 2 * 5 + y =    // assign (x*2 + 5) to y
```

### Control Flow
```
<condition> if
    <then-body>
else
    <else-body>
end
```

## Building

### Prerequisites
- GCC/G++ with C++17
- LLVM development libraries (`llvm-dev`)

### Build
```bash
chmod +x build.sh
./build.sh
```

### Verify
```bash
./compiler program5.sl > output.ll    # Generate IR
clang output.ll -o program5           # Compile to native
./program5                            # Run — prints "8"
```

## Usage

```bash
# Compile and print LLVM IR to stdout
./compiler program.sl

# Save IR to file
./compiler program.sl > output.ll

# Validate only (no IR generation)
./compiler --validate-only program.sl

# Show help
./compiler --help
```

## Example Programs

### program1.sl — Simple Addition
```
5 3 +
```
Returns exit code 8.

### program5.sl — Print Result
```
5 3 + print
```
Prints `8` to stdout.

### program6.sl — Input/Output
```
input dup print 10 + print
```
Reads an integer, prints it, adds 10, prints the sum.

### program7.sl — Stack Manipulation
```
10 20 swap print print
42 dup print print
7 neg print
```
Prints: `10`, `20`, `42`, `42`, `-7`

### program8.sl — Comparisons & Modulo
```
10 5 > print
3 7 < print
5 5 == print
4 9 != print
10 3 % print
```
Prints: `1`, `1`, `1`, `1`, `1`

### program9.sl — Mini Calculator
```
input x =
input y =
x y + dup print
x y - print
x y * print
x y > if x print else y print end
```

## Error Handling

The compiler reports errors with **line and column numbers**:

```
error [1:1]: Stack underflow: '+' requires 2 value(s) on the stack, but only 0 available.
```

### Validation Checks
- **Stack underflow** — operations requiring more values than available
- **Undefined variables** — warning when reading before assignment
- **Unbalanced if/end** — missing `end` or `else` without `if`
- **Bare assignment** — `=` without a preceding identifier

### Error Test Programs
| File       | Error Type                        |
|------------|-----------------------------------|
| error1.sl  | Stack underflow on `+`            |
| error2.sl  | Unbalanced `if`/`else` (no `end`) |
| error3.sl  | Undefined variable warning        |

## Testing

```bash
chmod +x test.sh
./test.sh
```

Runs all valid programs (expects success) and error programs (expects rejection), then prints a pass/fail summary.

## Generated LLVM IR Example

For `5 3 + print`:
```llvm
; ModuleID = 'stack_lang'

@fmt_print = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)
declare i32 @scanf(i8*, ...)

define i32 @main() {
entry:
  %add_result = add i32 5, 3
  %printf_call = call i32 (i8*, ...) @printf(i8* getelementptr (...), i32 %add_result)
  ret i32 0
}
```

## Project Structure

```
├── compiler.h        # Header: tokens, classes, diagnostics
├── lexer.cpp         # Lexical analysis with position tracking
├── validator.cpp     # Pre-codegen semantic validation
├── parser.cpp        # Recursive-descent parser + IR emission
├── codegen.cpp       # LLVM IR generation (IRBuilder wrappers)
├── main.cpp          # CLI driver
├── build.sh          # Build script
├── test.sh           # Automated test runner
├── program[1-9].sl   # Sample programs
└── error[1-3].sl     # Error test cases
```
