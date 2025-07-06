#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"

#include <functional>

/**
 * @file parser.hpp
 * @brief Define the parser to analyze the source code and create an abstract syntax tree (AST).
 * @author Arthur
 * @version 1.0
 * @date 2025-07-05
 */

class Parser {
    /**
     * @class Parser
     * @brief Parses the source code and builds an abstract syntax tree (AST).
     */

    Lexer lexer;            // Lexer instance to take tokens from the source code.
    Token current_token;    // Current token to analyse.

    void next_token() {
        /**
         * @brief Advance to the next token.
         */
        current_token = lexer.next_token();
    }

    void expect(TokenType _type, const std::string& _value = "") {
        /**
         * @brief Check if the current token matches the expected type and value.
         * @brief Throws a ParseError if the current token does not match.
         * @param _type Expected token type.
         * @param _value Expected token value (optional).
         * @throws ParseError if the current token does not match the expected type and value.
         */
        if (current_token.type != _type || (!_value.empty() && current_token.value != _value)) {
            throw ParseError("Unexpected token: '" + current_token.value + "'", current_token.line, current_token.column);
        }
        next_token();
    }

    std::shared_ptr<BoolNode> eval_bool_expression() {
        /**
         * @brief Evaluate a complex boolean expression and return a BoolNode.
         * @brief Function recursively lambda to parse boolean expressions.
         * @return A shared pointer to a BoolNode representing the evaluated boolean expressions.
         * @throw ParseError if the expressions are invalid or if an unexpected token is encountered.
         */
        std::function<bool()> parse_expression;
        std::function<bool()> parse_primary;
        std::function<bool()> parse_not;
        std::function<bool()> parse_and;

        auto to_bool = [](const std::string& _value) -> bool {
            /**
             * @brief Convert a string ("true" or "false") to a boolean value.
             * @param _value The string to convert.
             * @return True if the string is 'true', false if it is 'false'.
             */
            return _value == "true";
        };

        parse_primary = [&]() -> bool {
            /**
             * @brief Parse a primary boolean expressions.
             * @brief Handles parentheses, boolean literals, variables, and comparisons.
             * @return True or false based on the parsed expressions.
             * @throw ParseError if the expressions are invalid or if an unexpected token is encountered.
             * @note This function does not handle logical operators with variable names.
             * @note This function will integrate operations with variables in the future.
             */
            if (current_token.type == TokenType::Symbol && current_token.value == "(") {
                next_token();
                bool val = parse_expression();
                expect(TokenType::Symbol, ")");
                return val;
            } else if (current_token.type == TokenType::Bool) {
                bool val = to_bool(current_token.value);
                next_token();
                return val;
            } else if (current_token.type == TokenType::Number || current_token.type == TokenType::Identifier || (current_token.type == TokenType::Symbol && current_token.value == "(")) {
                /**
                 * @brief Comparison like 5 < 10, 5 > 10, etc. with digit is supported.
                 * @note This function will extend to handle variables name in the future.
                 */
                inline std::shared_ptr<LiteralNode> left = eval_expression("float");
                if ((current_token.type == TokenType::Symbol || current_token.type == TokenType::BooleanOperator) &&
                   (current_token.value == "<" || current_token.value == ">" ||
                    current_token.value == "<=" || current_token.value == ">=" ||
                    current_token.value == "==" || current_token.value == "!=")) {
                    std::string op = current_token.value;
                    next_token();
                    inline std::shared_ptr<LiteralNode> right = eval_expression("float");
                    float left_value = std::stof(left->value);
                    float right_value = std::stof(right->value);
                    /**
                     * @brief Evaluate the comparison operation and return the result.
                     * @brief supported operations are <, >, <=, >=, ==, !=.
                     */
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
            /**
             * @brief Parse a NOT operation
             * @brief Handles logical NOT (!) operations.
             * @return True or false based on the parsed expressions.
             * @note This function does not handle logical operators with variable names.
             * @note This function will integrate operations with variables in the future.
             */
            if (current_token.type == TokenType::BooleanOperator && current_token.value == "!") {
                next_token();
                return !parse_not();
            } else {
                return parse_primary();
            }
        };

        parse_and = [&]() -> bool {
            /**
             * @brief Parse AND operations
             * @brief Handles logical AND (&&) and NAND (!&&) operations.
             * @return True or false based on the parsed expressions.
             * @note This function does not handle logical operators with variable names.
             * @note This function will integrate operations with variables in the future.
             */
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
            /**
             * @brief Parse complex boolean expressions.
             * @brief Handles logical OR (||), NOR (!||), XOR (xor), NXOR (nxor), implications (=>, !=>) and comparisons (<, >, <=, >=, ==, !=).
             * @return True or false based on the parsed expressions.
             * @note This function does not handle logical operators with variable names.
             * @note This function will integrate operations with variables in the future.
             */
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
                
                /**
                 * @brief Evaluate the logical operation based on the operator.
                 * @brief supported operations are ||, !||, xor, nxor, ==, !=, =>, !=>, <, <=, >, >=.
                 */
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
        /**
         * @brief Evaluate a mathematical expression and return a LiteralNode.
         * @brief Function recursively lambda to parse mathematical expressions.
         * @param _expected_type Expected type of the result ("int" or "float").
         * @return A shared pointer to a LiteralNode representing the evaluated expression.
         */
        std::function<std::string()> parse_expression;
        std::function<std::string()> parse_factor;
        std::function<std::string()> parse_primary;

        parse_primary = [&]() {
            /**
             * @brief Parse a primary expression.
             * @brief Handles numbers, variables, parentheses, square roots and negation.
             * @return A string representing the primary expression.
             * @throw ParseError if the primary expression is invalid or if an unexpected token is encountered
             */
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
            /**
             * @brief Parse a factor in the expressions.
             * @brief Handles multiplication, division, and modulus operations.
             * @return A string representing the factor expression.
             */
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
            /**
             * @brief Parse a complete arithmetic expression.
             * @brief Handles addition and subtraction operations.
             * @return A string representing the complete expression.
             */
            std::string left = parse_factor();
            while (current_token.type == TokenType::Symbol && (current_token.value == "+" || current_token.value == "-")) {
                std::string op = current_token.value;
                next_token();
                std::string right = parse_factor();
                left = "(" + left + " " + op + " " + right + ")";
            }
            return left;
        };

        /**
         * @brief Parse the expression and return a LiteralNode.
         * @return A shared pointer to the created LiteralNode.
         * @note The type of the LiteralNode is determined by the _expected_type parameter.
         */
        std::string expr = parse_expression();
        if (_expected_type == "int") {
            return std::make_shared<IntNode>(expr);
        } else {
            return std::make_shared<FloatNode>(expr);
        }
    }

public:
    /**
     * @brief Constructor for the Parser class and charge the first token.
     * @param _input The source code to parse.
     */
    Parser(const std::string& _input) : lexer(_input) {
        next_token();
    }

    std::shared_ptr<LiteralNode> parse_value(const std::string _type) {
        /**
         * @brief Parse a value based on the specified type (int, float, bool, string).
         * @param _type The type of the value to parse ("int", "float", "bool", "string").
         * @return A shared pointer to a LiteralNode representing the parsed.
         * @throw ParseError if the current token does not match the expected type or if an unexpected token is encountered.
         */
        if (_type == "int" || _type == "float") {
            // If the current token is a number, identifier, or a negative sign or parenthesis, parse the expression.
            if (current_token.type == TokenType::Number || current_token.type == TokenType::Identifier ||
               (current_token.type == TokenType::Symbol && (current_token.value == "-" || current_token.value == "("))) {
                return eval_expression(_type);
            }
        } else if (_type == "bool") {
            /**
             * @brief Parse a complex boolean value.
             * @brief Handles boolean literals (true, false), boolean operators, identifiers, and numbers.
             * @return A shared pointer to a BoolNode representing the parsed boolean value.
             * @throw ParseError if the current token does not match the expected type or if an unexpected token is encountered.
             */
            if (current_token.value == "true" || current_token.value == "false") {
                std::string value = current_token.value;
                next_token();
                return std::make_shared<BoolNode>(value);
            } else if (current_token.type == TokenType::BooleanOperator || current_token.type == TokenType::Symbol ||
                       current_token.type == TokenType::Identifier || current_token.type == TokenType::Bool ||
                       current_token.type == TokenType::Number) {
                inline std::shared_ptr<BoolNode> boolNode = eval_bool_expression();
                return boolNode;
            }
        } else if (_type == "string") {
            // If the current token is a string or an identifier, parse the string value.
            if (current_token.type == TokenType::String) {
                std::string value = current_token.value;
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
        /**
         * @brief Parse a variable declaration (let or const).
         * @param _is_const True if the declaration is a constant, false if it is a variable.
         * @return A shared pointer to a DeclarationNode representing the parsed declaration.
         * @throw ParseError if the current token does not match the expected type or if an unexpected token is encountered.
         * @note The declaration can be a variable or a constant, depending on the _is_const parameter.
         * @note The declaration can be a reference or a value, depending on the current token.
         * @note The type of the variable is expected to be specified in the next token after the declaration keyword (let or const).
         * @note The variable name is expected to be specified in the next token after the type.
         * @note The value of the variable is expected to be specified after the '=' symbol.
         * @note The value can be a number, string, boolean, or a boolean expression.
         * @note The value can also be a variable name, in which case it is treated as a reference.
         * @note The declaration can be a reference or a value, depending on the current token.
         */
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
        inline std::shared_ptr<LiteralNode> value_node = parse_value(type);

        return std::make_shared<DeclarationNode>(_is_const, type, name, value_node, false);
    }

    std::shared_ptr<ASTNode> parse_assign() {
        /**
         * @brief Parse an assignment operation.
         * @brief Handles assignment of a value to a variable.
         * @return A shared pointer to an AssignNode representing the parsed assignment.
         * @throw ParseError if the current token does not match the expected type or if an unexpected token is encountered.
         * @note The assignment can be a reference or a value, depending on the current token.
         * @note The target variable is expected to be specified in the next token after the '=' symbol.
         * @note The source value can be a variable name, a number, a string, a boolean, or a boolean expression.
         * @note If the source value is a variable name, it is treated as a reference.
         * @note If the source value is a number, string, or boolean, it is treated as a value.
         * @note If the source value is a boolean expression, it is evaluated and returned as a BoolNode.
         */
        if (current_token.type != TokenType::Identifier) {
            throw ParseError("Expected target variable", current_token.line, current_token.column);
        }

        std::string target = current_token.value;
        next_token();
        expect(TokenType::Symbol, "=");

        // Source can be a variable, a literal, or a Boolean expression
        if (current_token.type == TokenType::Identifier) {
            std::string source = current_token.value;
            next_token();
            return std::make_shared<AssignNode>(target, source, true);
        } 
        else if (current_token.type == TokenType::Number || current_token.type == TokenType::String || current_token.type == TokenType::Bool) {
            std::string source = current_token.value;
            next_token();
            return std::make_shared<AssignNode>(target, source, false);
        }
        else if (current_token.type == TokenType::BooleanOperator || current_token.type == TokenType::Symbol || current_token.type == TokenType::Bool) {
            inline std::shared_ptr<BoolNode> expr = eval_bool_expression();
            return std::make_shared<AssignNode>(target, expr);
        }
        else {
            throw ParseError("Expected a value or variable after '='", current_token.line, current_token.column);
        }
    }

    // Parse "let" declaration
    std::shared_ptr<ASTNode> parse_let() {
        return parse_declaration(false);
    }

    // Parse "const" declaration
    std::shared_ptr<ASTNode> parse_const() {
        return parse_declaration(true);
    }

    // Parse "log" instruction
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
            } else if (current_token.type == TokenType::String) {
                std::string value = current_token.value;
                next_token();
                expect(TokenType::Symbol, ")");   
                return std::make_shared<LogNode>(std::make_shared<StringNode>(value));
            } else if (current_token.type == TokenType::Bool) {
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