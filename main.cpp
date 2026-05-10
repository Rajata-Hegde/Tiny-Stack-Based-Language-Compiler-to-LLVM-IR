#include "compiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

// ============================================================
// Read entire file into a string
// ============================================================
static std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ============================================================
// Print tokens in human-readable form
// ============================================================
static void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        if (token.type == TokenType::EOF_TOKEN) {
            std::cerr << "  [" << token.line << ":" << token.column
                      << "]  EOF\n";
            continue;
        }
        std::cerr << "  [" << token.line << ":" << token.column << "]  "
                  << tokenTypeName(token.type)
                  << "  \"" << token.value << "\"\n";
    }
}

// ============================================================
// Print diagnostics
// ============================================================
static void printDiagnostics(const std::vector<Diagnostic>& diags) {
    for (const auto& d : diags) {
        std::cerr << "  " << d.toString() << "\n";
    }
}

// ============================================================
// Main entry point
// ============================================================
int main(int argc, char* argv[]) {
    // ----------------------------------------------------------
    // Parse command-line arguments
    // ----------------------------------------------------------
    bool validateOnly = false;
    std::string filename;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--validate-only") == 0) {
            validateOnly = true;
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << "Stack Language Compiler — Enhanced Edition\n\n"
                      << "Usage: " << argv[0] << " [options] <input_file.sl>\n\n"
                      << "Options:\n"
                      << "  --validate-only   Run validation only (no IR generation)\n"
                      << "  --help, -h        Show this help message\n\n"
                      << "Supported instructions:\n"
                      << "  <number>          Push integer constant\n"
                      << "  <identifier>      Push variable value / begin assignment\n"
                      << "  + - * / %         Arithmetic (pop 2, push result)\n"
                      << "  > < == !=         Comparison (pop 2, push bool)\n"
                      << "  =                 Assignment: <id> <expr> =\n"
                      << "  if ... else ... end   Conditional execution\n"
                      << "  print             Pop & print top-of-stack\n"
                      << "  input             Read integer from stdin, push\n"
                      << "  dup               Duplicate top-of-stack\n"
                      << "  swap              Swap top two values\n"
                      << "  neg               Negate top-of-stack\n";
            return 0;
        } else {
            filename = argv[i];
        }
    }

    if (filename.empty()) {
        std::cerr << "Usage: " << argv[0] << " [--validate-only] <input_file.sl>\n";
        std::cerr << "Try '" << argv[0] << " --help' for more information.\n";
        return 1;
    }

    // ----------------------------------------------------------
    // Read source file
    // ----------------------------------------------------------
    std::string input = readFile(filename);
    if (input.empty()) {
        return 1;
    }

    std::cerr << "==== Input Program (" << filename << ") ====\n";
    std::cerr << input << "\n";

    // ----------------------------------------------------------
    // Step 1: Lexical analysis
    // ----------------------------------------------------------
    std::cerr << "\n==== Tokens ====\n";
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();
    printTokens(tokens);

    // ----------------------------------------------------------
    // Step 2: Validation (pre-codegen semantic checks)
    // ----------------------------------------------------------
    std::cerr << "\n==== Validation ====\n";
    Validator validator(tokens);
    bool valid = validator.validate();

    const auto& valDiags = validator.diagnostics();
    if (!valDiags.empty()) {
        printDiagnostics(valDiags);
    }

    if (valid) {
        std::cerr << "  Validation passed — no errors.\n";
    } else {
        std::cerr << "  Validation FAILED — " << valDiags.size()
                  << " diagnostic(s) reported.\n";
    }

    if (validateOnly) {
        return valid ? 0 : 1;
    }

    if (!valid) {
        std::cerr << "\nAborting code generation due to validation errors.\n";
        return 1;
    }

    // ----------------------------------------------------------
    // Step 3: Parse & Generate LLVM IR
    // ----------------------------------------------------------
    std::cerr << "\n==== LLVM IR Output ====\n";

    Parser parser(tokens);
    bool parseOk = parser.parse();

    const auto& parseDiags = parser.diagnostics();
    if (!parseDiags.empty()) {
        std::cerr << "\n==== Parser Diagnostics ====\n";
        printDiagnostics(parseDiags);
    }

    if (!parseOk) {
        std::cerr << "\nCompilation finished with errors.\n";
        return 1;
    }

    std::cerr << "\nCompilation successful.\n";
    return 0;
}
