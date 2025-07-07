#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "parser.hpp"

#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <sstream>

enum class SymbolKind {
    /**
     * @enum SymbolKind
     * @brief Represents the kind of symbol in the symbol table.
     */
    VARIABLE,
    CONSTANT
};

struct SymbolInfo {
    /**
     * @struct SymbolInfo
     * @brief Contains information about a symbol in the symbol table.
     */
    std::string type;           /**< Type of the symbol (e.g., int, float, string) */
    std::string value;          /**< Value of the symbol */
    bool is_reference;          /**< True if the symbol is a reference, false if it is a value */
    SymbolKind kind;            /**< Kind of the symbol (variable or constant) */
};

std::ostringstream out;
std::unordered_map<std::string, SymbolInfo> symbol_table;

std::string generate(const std::shared_ptr<ASTNode>& _node) {
    /**
     * @fn generate
     * @brief Generates C++ code from the given AST node.
     * @param _node The AST node to generate code for.
     * @return A string containing the generated C++ code.
     * @details This function clears the output stream and generates code for the provided AST node.
     */
    out.str("");
    out.clear();
    generate_node(_node);
    return out.str();
}

void generate_node(const std::shared_ptr<ASTNode>& _node) {
    /**
     * @fn generate_node
     * @brief Generates C++ code for a specific AST node.
     * @param _node The AST node to generate code for.
     * @details This function checks the type of the AST node and calls the appropriate generation function
     * based on the node type. It handles declaration nodes, log nodes, assignment nodes,
     * multi-operation nodes, and multi-operation boolean nodes.
     * If the node type is not recognized, it outputs a comment indicating an unknown node.
     * @note This function does not handle indentation levels; it assumes the caller will handle that
     * if necessary.
     * @note The function uses dynamic_pointer_cast to determine the type of the node.
     */
    const std::shared_ptr<DeclarationNode> declaration_node = std::dynamic_pointer_cast<DeclarationNode>(_node);
    const std::shared_ptr<LogNode> log_node = std::dynamic_pointer_cast<LogNode>(_node);
    const std::shared_ptr<AssignNode> assign_node = std::dynamic_pointer_cast<AssignNode>(_node);
    const std::shared_ptr<MultiOpNode> multi_op_node = std::dynamic_pointer_cast<MultiOpNode>(_node);
    const std::shared_ptr<MultiOpBoolNode> multi_op_bool_node = std::dynamic_pointer_cast<MultiOpBoolNode>(_node);

    if (declaration_node) {
        SymbolKind declaration_node_is_const = (declaration_node->is_const ? SymbolKind::CONSTANT : SymbolKind::VARIABLE);
        generate_declaration(declaration_node, declaration_node_is_const);
    } else if (log_node) {
        generate_log(log_node);
    } else if (assign_node) {
        generate_assign(assign_node);
    } else if (multi_op_node) {
        out << "// Multi-op expression not evaluated at compile time (should be evaluated in parser)\n";
    } else if (multi_op_bool_node) {
        out << "// Multi-op bool expression not evaluated at compile time (should be evaluated in parser)\n";
    } else {
        out << "// Unknown node\n";
    }
}

void generate_declaration(const std::shared_ptr<DeclarationNode>& _node, const SymbolKind& _kind) {
    /**
     * @fn generate_declaratio
     * @brief Function to generate declaration and to save all information on a table name SymbolTable.
     * @brief If variable is already declared with a name and wet try to rename it, it thrwo an error.
     * @brief If a we try to reassign a constante, it throw an error.
     * @param _node The node to declared.
     * @param _kind The kind of the value.
     * @return Genenetaion of on C++.
     */
    bool is_declared = var_is_declared(_node->name, _node->is_const);
    if (is_declared) {
        out << "// Warning : " << (_kind == SymbolKind::CONSTANT ? "constant" : "variable") << " '" << _node->name << "' " << "already declared\n";
    } else {
        symbol_table[_node->name] = SymbolInfo {_node->type, _node->value->value, _node->is_reference, _kind};
    }
    out << (_kind == SymbolKind::CONSTANT ? "const " : "") << convert_type(_node->type) << " " << _node->name << " = ";

    const std::shared_ptr<MultiOpBoolNode> multi_op_bool_node = std::dynamic_pointer_cast<MultiOpBoolNode>(_node->value);
    if (multi_op_bool_node) {
        out << generate_multi_bool_node(multi_op_bool_node);
    } else if (_node->type == "string" && _node->is_reference) {
        out << "\'" << _node->value->value << "\'";
    } else {
        out << (_node->is_reference ? _node->value->value : format_literal(_node->value));
    }
    out << ";\n";
}

