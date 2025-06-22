#ifndef AST_PARSER
#define AST_PARSER

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <unordered_set>
#include <stdexcept>
#include <cmath>
#include <functional>
#include <sstream>

enum class TokenType {
    Identifier,
    Keyword,
    Type,
    Number,
    STRING,
    BOOL,
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

class LiteralNode : public ASTNode {
public:
    std::string type;
    std::string value;
    bool is_reference = false;

    LiteralNode(const std::string& _type, const std::string& _value, bool _is_reference = false) 
        : type(_type), value(_value), is_reference(_is_reference) {}
};

class IntNode : public LiteralNode {
public:
    IntNode(const std::string& _value)
        : LiteralNode("int", _value) {}
};

class FloatNode : public LiteralNode {
public:
    FloatNode(const std::string& _value)
        : LiteralNode("float", _value) {}
};

class StringNode : public LiteralNode {
public:
    StringNode(const std::string& _value)
        : LiteralNode("string", _value) {}
};

class BoolNode : public LiteralNode {
public:
    BoolNode(const std::string& _value)
        : LiteralNode("bool", _value) {}
};

class DeclarationNode : public ASTNode {
public:
    bool is_const;
    bool is_reference;
    std::string type;
    std::string name;
    std::shared_ptr<LiteralNode> value;

    DeclarationNode(bool _is_const, std::string _type, std::string _name, std::shared_ptr<LiteralNode> _value, bool _is_reference)
        : is_const(_is_const), type(_type), name(_name), value(_value), is_reference(_is_reference) {}
};

class AssignNode : public ASTNode {
public:
    std::string target_variable;
    std::string source_variable;
    bool is_reference;

    AssignNode(const std::string& _target, const std::string& _source, bool _is_reference)
        : target_variable(_target), source_variable(_source), is_reference(_is_reference) {}
};

class LogNode : public ASTNode {
public:
    std::shared_ptr<LiteralNode> value;
    std::string variable_name;
    bool is_variable;

    LogNode(const std::string& _var_name) : value(nullptr), variable_name(_var_name), is_variable(true) {}

    LogNode(std::shared_ptr<LiteralNode> _value) : value(_value), is_variable(false) {}
};

class MultiOpNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> operands;
    std::vector<std::string> operators;
    MultiOpNode(const std::vector<std::shared_ptr<ASTNode>>& _operands, const std::vector<std::string>& _operators)
        : operands(_operands), operators(_operators) {}
};

class ParseError : public  std::runtime_error {
public:
    int line;
    int column;

    ParseError(const std::string& _message, int _line, int _column) : std::runtime_error(_message), line(_line), column(_column) {}
};

class Lexer {
    const std::string& input;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    void skip_white_space() {
        while (pos < input.size()) {
            if (std::isspace(input[pos])) {
                if (input[pos] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                pos++;
            } else if (input[pos] == '/' && pos + 1 < input.size()) {
                if (input[pos + 1] == '/') {
                    while (pos < input.size() && input[pos] != '\n') {
                        pos++;
                    }
                } else if (input[pos + 1] == '*') {
                    pos += 2;
                    while (pos < input.size() && !(input[pos] == '*' && pos + 1 < input.size() && input[pos + 1] == '/')) {
                        if (input[pos] == '\n') {
                            line++;
                            column = 1;
                        } else {
                            column++;
                        }
                        pos++;
                    }
                    if (pos < input.size()) {
                        pos += 2;
                    }
                } else {
                    break;
                }
            } else {
                break;
            }
        }
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
            } else if(word == "true" || word == "false") {
                return {TokenType::BOOL, word, tok_line, tok_column};
            }else {
                return {TokenType::Identifier, word, tok_line, tok_column};
            }
        }
        if(character_to_analyse == '"') {
            advance();
            size_t start = pos;
            while(pos < input.size() && peek() != '"') advance();
            std::string string = input.substr(start, pos - start);
            advance();
            return {TokenType::STRING, string, tok_line, tok_column};
        }
        if(std::isdigit(character_to_analyse)) {
            size_t start = pos;
            bool has_dot = false;
            while(pos < input.size() && (std::isdigit(input[pos]) || (input[pos] == ',' || input[pos] == '.'))) {
                if(input[pos] == ',' || input[pos] == '.') has_dot = true;
                advance();
            }
            std::string number = input.substr(start, pos - start);
            return {TokenType::Number, number, tok_line, tok_column};
        } 

