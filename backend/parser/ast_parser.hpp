#ifndef AST_PARSER
#define AST_PARSER

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <unordered_set>

enum class TokenType {
    Identifier,
    Keyword,
    Type,
    Number,
    Symbol,
    EndOfFile,
    Unknown
};

static const std::unordered_set<std::string> keywords = {
    "let",
    "const"
};

static const std::unordered_set<std::string> types = {
    "int",
    "float",
    "bool",
    "string"
};

struct Token {
    TokenType type;
    std::string value;
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

class LetNode : public ASTNode {
public:
    std::string type;
    std::string name;
    std::string value;
    bool is_reference;
    
    LetNode(const std::string& _type, const std::string& _name, const std::string& _value, bool _is_reference) :
        type(_type), name(_name), value(_value), is_reference(_is_reference) {}

    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << "LetNode: " << type << " " << name << " = " << value << " is_reference: " << is_reference 
                  << "\n";
    }
};

class ConstNode : public ASTNode {
public:
    std::string type;
    std::string name;
    std::string value;
    bool is_reference;
    
    ConstNode(const std::string& _type, const std::string& _name, const std::string& _value, bool _is_reference) :
        type(_type), name(_name), value(_value), is_reference(_is_reference) {}

    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << "ConstNode: " << type << " " << name << " = " << value << " is_reference: " << is_reference 
                  << "\n";
    }
};

class AssignNode : public ASTNode {
public:
    std::string type;
    std::string name;
    std::string value;
    bool is_reference;

    AssignNode(const std::string& _type, const std::string& _name, const std::string& _value, bool _is_reference) :
        type(_type), name(_name), value(_value), is_reference(_is_reference) {}

    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << "AssignNode: " << type << " " << name << " = " << value << " is_reference: " << is_reference 
                  << "\n";
    }
};

class MultiOpNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> operands;
    std::vector<std::string> operators;

    MultiOpNode(const std::vector<std::shared_ptr<ASTNode>>& _operands, const std::vector<std::string>& _operators) : operands(_operands), operators(_operators) {}
    
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << "MultiOpNode:\n";

        for (size_t i = 0; i < operands.size(); i++) {
            operands[i]->print(indent + 1);
            if (i < operators.size()) {
                for (int j = 0; j < indent + 1; j++) std::cout << "  ";
                std::cout << "Operator: '" << operators[i] << "'\n";
            }
        }
    }
};

class Lexer {
    const std::string& input;
    size_t pos = 0;

    void skip_white_space() {
        while (pos < input.size() && std::isspace(input[pos])) pos++;
    }

    bool is_identifier_character(char character_to_identified) {
        return std::isalnum(character_to_identified) || character_to_identified == '_';
    }
public:
    Lexer(const std::string& _input) : input(_input) {}

    Token next_token() {
        skip_white_space();
        if(pos >= input.size()) return {TokenType::EndOfFile, ""};

        if(std::isalpha(input[pos]) || input[pos] == '_') {
            size_t start = pos;
            while(pos < input.size() && is_identifier_character(input[pos])) pos++;
            std::string word = input.substr(start, pos - start);

            if(keywords.find(word) != keywords.end()) {
                return {TokenType::Keyword, word};
            } else if(types.find(word) != types.end()) {
                return {TokenType::Type, word};
            } else {
                return {TokenType::Identifier, word};
            }
            
        } else if(std::isdigit(input[pos])) {
            size_t start = pos;
            while(pos < input.size() && std::isdigit(input[pos])) pos++;
            return {TokenType::Number, input.substr(start, pos - start)};
        }

        char character_to_analyse = input[pos++];
        return {TokenType::Symbol, std::string(1, character_to_analyse)};
    }
};

class Parser {
    Lexer lexer;
    Token current_token;

    void next_token() {
        current_token = lexer.next_token();
    }

    void expect(TokenType type, const std::string& value = "") {
        if(current_token.type != type || (!value.empty() && current_token.value != value)) {
            throw std::runtime_error("Unexpected token: " + current_token.value);
        }
        next_token();
    }
public:
    Parser(const std::string& _input) : lexer(_input) {
        next_token();
    }

    std::shared_ptr<ASTNode> parse_let() {
        expect(TokenType::Keyword, "let");
        if(current_token.type != TokenType::Type) {
            throw std::runtime_error("Expected type after let");
        }
        std::string type = current_token.value;
        next_token();

        if(current_token.type != TokenType::Identifier) {
            throw std::runtime_error("Expected identifier after type");
        }
        std::string name = current_token.value;
        next_token();

        expect(TokenType::Symbol, "=");

        if(current_token.type != TokenType::Number) {
            throw std::runtime_error("Expected number after '='");
        }
        std::string value = current_token.value;
        next_token();

        return std::make_shared<LetNode>(type, name, value, false);
    }
};

/*int main() {
    std::string code = "let int x = 42;";
    Parser Parser(code);
    auto node = Parser.parse_let();
    node->print();
    return 0;
}*/

#endif