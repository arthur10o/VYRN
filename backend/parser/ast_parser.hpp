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
    Unknown,
    BooleanOperator
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

static const std::unordered_set<std::string> boolean_operator = {
    "!",        // logical NOT
    "||",       // logical OR
    "!||",      // logical NOR
    "&&",       // logical AND
    "!&&",      // logical NAND
    "==",       // equality
    "!=",       // inequality
    "<",        // less than
    "<=",       // less than or equal to
    ">",        // greater than
    ">=",       // greater than or equal to
    "=>",       // implication
    "!=>",      // non-implication
    "xor",      // exclusive OR (XOR)
    "nxor"      // exclusive NOR (XNOR)
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
    std::shared_ptr<ASTNode> expr;

    AssignNode(const std::string& _target, const std::string& _source, bool _is_reference)
        : target_variable(_target), source_variable(_source), is_reference(_is_reference), expr(nullptr) {}

    AssignNode(const std::string& _target, std::shared_ptr<ASTNode> _expr)
        : target_variable(_target), source_variable(""), is_reference(false), expr(_expr) {}
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

class CompareNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> operands;
    std::vector<std::string> operators;

    CompareNode(const std::vector<std::shared_ptr<ASTNode>>& _operands, const std::vector<std::string>&  _operators)
        : operands(_operands), operators(_operators) {}
};

class MultiOpBoolNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> operands;
    std::vector<std::string> operators;

    MultiOpBoolNode(const std::vector<std::shared_ptr<ASTNode>>& _operands, const std::vector<std::string>& _operators)
        : operands(_operands), operators(_operators) {}
};

class ParseError : public std::runtime_error {
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
        if (pos >= input.size()) return '\0';
        char character_to_analyse = input[pos++];
        if (character_to_analyse == '\n') {
            line++;
            column = 1;
        } else {
            column ++;
        }
        return character_to_analyse;
    }

    bool is_identifier_character(char _character_to_identified) {
        return std::isalnum(_character_to_identified) || _character_to_identified == '_';
    }

