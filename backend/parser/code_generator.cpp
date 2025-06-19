#include "ast_parser.hpp"
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <algorithm>

struct VariableInfo {
    std::string type;
    std::string value;
    bool is_reference;
};

struct ConstantInfo {
    std::string type;
    std::string value;
    bool is_reference;
};

class CodeGenerator {
    std::ostringstream out;
    std::unordered_map<std::string, VariableInfo> symbol_table_for_variable;         // Table of symbols for variable: variable name → information
    std::unordered_map<std::string, ConstantInfo> symbol_table_for_constant;         // Table of symbols for constant: constant name → information

    void indent(int level) {
        for(int i = 0; i < level; i++) {
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
        if(decl) {
            if(decl->is_const) {
                generate_const(decl, _indent_level);
            } else {
                generate_let(decl, _indent_level);
            }
        } else if(log_node) {
            generate_log(log_node, _indent_level);
            return;
        } else if(assign_node) {
            generate_assign(assign_node, _indent_level);
        } else {
            indent(_indent_level);
            out << "// Unknown node\n";
        }
    }

    void generate_assign(const std::shared_ptr<AssignNode>& _node, int _indent_level) {
        indent(_indent_level);
        if(symbol_table_for_variable.find(_node->target_variable) == symbol_table_for_variable.end()) {
            out << "// Error: variable '" << _node->target_variable << "' is not declared\n";
        } else {
            out << _node->target_variable << " = " << _node->source_variable << ";\n";
            if (_node->is_reference) {
                out << _node->source_variable;
            } else {
                out << _node->source_variable;
            }
        }
    }

    void generate_let(const std::shared_ptr<DeclarationNode >& _node, int _indent_level) {
        indent(_indent_level);
        if(symbol_table_for_variable.find(_node->name) != symbol_table_for_variable.end()) {
            out << "// Warning: variable '" << _node->name << "' already declared\n";
        } else {
            symbol_table_for_variable[_node->name] = VariableInfo{_node->type, _node->value->value, _node->is_reference};
        }
        out << convert_type(_node->type) << " " << _node->name << " = ";
        if(_node->is_reference) {
            out << _node->value->value;
        } else {
            out << format_literal(_node->value) << ";\n";
        }
    }

    void generate_const(const std::shared_ptr<DeclarationNode >& _node, int _indent_level) {
        indent(_indent_level);
        if(symbol_table_for_constant.find(_node->name) != symbol_table_for_constant.end()) {
            out << "// Warning: constant '" << _node->name << "' already declared\n";
        } else {
            symbol_table_for_constant[_node->name] = ConstantInfo{_node->type, _node->value->value, _node->is_reference};
        }
        out << "const " << convert_type(_node->type) << " " << _node->name << " = ";
        if(_node->is_reference) {
            out << _node->value->value;
        } else {
            out << format_literal(_node->value) << ";\n";
        }
    }

    void generate_log(const std::shared_ptr<LogNode>& _node, int _indent_level) {
        indent(_indent_level);
        out << "std::cout << ";
        if(_node->is_variable) {
            if(symbol_table_for_variable.count(_node->variable_name)) {
                out << _node->variable_name;
            } else if(symbol_table_for_constant.count(_node->variable_name)) {
                out << _node->variable_name;
            } else {
                out << "\"[Undefined variable: " << _node->variable_name << "]\"";
            }
        } else {
            out << format_literal(_node->value);
        }
        out << " << std::endl;\n";
    }

    std::string format_literal(const std::shared_ptr<LiteralNode>& node) {
        if(node->type == "string" && !node->is_reference) {
            return "\"" + node->value + "\"";
        } else if(node->type == "bool") {
            return (node->value == "true") ? "true" : "false";
        } else  if(node->type == "float") {
            std::string val = node->value;
            std::replace(val.begin(), val.end(), ',', '.');
            return val;
        } else if(node->type == "int") {
            return node->value;
        } else {
            return node->value;
        }
    }

    std::string convert_type(const std::string& original_type) {
        if(original_type == "string") {
            return "std::string";
        } else if(original_type == "int") {
            return "int";
        } else if(original_type == "float") {
            return "float";
        } else if(original_type == "bool") {
            return "bool";
        }
        return original_type;
    }
};

std::vector<std::string> split_instructions(const std::string& code) {
    std::vector<std::string> instructions;
    std::stringstream instr_stream(code);
    std::string instruction;

    while (std::getline(instr_stream, instruction, ';')) {
        instruction.erase(0, instruction.find_first_not_of(" \t\r\n"));
        instruction.erase(instruction.find_last_not_of(" \t\r\n") + 1);
        if (!instruction.empty())
            instructions.push_back(instruction);
    }

    return instructions;
}

int main() {
    std::ostringstream error_output;

    try {
        std::ifstream file("communication/input_code.txt");
        if(!file) {
            std::cerr  << "Error: unable to open input_code.txt.\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();

        std::string instructions = code;
        int line_number = 1;

        CodeGenerator cg;
        std::ostringstream all_generated_code;
        all_generated_code << "#include <iostream>\n";
        all_generated_code << "#include <string>\n";
        all_generated_code << "#include <iomanip>\n";
        all_generated_code << "int main() {\n";
        all_generated_code << "    std::cout << std::boolalpha;\n";
        all_generated_code << "    std::cout << std::setprecision(21);\n";

        std::vector<std::string> parts = split_instructions(instructions);

        for(const auto& instruction : parts) {
            try {
                Parser parser(instruction);
                std::shared_ptr<ASTNode> node;

                if (instruction.find("let") == 0) {
                    node = parser.parse_let();
                } else if (instruction.find("const") == 0) {
                    node = parser.parse_const();
                } else if (instruction.find("log(") == 0 || instruction.find("log") == 0) {
                    node = parser.parse_log();
                } else {
                    error_output << "Unknown declaration at line " + std::to_string(line_number);
                    line_number++;
                    continue;
                }

                all_generated_code << cg.generate(node);
            } catch (const ParseError& err) {
                error_output << "Error at line " << line_number << ", column " << err.column << ": " << err.what() << "\n";
                error_output << "  " << instructions << "\n";
                error_output << "  " << std::string(err.column - 1, ' ') << "^\n";
            }
        }
        all_generated_code << "\n";
        line_number++;

        all_generated_code << "    return 0;";
        all_generated_code << "}";

        file.close();

        const std::string generated_filename = "communication/generated_code.cpp";
        std::ofstream output(generated_filename);
        if(!output) {
            std::cerr << "Error: unable to write to " << generated_filename << "\n";
            return 1;
        }

        output << all_generated_code.str();
        output.close();

        const std::string executable_name = "communication\\generated_program.exe";
        std::string compile_command = "g++ -std=c++17 " + generated_filename + " -o " + executable_name + " 2> communication/compile_errors.txt";
        int compile_result = system(compile_command.c_str());

        if(compile_result != 0) {
            std::ifstream compile_errors("communication/compile_errors.txt");
            if(compile_errors) {
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
        int run_result = system(run_command.c_str());
        std::cerr << "Run command: " << run_command << "\n";
        std::cerr << "Return code: " << run_result << "\n";
        if(run_result != 0) {
            std::cerr << "Error: execution of generated program failed.\n";
            return 1;
        }

        std::ifstream program_output(output_capture_file);
        if(program_output) {
            std::cout << "===== Output of generated program =====\n";
            std::cout << program_output.rdbuf();
            std::cout << "======================================\n";
            program_output.close();
            std::ofstream output_bis ("communication/program_output.txt", std::ios::app);
        if(output_bis) {
            output_bis << "\n✔ Le code a été exécuté avec succès.\n";
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