void generate_assign(const std::shared_ptr<AssignNode>& _node) {
    /**
     * @fn generate_assign
     * @brief Function to generate assignation of a variable.
     * @param _node The node to assign.
     * @details This function checks if the target variable is declared and not a constant.
     * If the variable is not declared, it outputs an error comment.
     * If the variable is a constant, it outputs an error comment indicating that assignment to a constant is not allowed.
     * If the variable is declared and not a constant, it generates the assignment statement.
     * If the expression is a MultiOpBoolNode, it generates the multi-operation boolean expression.
     * If the expression is a BoolNode, it formats the literal value.
     * If the expression is not recognized, it outputs a comment indicating unsupported expression.
     * If the source variable is a reference, it uses the source variable directly.
     * If the source variable is a string and does not contain quotes, it adds quotes around it.
     * Otherwise, it formats the literal value using the format_literal function.
     */
    bool is_declared = var_is_declared(_node->target_variable);
    if (!is_declared) {
        out << "//Error variable " + _node->target_variable + " is not declared\n";
    } else if (is_const(_node->target_variable)) {
        out << "// Error: cannot assign to constant '" << _node->target_variable << "'\n";
    } else {
        out << _node->target_variable << " = ";
        if (_node->expr) {
            if (std::shared_ptr<MultiOpBoolNode> multiop = std::dynamic_pointer_cast<MultiOpBoolNode>(_node->expr)) {
                out << generate_multi_bool_node(multiop);
            } else if (std::shared_ptr<BoolNode> bool_lit = std::dynamic_pointer_cast<BoolNode>(_node->expr)) {
                out << format_literal(bool_lit);
            } else {
                out << "/* unsupported expr */";
            }
        } else if (_node->is_reference) {
            out << _node->source_variable;
        } else {
            if (symbol_table[_node->target_variable].type == "string" && _node->source_variable.find('"') == std::string::npos) {
                out << "\"" << _node->source_variable << "\"";
            } else {
                out << format_literal(std::make_shared<LiteralNode>("", _node->source_variable));
            }
        }
        out << ";\n";
    }
}

void generate_log(const std::shared_ptr<LogNode>& _node) {
    /**
     * @fn generate_log
     * @brief Function to generate logging of a variable or literal value.
     * @param _node The node to log.
     * @details This function generates a C++ logging statement using std::cout.
     * If the node represents a variable, it checks if the variable is declared in the symbol table.
     * If the variable is declared, it outputs the variable name; otherwise, it outputs an error message indicating that the variable is undefined.
     * If the node represents a literal value, it formats the value using the format_literal function.
     */
    out << "std::cout";
    if (_node->is_variable) {
        if (symbol_table.count(_node->variable_name)) {
            out << _node->variable_name;
        } else {
            out << "\"[Undefined variable: " << _node->variable_name << "]\"";
        }
    } else {
        out << format_literal(_node->value);
    }
    out << "std::endl;\n";
}