public:
    Lexer(const std::string& _input) : input(_input) {}

    Token next_token() {
        skip_white_space();
        if (pos >= input.size()) return {TokenType::EndOfFile, "", line, column};

        char character_to_analyse = peek();
        int tok_line  = line;
        int tok_column = column;

        static const std::vector<std::string> two_character_operations = {
            "&&",
            "||",
            "==",
            "!=",
            "<=",
            ">=",
            "=>"
        };

        static const std::vector<std::string> three_character_operations = {
            "!&&",
            "!||",
            "!=>"
        };

        if (pos + 2 < input.size()) {
            std::string three = input.substr(pos, 3);
            for (const auto& op : three_character_operations) {
                if (three == op) {
                    pos += 3;
                    column += 3;
                    return {TokenType::BooleanOperator, op, tok_line, tok_column};
                }
            }
        }

        if (pos + 1 < input.size()) {
            std::string two = input.substr(pos, 2);
            for (const auto& op : two_character_operations) {
                if (two == op) {
                    pos += 2;
                    column += 2;
                    return {TokenType::BooleanOperator, op, tok_line, tok_column};
                }
            }
        }

        if (std::isalpha(character_to_analyse) || character_to_analyse == '_') {
            size_t start = pos;
            while(pos < input.size() && is_identifier_character(input[pos])) advance();
            std::string word = input.substr(start, pos - start);

            if (keywords.find(word) != keywords.end()) {
                return {TokenType::Keyword, word, tok_line, tok_column};
            } else if (types.find(word) != types.end()) {
                return {TokenType::Type, word, tok_line, tok_column};
            } else if (word == "true" || word == "false") {
                return {TokenType::BOOL, word, tok_line, tok_column};
            } else if (boolean_operator.find(word) != boolean_operator.end()) {
                return {TokenType::BooleanOperator, word, tok_line, tok_column};
            } else {
                return {TokenType::Identifier, word, tok_line, tok_column};
            }
        }

        if (character_to_analyse == '"') {
            advance();
            size_t start = pos;
            while (pos < input.size() && peek() != '"') advance();
            std::string string = input.substr(start, pos - start);
            advance();
            return {TokenType::STRING, string, tok_line, tok_column};
        }

        if (std::isdigit(character_to_analyse)) {
            size_t start = pos;
            bool has_dot = false;
            while (pos < input.size() && (std::isdigit(input[pos]) || (input[pos] == ',' || input[pos] == '.'))) {
                if (input[pos] == ',' || input[pos] == '.') has_dot = true;
                advance();
            }
            std::string number = input.substr(start, pos - start);
            return {TokenType::Number, number, tok_line, tok_column};
        } 

        if (character_to_analyse == '<' || character_to_analyse == '>') {
            pos++;
            column++;
            return {TokenType::BooleanOperator, std::string(1, character_to_analyse), tok_line, tok_column};
        }

        if (character_to_analyse == '!') {
            pos++;
            column++;
            return {TokenType::BooleanOperator, "!", tok_line, tok_column};
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

    void expect(TokenType _type, const std::string& _value = "") {
        if (current_token.type != _type || (!_value.empty() && current_token.value != _value)) {
            throw ParseError("Unexpected token: '" + current_token.value + "'", current_token.line, current_token.column);
        }
        next_token();
    }

    std::shared_ptr<BoolNode> eval_bool_expression() {
        std::function<bool()> parse_expression;
        std::function<bool()> parse_primary;
        std::function<bool()> parse_not;
        std::function<bool()> parse_and;

        auto to_bool = [](const std::string& _value) -> bool {
            return _value == "true";
        };

        parse_primary = [&]() -> bool {
            if (current_token.type == TokenType::Symbol && current_token.value == "(") {
                next_token();
                bool val = parse_expression();
                expect(TokenType::Symbol, ")");
                return val;
            } else if (current_token.type == TokenType::BOOL) {
                bool val = to_bool(current_token.value);
                next_token();
                return val;
            } else if (current_token.type == TokenType::Number || current_token.type == TokenType::Identifier || (current_token.type == TokenType::Symbol && current_token.value == "(")) {
                auto left = eval_expression("float");
                if ((current_token.type == TokenType::Symbol || current_token.type == TokenType::BooleanOperator) &&
                   (current_token.value == "<" || current_token.value == ">" ||
                    current_token.value == "<=" || current_token.value == ">=" ||
                    current_token.value == "==" || current_token.value == "!=")) {
                    std::string op = current_token.value;
                    next_token();
                    auto right = eval_expression("float");
                    float left_value = std::stof(left->value);
                    float right_value = std::stof(right->value);
                    if (op == "<") return left_value < right_value;
                    else if (op == ">") return left_value > right_value;
                    else if (op == "<=") return left_value <= right_value;
                    else if (op == ">=") return left_value >= right_value;
                    else if (op == "==") return left_value == right_value;
                    else if (op == "!=") return left_value != right_value;
                    else throw ParseError("unauthorized comparison operation", current_token.line, current_token.column);
                } else {
                    throw ParseError("unauthorized comparison operation", current_token.line, current_token.column);
                }
            } else {
                throw ParseError("Expected boolean, variable or parenthesis", current_token.line, current_token.column);
            }
        };

        parse_not = [&]() -> bool {
            if (current_token.type == TokenType::BooleanOperator && current_token.value == "!") {
                next_token();
                return !parse_not();
            } else {
                return parse_primary();
            }
        };

        parse_and = [&]() -> bool {
            bool left = parse_not();
            while (current_token.type == TokenType::BooleanOperator && (current_token.value == "&&" || current_token.value == "!&&")) {
                std::string op = current_token.value;
                next_token();
                bool right = parse_not();
                if (op == "&&") left = left && right;
                else if (op == "!&&") left = !(left && right);
            }
            return left;
        };

        parse_expression = [&]() -> bool {
            bool left = parse_and();
            while (current_token.type == TokenType::BooleanOperator && 
                (current_token.value == "||" || current_token.value == "!||" ||
                current_token.value == "xor" || current_token.value == "nxor" ||
                current_token.value == "=>" || current_token.value == "!=>" ||
                current_token.value == "<" || current_token.value == ">" ||
                current_token.value == "<=" || current_token.value == ">=" ||
                current_token.value == "==" || current_token.value == "!=")) {
                std::string op = current_token.value;
                next_token();
                bool right = parse_and();

                if (op == "||") left = left || right;
                else if (op == "!||") left = !(left || right);
                else if (op == "xor") left = left != right;
                else if (op == "nxor") left = left == right;
                else if (op == "==") left = left == right;
                else if (op == "!=") left = left != right;
                else if (op == "=>") left = (!left) || right;
                else if (op == "!=>") left = left && !right;
                else if (op == "<") left = (!left) && right;
                else if (op == "<=") left = (!left) || right;
                else if (op == ">") left = left && (!right);
                else if (op == ">=") left = left || (!right);
            }
            return left;
        };

        bool result = parse_expression();
        return std::make_shared<BoolNode>(result ? "true" : "false");
    }

    std::shared_ptr<LiteralNode> eval_expression(const std::string& _expected_type) {
        std::function<std::string()> parse_expression;
        std::function<std::string()> parse_factor;
        std::function<std::string()> parse_primary;

        parse_primary = [&]() {
            if (current_token.type == TokenType::Symbol && current_token.value == "(") {
                next_token();
                std::string val = parse_expression();
                expect(TokenType::Symbol, ")");
                return "(" + val + ")";
            } else if (current_token.type == TokenType::Number) {
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
                throw ParseError("Expected number, variable, parenthesis or sqrt", current_token.line, current_token.column);
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
        if (_expected_type == "int") {
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
        if (_type == "int" || _type == "float") {
            if (current_token.type == TokenType::Number || current_token.type == TokenType::Identifier ||
               (current_token.type == TokenType::Symbol && (current_token.value == "-" || current_token.value == "("))) {
                return eval_expression(_type);
            }
        } else if (_type == "bool") {
            if (current_token.value == "true" || current_token.value == "false") {
                auto value = current_token.value;
                next_token();
                return std::make_shared<BoolNode>(value);
            } else if (current_token.type == TokenType::BooleanOperator || current_token.type == TokenType::Symbol ||
                       current_token.type == TokenType::Identifier || current_token.type == TokenType::BOOL ||
                       current_token.type == TokenType::Number) {
                auto boolNode = eval_bool_expression();
                return boolNode;
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

    std::shared_ptr<ASTNode> parse_declaration(bool _is_const) {
        next_token();
        if (current_token.type != TokenType::Type) {
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

        return std::make_shared<DeclarationNode>(_is_const, type, name, value_node, false);
    }

    std::shared_ptr<ASTNode> parse_assign() {
        if (current_token.type != TokenType::Identifier) {
            throw ParseError("Expected target variable", current_token.line, current_token.column);
        }

        std::string target = current_token.value;
        next_token();
        expect(TokenType::Symbol, "=");

        if (current_token.type == TokenType::Identifier) {
            std::string source = current_token.value;
            next_token();
            return std::make_shared<AssignNode>(target, source, true);
        } 
        else if (current_token.type == TokenType::Number || current_token.type == TokenType::STRING || current_token.type == TokenType::BOOL) {
            std::string source = current_token.value;
            next_token();
            return std::make_shared<AssignNode>(target, source, false);
        }
        else if (current_token.type == TokenType::BooleanOperator || current_token.type == TokenType::Symbol || current_token.type == TokenType::BOOL) {
            auto expr = eval_bool_expression();
            return std::make_shared<AssignNode>(target, expr);
        }
        else {
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

        if (current_token.type == TokenType::Identifier) {
            std::string var_name = current_token.value;
            next_token();
            expect(TokenType::Symbol, ")");   
            return std::make_shared<LogNode>(var_name);
        } else {
            if (current_token.type == TokenType::Number) {
                std::string value = current_token.value;
                expect(TokenType::Symbol, ")");   
                next_token();
                if (value.find('.') != std::string::npos || value.find(',') != std::string::npos) {
                    return std::make_shared<LogNode>(std::make_shared<FloatNode>(value));
                } else {
                    return std::make_shared<LogNode>(std::make_shared<IntNode>(value));
                }
            } else if (current_token.type == TokenType::STRING) {
                std::string value = current_token.value;
                next_token();
                expect(TokenType::Symbol, ")");   
                return std::make_shared<LogNode>(std::make_shared<StringNode>(value));
            } else if (current_token.type == TokenType::BOOL) {
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