        advance();
        return {TokenType::Symbol, std::string(1, character_to_analyse), tok_line, tok_column};
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
            throw ParseError("Unexpected token: '" + current_token.value + "'", current_token.line, current_token.column);
        }
        next_token();
    }

    std::shared_ptr<LiteralNode> eval_expression(const std::string& expected_type) {
        std::function<std::string()> parse_expression;
        std::function<std::string()> parse_factor;
        std::function<std::string()> parse_primary;

        parse_primary = [&]() {
            if (current_token.type == TokenType::Number) {
                std::string val = current_token.value;
                next_token();
                return val;
            } else if (current_token.type == TokenType::Identifier && current_token.value == "sqrt") {
                next_token();
                expect(TokenType::Symbol, "(");
                std::string val = parse_expression();
                expect(TokenType::Symbol, ")");
                return "sqrt(" + val + ")";
            } else if (current_token.type == TokenType::Identifier) {
                std::string var_name = current_token.value;
                next_token();
                return var_name;
            } else if (current_token.type == TokenType::Symbol && current_token.value == "-") {
                next_token();
                return "-" + parse_primary();
            } else {
                throw ParseError("Expected number, variable or sqrt", current_token.line, current_token.column);
            }
        };
        parse_factor = [&]() {
            std::string left = parse_primary();
            while (current_token.type == TokenType::Symbol && (current_token.value == "*" || current_token.value == "/" || current_token.value == "%")) {
                std::string op = current_token.value;
                next_token();
                std::string right = parse_primary();
                left = "(" + left + " " + op + " " + right + ")";
            }
            return left;
        };
        parse_expression = [&]() {
            std::string left = parse_factor();
            while (current_token.type == TokenType::Symbol && (current_token.value == "+" || current_token.value == "-")) {
                std::string op = current_token.value;
                next_token();
                std::string right = parse_factor();
                left = "(" + left + " " + op + " " + right + ")";
            }
            return left;
        };
        std::string expr = parse_expression();
        if (expected_type == "int") {
            return std::make_shared<IntNode>(expr);
        } else {
            return std::make_shared<FloatNode>(expr);
        }
    }

public:
    Parser(const std::string& _input) : lexer(_input) {
        next_token();
    }

    std::shared_ptr<LiteralNode> parse_value(const std::string _type) {
        if(_type == "int" || _type == "float") {
            if(current_token.type == TokenType::Number || current_token.type == TokenType::Identifier || (current_token.type == TokenType::Symbol && current_token.value == "-")) {
                return eval_expression(_type);
            } else if(current_token.type == TokenType::Identifier) {
                std::string var_name = current_token.value;
                next_token();
                return std::make_shared<LiteralNode>(_type, var_name, true);
            }
        } else if (_type == "bool") {
            if (current_token.value == "true" || current_token.value == "false") {
                auto value = current_token.value;
                next_token();
                return std::make_shared<BoolNode>(value);
            } else if (current_token.type == TokenType::Identifier) {
                std::string var_name = current_token.value;
                next_token();
                return std::make_shared<LiteralNode>(_type, var_name, true);
            }
        } else if (_type == "string") {
            if (current_token.type == TokenType::STRING) {
                auto value = current_token.value;
                next_token();
                return std::make_shared<StringNode>(value);
            } else if (current_token.type == TokenType::Identifier) {
                std::string var_name = current_token.value;
                next_token();
                return std::make_shared<LiteralNode>(_type, var_name, true);
            }
        }
        throw ParseError("Unknown type", current_token.line, current_token.column);
    }

    std::shared_ptr<ASTNode> parse_declaration(bool is_const) {
        next_token();
        if(current_token.type != TokenType::Type) {
            throw ParseError("Expected type", current_token.line, current_token.column);
        }
        std::string type = current_token.value;
        next_token();

        if (current_token.type != TokenType::Identifier)
            throw ParseError("Expected identifier", current_token.line, current_token.column);
        std::string name = current_token.value;
        next_token();

        expect(TokenType::Symbol, "=");
        auto value_node = parse_value(type);

        return std::make_shared<DeclarationNode>(is_const, type, name, value_node, false);
    }

    std::shared_ptr<ASTNode> parse_assign() {        
        if (current_token.type != TokenType::Identifier) {
            throw ParseError("Expected target variable", current_token.line, current_token.column);
        }

        std::string target = current_token.value;

        next_token();

        expect(TokenType::Symbol, "=");

        if(current_token.type == TokenType::Identifier) {
            std::string source = current_token.value;
            next_token();
            return std::make_shared<AssignNode>(target, source, true);
        } else if(current_token.type == TokenType::Number || current_token.type == TokenType::STRING || current_token.type == TokenType::BOOL) {
            std::string source = current_token.value;
            next_token();
            return std::make_shared<AssignNode>(target, source, false);
        } else {
            throw ParseError("Expected a value or variable after '='", current_token.line, current_token.column);
        }
    }

    std::shared_ptr<ASTNode> parse_let() {
        return parse_declaration(false);
    }

    std::shared_ptr<ASTNode> parse_const() {
        return parse_declaration(true);
    }

    std::shared_ptr<ASTNode> parse_log() {
        expect(TokenType::Identifier, "log");
        expect(TokenType::Symbol, "(");

        if(current_token.type == TokenType::Identifier) {
            std::string var_name = current_token.value;
            next_token();
            expect(TokenType::Symbol, ")");   
            return std::make_shared<LogNode>(var_name);
        } else {
            if(current_token.type == TokenType::Number) {
                std::string value = current_token.value;
                expect(TokenType::Symbol, ")");   
                next_token();
                if(value.find('.') != std::string::npos || value.find(',') != std::string::npos) {
                    return std::make_shared<LogNode>(std::make_shared<FloatNode>(value));
                } else {
                    return std::make_shared<LogNode>(std::make_shared<IntNode>(value));
                }
            } else if(current_token.type == TokenType::STRING) {
                std::string value = current_token.value;
                next_token();
                expect(TokenType::Symbol, ")");   
                return std::make_shared<LogNode>(std::make_shared<StringNode>(value));
            } else if(current_token.type == TokenType::BOOL) {
                std::string value = current_token.value;
                next_token();
                expect(TokenType::Symbol, ")");   
                return std::make_shared<LogNode>(std::make_shared<BoolNode>(value));
            } else {
                throw ParseError("Invalid value for log", current_token.line, current_token.column);
            }
        }
    }
};
#endif