# WSL Quick Start Guide - Stack Language Compiler

## Step 1: Verify WSL Environment

```bash
# Check if you're in WSL
uname -a
# Output should show: Linux ... microsoft ... #... SMP ... (Microsoft)

# Check LLVM installation
llvm-config --version
# If not found, proceed to Step 2
```

## Step 2: Install LLVM (if not already installed)

```bash
# Update package list
sudo apt update

# Install LLVM development libraries and build tools
sudo apt install -y llvm-dev build-essential g++ make

# Verify installation
llvm-config --version
```

Expected output: `14.0.6` (or similar version number)

## Step 3: Clone/Setup Project

```bash
# Navigate to your project directory
cd /path/to/llvm

# View project files
ls -la
# Should show: compiler.h, main.cpp, lexer.cpp, parser.cpp, codegen.cpp, program*.sl
```

## Step 4: Build the Compiler

**Option A: Using build script (Recommended)**
```bash
chmod +x build.sh
./build.sh
```

**Option B: Using make**
```bash
make
```

**Option C: Manual compilation**
```bash
g++ -fPIC main.cpp lexer.cpp parser.cpp codegen.cpp \
    $(llvm-config --cxxflags --ldflags --system-libs --libs core) \
    -o compiler
```

Expected output:
```
✓ Build successful!
  Executable: ./compiler
```

## Step 5: Test the Compiler

```bash
# Run first sample program
./compiler program1.sl

# Expected output shows:
# ==== Input Program ====
# 5 3 +
# 
# ==== Tokens ====
# ...
# 
# ==== LLVM IR Output ====
# define i32 @main() {
#   %add_result = add i32 5, 3
#   ret i32 %add_result
# }
```

## Step 6: Run All Samples

```bash
# Using make
make run-examples

# Or manually:
echo "=== Program 1 ===" && ./compiler program1.sl
echo "=== Program 2 ===" && ./compiler program2.sl
echo "=== Program 3 ===" && ./compiler program3.sl
echo "=== Program 4 ===" && ./compiler program4.sl
```

## Common Tasks

### Save IR output to file

```bash
./compiler program1.sl > output.ll
cat output.ll
```

### Create custom program

```bash
# Create new program file
echo "7 2 + 3 *" > myprog.sl

# Compile it
./compiler myprog.sl
```

### Optimize generated IR

```bash
# First, save IR to file
./compiler program1.sl > program1.ll

# Then optimize with LLVM opt tool (if installed)
opt -O2 program1.ll -S -o program1_opt.ll
cat program1_opt.ll
```

### Compile IR to executable

```bash
# Generate IR
./compiler program1.sl > program1.ll

# Convert LLVM IR to object file
llc program1.ll -o program1.s

# Or directly to object:
llc -filetype=obj program1.ll -o program1.o

# Link with C runtime to create executable
gcc program1.o -o program1_exe
./program1_exe
echo $?  # Print exit code (return value from main)
```

## Troubleshooting

### Build Fails with "llvm-config not found"

```bash
# Install LLVM dev libraries
sudo apt install -y llvm-dev

# Or find the full path to llvm-config
which llvm-config
find /usr -name llvm-config 2>/dev/null

# Use full path if not in PATH
/usr/lib/llvm-14/bin/llvm-config --version
```

### Undefined reference errors during linking

```bash
# Make sure LLVM libraries are being linked
llvm-config --libs core

# Try rebuilding with more verbose output
g++ -v main.cpp lexer.cpp parser.cpp codegen.cpp \
    $(llvm-config --cxxflags --ldflags --system-libs --libs core) \
    -o compiler 2>&1 | head -50
```

### Permission denied when running build.sh

```bash
# Make it executable
chmod +x build.sh

# Run it
./build.sh
```

## File Structure After Build

```
llvm/
├── compiler           ← Executable (after build)
├── compiler.h
├── main.cpp
├── lexer.cpp
├── parser.cpp
├── codegen.cpp
├── program1.sl        (5 3 +)
├── program2.sl        (10 5 * 2 /)
├── program3.sl        (x 10 = x 2 * 5 +)
├── program4.sl        (x 15 = x 10 > if x 1 + else x 1 - end)
├── build.sh
├── Makefile
├── BUILD.md
└── README.md
```

## Example Walkthrough

### Creating and running a new program

```bash
# Create a program that calculates: (20 + 5) * 2 = 50
cat > calc.sl << 'EOF'
20 5 + 2 *
EOF

# Compile and run
./compiler calc.sl

# Output will show the generated LLVM IR:
# define i32 @main() {
# entry:
#   %add_result = add i32 20, 5
#   %mul_result = mul i32 %add_result, 2
#   ret i32 %mul_result
# }

# The program returns 50
```

### Using variables

```bash
cat > vars.sl << 'EOF'
x 100 =
y 50 =
x y - 2 /
EOF

./compiler vars.sl
# Returns: (100 - 50) / 2 = 25
```

### Using conditionals

```bash
cat > cond.sl << 'EOF'
n 42 =
n 50 >
if
  1
else
  0
end
EOF

./compiler cond.sl
# Returns: 0 (because 42 is not > 50)
```

## Next Steps

1. **Explore the code**: Read through each `.cpp` file with comments
2. **Add features**: Implement new operators or language constructs
3. **Optimize**: Add LLVM optimization passes to generated IR
4. **Extend**: Add loops, function definitions, or other features
5. **Study**: Compare generated LLVM IR with C/C++ compiled code

## Useful Commands

```bash
# Check LLVM version
llvm-config --version

# Get LLVM C++ flags
llvm-config --cxxflags

# Get linking flags
llvm-config --ldflags

# List available LLVM libraries
llvm-config --libs

# View LLVM installation details
llvm-config --prefix
```

## Getting Help

- **LLVM Documentation**: https://llvm.org/docs/
- **LLVM IR Language Reference**: https://llvm.org/docs/LangRef/
- **IRBuilder Reference**: https://llvm.org/doxygen/classllvm_1_1IRBuilder.html

Happy coding! 🚀
