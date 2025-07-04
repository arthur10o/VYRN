#include "ast_parser.hpp"
#include <fstream>
#include <unordered_map>
#include <algorithm>

enum class SymbolKind {
    VARIABLE,
    CONSTANT
};

struct SymbolInfo {
    std::string types;
    std::string value;
    bool is_reference;
    SymbolKind kind;
};

class CodeGenerator {
    std::ostringstream out;
    std::unordered_map<std::string, SymbolInfo> symbol_table;

    void indent(int _level) {
        for(int i = 0; i < _level; i++) {
            out << "    ";
        }
    }

public:
    std::string generate(const std::shared_ptr<ASTNode>& _node, int _indent_level = 0) {
        out.str("");
        out.clear();
        generate_node(_node, _indent_level);
        return out.str();
    }

private:
    void generate_node(const std::shared_ptr<ASTNode>& _node, int _indent_level) {
        auto decl = std::dynamic_pointer_cast<DeclarationNode>(_node);
        auto log_node = std::dynamic_pointer_cast<LogNode>(_node);
        auto assign_node = std::dynamic_pointer_cast<AssignNode>(_node);
        auto multi_op_node = std::dynamic_pointer_cast<MultiOpNode>(_node);
        auto multi_op_bool_node = std::dynamic_pointer_cast<MultiOpBoolNode>(_node);

        if (decl) {
            generate_declaration(decl, _indent_level, decl->is_const ? SymbolKind::CONSTANT : SymbolKind::VARIABLE);
        } else if (log_node) {
            generate_log(log_node, _indent_level);
            return;
        } else if (assign_node) {
            generate_assign(assign_node, _indent_level);
        } else if (multi_op_node) {
            indent(_indent_level);
            out << "// Multi-op expression not evaluated at compile time (should be evaluated in parser)\n";
        } else if (multi_op_bool_node) {
            indent(_indent_level);
            out << "// Multi-op bool expression not evaluated at compile time (should be evaluated in parser)\n";
        } else {
            indent(_indent_level);
            out << "// Unknown node\n";
        }
    }

    bool is_declared(const std::string& _name, bool _is_const = false) {
        auto it = symbol_table.find(_name);
        if (it != symbol_table.end()) {
            return (_is_const && it->second.kind == SymbolKind::CONSTANT) || (!_is_const && it->second.kind == SymbolKind::VARIABLE);
        }
        return false;
    }

    bool is_const(const std::string& _name) {
        auto it = symbol_table.find(_name);
        if(it != symbol_table.end()) {
            return it->second.kind == SymbolKind::CONSTANT;
        }
        return false;
    }

    std::string generate_multiop_bool_expr(const std::shared_ptr<MultiOpBoolNode>& _node) {
        if (!_node || _node->operands.empty()) return "";
        std::ostringstream expr;
        for (size_t i = 0; i < _node->operands.size(); ++i) {
            if (auto sub = std::dynamic_pointer_cast<MultiOpBoolNode>(_node->operands[i])) {
                expr << "(" << generate_multiop_bool_expr(sub) << ")";
            } else if (auto lit = std::dynamic_pointer_cast<LiteralNode>(_node->operands[i])) {
                expr << format_literal(lit);
            } else if (auto var = std::dynamic_pointer_cast<ASTNode>(_node->operands[i])) {
                if (auto l = std::dynamic_pointer_cast<LiteralNode>(var)) {
                    expr << l->value;
                } else {
                    expr << "/*unsupported op*/";
                }
            }
            if (i < _node->operators.size()) {
                expr << " " << _node->operators[i] << " ";
            }
        }
        return expr.str();
    }