std::string generate_multi_bool_node(const std::shared_ptr<MultiOpBoolNode>& _node) {
    /**
     * @fn generate_multi_bool_node
     * @brief Generates a string representation of a multi-operation boolean node.
     * @param _node The multi-operation boolean node to generate the expression for.
     * @return A string containing the generated expression.
     * @details This function iterates through the operands and operators of the multi-operation boolean node,
     * formatting each operand and operator into a string expression. If an operand is another MultiOpBoolNode,
     * it recursively generates its expression enclosed in parentheses. If an operand is a LiteralNode, it formats
     * the literal value using the format_literal function. If an operand is an ASTNode, it outputs a comment
     * indicating that the operation is unsupported.
     */
    if (!_node || _node->operands.empty()) return "";
    std::ostringstream expression;
    for (size_t i = 0; i < _node->operands.size(); ++i) {
        if (std::shared_ptr<MultiOpBoolNode> sub = std::dynamic_pointer_cast<MultiOpBoolNode>(_node->operands[i])) {
            expression << "(" << generate_multi_bool_node(sub) << ")";
        } else if (std::shared_ptr<LiteralNode> lit = std::dynamic_pointer_cast<LiteralNode>(_node->operands[i])) {
            expression << "(" << format_literal(lit) << ")";
        } else if (std::shared_ptr<ASTNode> var = std::dynamic_pointer_cast<ASTNode>(_node->operands[i])) {
            if (std::shared_ptr<LiteralNode> l = std::dynamic_pointer_cast<LiteralNode>(var)) {
                expression << l->value;
            } else {
                expression << "// Unsuported operation";
            }
        }
        if (i < _node->operators.size()) {
            expression << " " + _node->operators[i] + " ";
        }
    }
    return expression.str();
}

std::string convert_type(const std::string& _type_to_convert) {
    /**
     * @fn convert_type
     * @brief If the type of variabele or constant is a string, is it convert on "std::string" to be understand by C++;
     * @param _type_to_convert The type to convert.
     * @return The type write in C++ language.
     */
    if (_type_to_convert == "string") {
        return "std::string";
    }
    return _type_to_convert;
}

std::string format_literal(const std::shared_ptr<LiteralNode>& _node) {
    /**
     * @fn format_literal
     * @brief Convert the value in literal format associate to his type.
     * @param _node The value to convert in literal format.
     * @return The value in good format.
     */
    if (_node->type == "string") {
        return "\'" + _node->value + "\'";
    } else if (_node->type == "bool") {
        return (_node->value == "true") ? "true" : "false";
    } else if (_node->type == "int") {
        return _node->value;
    } else if (_node->type == "float") {
        std::string value = _node->value;
        std::replace(value.begin(), value.end(), ',', '.');
        return value;
    }
}

bool var_is_declared(const std::string& _var_name, const bool& _is_const = false) {
    /**
     * @fn var_is_declared
     * @brief Check if a variable or a constant is already declared or not.
     * @param _var_name The name of the variable to check.
     * @param _is_const If is a variable _is_const = false, else _is_const = true. 
     */
    std::unordered_map<std::string, SymbolInfo>::iterator iterator = symbol_table.find(_var_name);
    if (iterator != symbol_table.end()) {
        return (_is_const && iterator->second.kind == SymbolKind::CONSTANT) || (!_is_const && iterator->second.kind == SymbolKind::VARIABLE);
    }
    return false;
}

bool is_const(std::string& _var_name) {
    /**
     * @fn is_const
     * @brief Check if a variable is a constant.
     * @param _var_name The name of the variable to check.
     * @return True if the variable is a constant, false otherwise.
     */
    std::unordered_map<std::string, SymbolInfo>::iterator iterateur = symbol_table.find(_var_name);
    if (iterateur != symbol_table.end()) {
        return iterateur->second.kind == SymbolKind::CONSTANT;
    }
    return false;
}

std::string trim(const std::string& _s) {
    /**
     * @fn trim
     * @brief Remove leading and trailing whitespace from a string.
     * @param _s The string to trim.
     * @return A new string with leading and trailing whitespace removed.
     * @details This function uses find_first_not_of and find_last_not_of to locate the first and last non-whitespace characters in the string.
     * If the string is empty or contains only whitespace, it returns an empty string.
     * Otherwise, it returns a substring containing the trimmed content.
     */
    auto start = _s.find_first_not_of(" \t\r\n");
    auto end = _s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : _s.substr(start, end - start + 1);
}

std::vector<std::string> split_instructions(const std::string& _code) {
    /**
     * @fn split_instructions
     * @brief Split the code into individual instructions based on semicolons.
     * @param _code The code to split.
     * @return A vector of strings, each representing an instruction.
     * @details This function processes the input code, ignoring comments and whitespace,
     * and splits it into individual instructions based on semicolons. It handles both single-line
     * and multi-line comments, ensuring that comments do not interfere with the splitting process.
     */
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

#endif