#ifndef AST_PARSER
#define AST_PARSER

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <unordered_set>
#include <stdexcept>

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
    int line;
    int column;
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class VarNode : public ASTNode {
public:
    std::string type;
    std::string name;
    std::string value;
    bool is_reference;
    
    VarNode(const std::string& _type, const std::string& _name, const std::string& _value, bool _is_reference) :
        type(_type), name(_name), value(_value), is_reference(_is_reference) {}
};

class ConstNode : public ASTNode {
public:
    std::string type;
    std::string name;
    std::string value;
    bool is_reference;
    
    ConstNode(const std::string& _type, const std::string& _name, const std::string& _value, bool _is_reference) :
        type(_type), name(_name), value(_value), is_reference(_is_reference) {}
};

class AssignNode : public ASTNode {
public:
    std::string type;
    std::string name;
    std::string value;
    bool is_reference;

    AssignNode(const std::string& _type, const std::string& _name, const std::string& _value, bool _is_reference) :
        type(_type), name(_name), value(_value), is_reference(_is_reference) {}
};

class MultiOpNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> operands;
    std::vector<std::string> operators;

    MultiOpNode(const std::vector<std::shared_ptr<ASTNode>>& _operands, const std::vector<std::string>& _operators) : operands(_operands), operators(_operators) {}
};

class Lexer {
    const std::string& input;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    void skip_white_space() {
        while (pos < input.size() && std::isspace(input[pos])) pos++;
    }

    char peek() const {
        return pos < input.size() ? input[pos] : '\0';
    }

    char advance() {
        if(pos >= input.size()) return '\0';
        char character_to_analyse = input[pos++];
        if(character_to_analyse == '\n') {
            line++;
            column = 1;
        } else {
            column ++;
        }
        return character_to_analyse;
    }

    bool is_identifier_character(char character_to_identified) {
        return std::isalnum(character_to_identified) || character_to_identified == '_';
    }

public:
    Lexer(const std::string& _input) : input(_input) {}

    Token next_token() {
        skip_white_space();
        if(pos >= input.size()) return {TokenType::EndOfFile, "", line, column};

        char character_to_analyse = peek();
        int tok_line  = line;
        int tok_column = column;

        if(std::isalpha(character_to_analyse) || character_to_analyse == '_') {
            size_t start = pos;
            while(pos < input.size() && is_identifier_character(input[pos])) advance();
            std::string word = input.substr(start, pos - start);

            if(keywords.find(word) != keywords.end()) {
                return {TokenType::Keyword, word, tok_line, tok_column};
            } else if(types.find(word) != types.end()) {
                return {TokenType::Type, word, tok_line, tok_column};
            } else {
                return {TokenType::Identifier, word, tok_line, tok_column};
            }
            
        } else if(std::isdigit(character_to_analyse)) {
            size_t start = pos;
            while(pos < input.size() && std::isdigit(input[pos])) advance();
            return {TokenType::Number, input.substr(start, pos - start), tok_line, tok_column};
        }

        advance();
        return {TokenType::Symbol, std::string(1, character_to_analyse), tok_line, tok_column};
    }
};

class ParseError : public  std::runtime_error {
public:
    int line;
    int column;

    ParseError(const std::string& _message, int _line, int _column) : std::runtime_error(_message), line(_line), column(_column) {}
};

class Parser {
    Lexer lexer;
    Token current_token;

    void next_token() {
        current_token = lexer.next_token();
    }

    void expect(TokenType type, const std::string& value = "") {
        if(current_token.type != type || (!value.empty() && current_token.value != value)) {
            throw ParseError("Unexpected token: '" + current_token.value + "'", current_token.line, current_token.column);
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
            throw ParseError("Expected a type after 'let'", current_token.line, current_token.column);
        }
        std::string type = current_token.value;
        next_token();

        if(current_token.type != TokenType::Identifier) {
            throw ParseError("Expected an identifier after type", current_token.line, current_token.column);
        }
        std::string name = current_token.value;
        next_token();

        expect(TokenType::Symbol, "=");

        if(current_token.type != TokenType::Number) {
            throw ParseError("Expected a number after '='", current_token.line, current_token.column);
        }
        std::string value = current_token.value;
        next_token();

        return std::make_shared<VarNode>(type, name, value, false);
    }

    std::shared_ptr<ASTNode> parse_const() {
        expect(TokenType::Keyword, "const");
        if(current_token.type != TokenType::Type) {
            throw ParseError("Expected a type after 'const'", current_token.line, current_token.column);
        }
        std::string type = current_token.value;
        next_token();

        if(current_token.type != TokenType::Identifier) {
            throw ParseError("Expected an identifier after type", current_token.line, current_token.column);
        }
        std::string name = current_token.value;
        next_token();

        expect(TokenType::Symbol, "=");

        if(current_token.type != TokenType::Number) {
            throw ParseError("Expected a number after '='", current_token.line, current_token.column);
        }
        std::string value = current_token.value;
        next_token();
        
        return std::make_shared<ConstNode>(type, name, value, false);
    }
};
#endif