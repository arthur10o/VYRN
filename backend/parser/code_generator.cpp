#include "ast_parser.hpp"
#include <sstream>
#include <fstream>
#include <unordered_map>

struct VariableInfo {
    std::string type;
    std::string value;
    bool is_reference;
};

class CodeGenerator {
    std::ostringstream out;
    std::unordered_map<std::string, VariableInfo> symbol_table;         // Table of symbols: variable name → information

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
        if(auto let = std::dynamic_pointer_cast<LetNode>(_node)) {
            generate_let(let, _indent_level);
        } else {
            indent(_indent_level);
            out << "// Unknown node\n";
        }
    }

    void generate_let(const std::shared_ptr<LetNode>& _node, int _indent_level) {
        indent(_indent_level);
        if(symbol_table.find(_node->name) != symbol_table.end()) {
            out << "// Warning: variable '" << _node->name << "' already declared\n";
        } else {
            symbol_table[_node->name] = VariableInfo{_node->type, _node->value, _node->is_reference};
        }
        out << _node->type << " " << _node->name << " = " << _node->value << ";\n";
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

        while(std::getline(file, line)) {
            if(line.empty()) {
                line_number++;
                continue;
            }

            try {
                Parser Parser(line);
                auto node = Parser.parse_let();
                all_generated_code << cg.generate(node);
            } catch (const ParseError& err) {
                error_output << "Error at line " << line_number << ", column " << err.column << ": " << err.what() << "\n";
                error_output << "  " << line << "\n";
                error_output << "  " << std::string(err.column - 1, ' ') << "^\n";
            }
            line_number++;
        }

        std::cout << all_generated_code.str();

        std::cout << error_output.str();

        std::ofstream output("communication/output_result.txt");
        if(output) {
            output << error_output.str();
        } else {
            std::cerr << "Error: unable to write to output_result.txt.\n";
        }

        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}