    void generate_assign(const std::shared_ptr<AssignNode>& _node, int _indent_level) {
        indent(_indent_level);
        bool declared = is_declared(_node->target_variable);
        if(!declared) {
            out << "// Error: variable '" << _node->target_variable << "' is not declared\n";
        } else if(is_const(_node->target_variable)) {
            out << "// Error: cannot assign to constant '" << _node->target_variable << "'\n";
        } else {
            out << _node->target_variable << " = ";
            if (_node->expr) {
                if (auto multi_bool = std::dynamic_pointer_cast<MultiOpBoolNode>(_node->expr)) {
                    out << generate_multiop_bool_expr(multi_bool);
                } else if (auto bool_lit = std::dynamic_pointer_cast<BoolNode>(_node->expr)) {
                    out << format_literal(bool_lit);
                } else {
                    out << "/* unsupported expr */";
                }
            } else if (_node->is_reference) {
                out << _node->source_variable;
            } else {
                if (symbol_table[_node->target_variable].types == "string" && _node->source_variable.find('"') == std::string::npos) {
                    out << "\"" << _node->source_variable << "\"";
                } else {
                    out << format_literal(std::make_shared<LiteralNode>("", _node->source_variable));
                }
            }
            out << ";\n";
        }
    }

    void generate_declaration(const std::shared_ptr<DeclarationNode>& _node, int _indent_level, SymbolKind _kind) {
        indent(_indent_level);
        bool declared = is_declared(_node->name, _node->is_const);
        if(declared) {
            out << "// Warning: " << (_kind == SymbolKind::CONSTANT ? "constant" : "variable") << " '" << _node->name << "' already declared\n";
        } else {
            symbol_table[_node->name] = SymbolInfo {_node->type, _node->value->value, _node->is_reference, _kind};
        }
        out << (_kind == SymbolKind::CONSTANT ? "const " : "") << convert_type(_node->type) << " " << _node->name << " = ";
        if (auto multi_bool = std::dynamic_pointer_cast<MultiOpBoolNode>(_node->value)) {
            out << generate_multiop_bool_expr(multi_bool);
        } else if (_node->type == "string" && !_node->is_reference) {
            out << "\"" << _node->value->value << "\"";
        } else {
            out << (_node->is_reference ? _node->value->value : format_literal(_node->value));
        }
        out << ";\n";
    }

    void generate_log(const std::shared_ptr<LogNode>& _node, int _indent_level) {
        indent(_indent_level);
        out << "std::cout << ";
        if(_node->is_variable) {
            if(symbol_table.count(_node->variable_name)) {
                out << _node->variable_name;
            } else {
                out << "\"[Undefined variable: " << _node->variable_name << "]\"";
            }
        } else {
            out << format_literal(_node->value);
        }
        out << " << std::endl;\n";
    }

    std::string format_literal(const std::shared_ptr<LiteralNode>& _node) {
        if(_node->type == "string" && !_node->is_reference) {
            return "\"" + _node->value + "\"";
        } else if(_node->type == "bool") {
            return (_node->value == "true") ? "true" : "false";
        } else  if(_node->type == "float") {
            std::string val = _node->value;
            std::replace(val.begin(), val.end(), ',', '.');
            return val;
        } else if(_node->type == "int") {
            return _node->value;
        } else {
            return _node->value;
        }
    }

    std::string convert_type(const std::string& _original_type) {
        if(_original_type == "string") {
            return "std::string";
        }
        return _original_type;
    }
};

std::string trim(const std::string& _s) {
    auto start = _s.find_first_not_of(" \t\r\n");
    auto end = _s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : _s.substr(start, end - start + 1);
}

std::vector<std::string> split_instructions(const std::string& _code) {
    std::vector<std::string> instructions;
    std::string instruction;
    bool in_multi_line_comment = false;

    for (size_t i = 0; i < _code.size(); ) {
        if (!in_multi_line_comment && i + 1 < _code.size() && _code[i] == '/' && _code[i + 1] == '*') {
            in_multi_line_comment = true;
            i += 2;
        } else if (in_multi_line_comment && i + 1 < _code.size() && _code[i] == '*' && _code[i + 1] == '/') {
            in_multi_line_comment = false;
            i += 2;
        } else if (!in_multi_line_comment && i + 1 < _code.size() && _code[i] == '/' && _code[i + 1] == '/') {
            while (i < _code.size() && _code[i] != '\n') {
                i++;
            }
        } else if (!in_multi_line_comment) {
            if (_code[i] == ';') {
                instruction = trim(instruction);
                if (!instruction.empty()) {
                    instructions.push_back(instruction);
                }
                instruction.clear();
            } else {
                instruction += _code[i];
            }
            i++;
        } else {
            i++;
        }
    }

    if (!instruction.empty()) {
        instruction = trim(instruction);
        if (!instruction.empty()) {
            instructions.push_back(instruction);
        }
    }

    return instructions;
}

