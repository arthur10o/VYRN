#include "ast_parser.hpp"
#include <sstream>
#include <fstream>

class CodeGenerator {
    std::ostringstream out;

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
        out << _node->type << " " << _node->name << " = " << _node->value << ";\n";
    }
};

int main() {
    try {
        std::ifstream file("communication/input_code.txt");
        if(!file) {
            std::cerr << "Error: unable to open input_code.txt.\n";
            return 1;
        }

        std::string line;
        CodeGenerator cg;
        std::ostringstream all_generated_code;

        while(std::getline(file, line)) {
            if(line.empty()) continue;
            Parser Parser(line);
            auto node = Parser.parse_let();
            node->print();

            std::string generated_code = cg.generate(node);
            all_generated_code << generated_code;
        }

        std::cout << all_generated_code.str();

        std::ofstream output("communication/output_result.txt");
        if(output) {
            output << all_generated_code.str();
        } else {
            std::cerr << "Error: unable to write to output_result.txt.\n";
        }

        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}