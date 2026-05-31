#include "compiler.h"
#include <iostream>
#include <set>

// ============================================================
// Validator implementation — simulates execution to catch
// semantic errors before LLVM IR code generation
// ============================================================

Validator::Validator(const std::vector<Token>& tokens)
    : tokens(tokens), idx(0), simulatedStackSize(0), ifDepth(0), loopDepth(0) {}

Token Validator::current() {
    if (idx < tokens.size()) return tokens[idx];
    return Token(TokenType::EOF_TOKEN, "", 0, 0);
}

void Validator::advance() {
    if (idx < tokens.size()) idx++;
}

void Validator::checkStackUnderflow(int required, const std::string& op, const Token& tok) {
    if (simulatedStackSize < required) {
        diags.push_back({
            Diagnostic::ERROR, tok.line, tok.column,
            "Stack underflow: '" + op + "' requires " +
            std::to_string(required) + " value(s) on the stack, but only " +
            std::to_string(simulatedStackSize) + " available."
        });
    }
}

const std::vector<Diagnostic>& Validator::diagnostics() const {
    return diags;
}

// ============================================================
// Main validation entry point
// ============================================================
bool Validator::validate() {
    std::set<std::string> definedVars;   // tracks assigned variables
    std::string lastIdentifier;

    while (idx < tokens.size()) {
        Token tok = current();

        switch (tok.type) {
            case TokenType::NUMBER: {
                simulatedStackSize++;
                advance();
                break;
            }

            case TokenType::IDENTIFIER: {
                lastIdentifier = tok.value;
                advance();

                // Check if the NEXT token is '=' (immediate assign: x =)
                if (idx < tokens.size() && tokens[idx].type == TokenType::ASSIGN) {
                    // Pattern: x <value-already-on-stack> =
                    // but here it's immediate: x = (pop TOS, assign to x)
                    checkStackUnderflow(1, "= (assign to '" + lastIdentifier + "')", tokens[idx]);
                    if (simulatedStackSize >= 1) {
                        simulatedStackSize--;  // pop the value
                    }
                    definedVars.insert(lastIdentifier);
                    advance();  // skip '='
                } else {
                    // Variable read — push its value onto the stack
                    if (definedVars.find(lastIdentifier) == definedVars.end()) {
                        diags.push_back({
                            Diagnostic::WARNING, tok.line, tok.column,
                            "Variable '" + lastIdentifier +
                            "' used before assignment (defaults to 0)."
                        });
                    }
                    simulatedStackSize++;  // push variable value
                }
                break;
            }

            // Binary arithmetic operators — pop 2, push 1
            case TokenType::PLUS: {
                checkStackUnderflow(2, "+", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::MINUS: {
                checkStackUnderflow(2, "-", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::MUL: {
                checkStackUnderflow(2, "*", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::DIV: {
                checkStackUnderflow(2, "/", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::MOD: {
                checkStackUnderflow(2, "%", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }

            // Comparison operators — pop 2, push 1
            case TokenType::GREATER: {
                checkStackUnderflow(2, ">", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::LESS: {
                checkStackUnderflow(2, "<", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::EQUAL: {
                checkStackUnderflow(2, "==", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::NOT_EQUAL: {
                checkStackUnderflow(2, "!=", tok);
                if (simulatedStackSize >= 2) simulatedStackSize--;
                advance();
                break;
            }

            // Unary operators
            case TokenType::NEG: {
                checkStackUnderflow(1, "neg", tok);
                // stack size unchanged (pop 1, push 1)
                advance();
                break;
            }

            // Stack manipulation
            case TokenType::DUP: {
                checkStackUnderflow(1, "dup", tok);
                if (simulatedStackSize >= 1) simulatedStackSize++;
                advance();
                break;
            }
            case TokenType::SWAP: {
                checkStackUnderflow(2, "swap", tok);
                // stack size unchanged
                advance();
                break;
            }

            // I/O
            case TokenType::PRINT: {
                checkStackUnderflow(1, "print", tok);
                if (simulatedStackSize >= 1) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::INPUT: {
                simulatedStackSize++;  // push scanned value
                advance();
                break;
            }

            // Control flow — if/else/end
            case TokenType::IF: {
                checkStackUnderflow(1, "if (condition)", tok);
                if (simulatedStackSize >= 1) simulatedStackSize--;  // pop condition
                ifDepth++;
                advance();
                break;
            }
            case TokenType::ELSE: {
                if (ifDepth <= 0) {
                    diags.push_back({
                        Diagnostic::ERROR, tok.line, tok.column,
                        "'else' without matching 'if'."
                    });
                }
                advance();
                break;
            }
            case TokenType::END: {
                if (ifDepth > 0) {
                    ifDepth--;
                } else if (loopDepth > 0) {
                    loopDepth--;
                } else {
                    diags.push_back({
                        Diagnostic::ERROR, tok.line, tok.column,
                        "'end' without matching 'if', 'for', or 'repeat'."
                    });
                }
                advance();
                break;
            }

            // Loop constructs
            case TokenType::FOR: {
                checkStackUnderflow(2, "for (requires limit and start)", tok);
                if (simulatedStackSize >= 2) simulatedStackSize -= 2;  // pop limit and start
                loopDepth++;
                advance();
                break;
            }
            case TokenType::REPEAT: {
                checkStackUnderflow(1, "repeat (requires count)", tok);
                if (simulatedStackSize >= 1) simulatedStackSize--;  // pop count
                loopDepth++;
                advance();
                break;
            }

            case TokenType::FLOAT: {
                simulatedStackSize++;
                advance();
                break;
            }
            case TokenType::DROP: {
                checkStackUnderflow(1, "drop", tok);
                if (simulatedStackSize >= 1) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::PRINTLN: {
                checkStackUnderflow(1, "println", tok);
                if (simulatedStackSize >= 1) simulatedStackSize--;
                advance();
                break;
            }
            case TokenType::F2I: {
                checkStackUnderflow(1, "f2i", tok);
                // stack size unchanged (pop 1, push 1)
                advance();
                break;
            }
            case TokenType::I2F: {
                checkStackUnderflow(1, "i2f", tok);
                // stack size unchanged (pop 1, push 1)
                advance();
                break;
            }

            case TokenType::ASSIGN: {
                // '=' that follows a value on the stack — assign to lastIdentifier
                if (!lastIdentifier.empty()) {
                    checkStackUnderflow(1, "= (assign to '" + lastIdentifier + "')", tok);
                    if (simulatedStackSize >= 1) {
                        simulatedStackSize--;  // pop the assigned value
                    }
                    definedVars.insert(lastIdentifier);
                } else {
                    diags.push_back({
                        Diagnostic::ERROR, tok.line, tok.column,
                        "Unexpected '=' — no preceding identifier for assignment."
                    });
                }
                advance();
                break;
            }

            case TokenType::EOF_TOKEN: {
                // Done
                goto done;
            }

            default: {
                diags.push_back({
                    Diagnostic::WARNING, tok.line, tok.column,
                    "Unknown token '" + tok.value + "', skipping."
                });
                advance();
                break;
            }
        }
    }

done:
    // Check for unclosed if blocks
    if (ifDepth > 0) {
        diags.push_back({
            Diagnostic::ERROR, 0, 0,
            std::to_string(ifDepth) + " unclosed 'if' block(s) — missing 'end'."
        });
    }
    // Check for unclosed loop blocks
    if (loopDepth > 0) {
        diags.push_back({
            Diagnostic::ERROR, 0, 0,
            std::to_string(loopDepth) + " unclosed loop block(s) (for/repeat) — missing 'end'."
        });
    }

    // Count errors
    bool hasErrors = false;
    for (const auto& d : diags) {
        if (d.level == Diagnostic::ERROR) {
            hasErrors = true;
            break;
        }
    }

    return !hasErrors;
}