int main() {
    std::ostringstream error_output;

    try {
        std::ifstream file("communication/input_code.txt");

        if (!file) {
            std::cerr  << "Error: unable to open input_code.txt.\n";
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();

        CodeGenerator cg;
        std::ostringstream all_generated_code;

        all_generated_code << "#include <iostream>\n#include <string>\n#include <iomanip>\n#include <cmath>\nint main() {\nstd::cout << std::boolalpha;\nstd::cout << std::setprecision(21);\n";

        std::vector<std::string> parts = split_instructions(code);

        for (const auto& instruction : parts) {
            try {
                Parser parser(instruction);
                std::shared_ptr<ASTNode> node;

                if (instruction.find("let") == 0) {
                    node = parser.parse_let();
                } else if (instruction.find("const") == 0) {
                    node = parser.parse_const();
                } else if (instruction.find("log(") == 0 || instruction.find("log") == 0) {
                    node = parser.parse_log();
                } else if (instruction.find("=") != std::string::npos) {
                    node = parser.parse_assign();
                } else {
                    error_output << "Unknown declaration";
                    continue;
                }
                all_generated_code << cg.generate(node);
            } catch (const ParseError& err) {
                error_output << "Error: " << err.what() << "\n";
            }
        }

        all_generated_code << "\n    return 0;\n}";

        file.close();

        const std::string generated_filename = "communication/generated_code.cpp";
        std::ofstream output(generated_filename);

        if (!output) {
            std::cerr << "Error: unable to write to " << generated_filename << "\n";
            return 1;
        }

        output << all_generated_code.str();
        output.close();

        const std::string executable_name = "communication\\generated_program.exe";
        std::string compile_command = "g++ -std=c++17 -O0 -pipe -march=native " + generated_filename + " -o " + executable_name + " 2> communication/compile_errors.txt";
        int compile_result = std::system(compile_command.c_str());

        if (compile_result != 0) {
            std::ifstream compile_errors("communication/compile_errors.txt");
            if (compile_errors) {
                std::cerr << "Compilation errors:\n";
                std::cerr << compile_errors.rdbuf();
                compile_errors.close();
            } else {
                std::cerr << "Unknown compilation error.\n";
            }
            return 1;
        }

        const std::string output_capture_file = "communication/program_output.txt";
        std::string run_command = ".\\" + executable_name + " > " + output_capture_file + " 2>&1";
        int run_result = std::system(run_command.c_str());
        if (run_result != 0) {
            std::cerr << "Error: execution of generated program failed.\n";
            return 1;
        }

        std::ifstream program_output(output_capture_file);

        if (program_output) {
            std::cout << "===== Output of generated program =====\n";
            std::cout << program_output.rdbuf();
            std::cout << "======================================\n";
            program_output.close();
            std::ofstream output_bis ("communication/program_output.txt", std::ios::app);
            if (output_bis) {
                output_bis << "\n✔ The code has been successfully executed...\n";
                output_bis.close();
            }
        } else {
            std::cerr << "Error: unable to read program output.\n";
            return 1;
        }

        if (!error_output.str().empty()) {
            std::ofstream parsing_error("communication/parsing_errors.txt");
            if (parsing_error) {
                parsing_error << error_output.str();
                parsing_error.close();
            }
        } else {
            std::ofstream parsing_error("communication/parsing_errors.txt", std::ios::trunc);
            parsing_error.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}