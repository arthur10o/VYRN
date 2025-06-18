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
        if(decl) {
            if(decl->is_const) {
                generate_const(decl, _indent_level);
            } else {
                generate_let(decl, _indent_level);
            }
        } else {
            indent(_indent_level);
            out << "// Unknown node\n";
        }
    }

    void generate_let(const std::shared_ptr<DeclarationNode >& _node, int _indent_level) {
        indent(_indent_level);
        if(symbol_table_for_variable.find(_node->name) != symbol_table_for_variable.end()) {
            out << "// Warning: variable '" << _node->name << "' already declared\n";
        } else {
            symbol_table_for_variable[_node->name] = VariableInfo{_node->type, _node->value->value, _node->is_reference};
        }
        out << convert_type(_node->type) << " " << _node->name << " = " << format_literal(_node->value) << ";\n";
    }

    void generate_const(const std::shared_ptr<DeclarationNode >& _node, int _indent_level) {
        indent(_indent_level);
        if(symbol_table_for_constant.find(_node->name) != symbol_table_for_constant.end()) {
            out << "// Warning: constant '" << _node->name << "' already declared\n";
        } else {
            symbol_table_for_constant[_node->name] = ConstantInfo{_node->type, _node->value->value, _node->is_reference};
        }
        out << "const " << convert_type(_node->type) << " " << _node->name << " = " << format_literal(_node->value) << ";\n";
    }

    std::string format_literal(const std::shared_ptr<LiteralNode>& node) {
        if(node->type == "string") {
            return "\"" + node->value + "\"";
        } else if(node->type == "bool") {
            return (node->value == "true") ? "true" : "false";
        } else  if(node->type == "float") {
            std::string val = node->value;
            std::replace(val.begin(), val.end(), ',', '.');
            return val;
        } else {
            return node->value;
        }
    }

    std::string convert_type(const std::string& original_type) {
        if(original_type == "string") {
            return "std::string";
        }
        return original_type;
    }
};

int main() {
    std::ostringstream error_output;

    try {
        std::ifstream file("communication/input_code.txt");
        if(!file) {
            error_output << "Error: unable to open input_code.txt.\n";
            return 1;
        }

        std::string line;
        int line_number = 1;
        CodeGenerator cg;
        std::ostringstream all_generated_code;
        all_generated_code << "#include <iostream>\n";
        all_generated_code << "#include <string>\n";
        all_generated_code << "#include <iomanip>\n";
        all_generated_code << "int main() {\n";
        all_generated_code << "    std::cout << std::boolalpha;\n";
        all_generated_code << "    std::cout << std::setprecision(21);\n";

        while(std::getline(file, line)) {
            if(line.empty()) {
                line_number++;
                continue;
            }

            try {
                Parser Parser(line);
                std::shared_ptr<ASTNode> node;

                if(line.find("let") == 0) {
                    node = Parser.parse_let();
                } else if(line.find("const") == 0) {
                    node = Parser.parse_const();
                } else {
                    error_output << "Unknown declaration at line " + std::to_string(line_number);
                    line_number++;
                    continue;
                }

                all_generated_code << cg.generate(node);
            } catch (const ParseError& err) {
                error_output << "Error at line " << line_number << ", column " << err.column << ": " << err.what() << "\n";
                error_output << "  " << line << "\n";
                error_output << "  " << std::string(err.column - 1, ' ') << "^\n";
            }
            line_number++;
        }

        all_generated_code << "    std::cout << \"\\n✔ Le code a été exécuté avec succès.\\n\";\n";
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
        } else {
            std::cerr << "Error: unable to read program output.\n";
            return 1;
        }

        if(!error_output.str().empty()) {
            std::cerr << "Parsing errors:\n" << error_output.str();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}