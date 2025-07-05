#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_set>

/**
 * @file parser.hpp
 * @brief Define all structures and tokens used in the Abstract Syntax Tree (AST).
 * @author Arthur
 * @version 1.0
 * @date 2025-07-05
 */

 static const std::unordered_set<std::string> keywords = {
    /**
     * @brief All reserved keywords.
     */
    "let",
    "const"
};

static const std::unordered_set<std::string> types = {
    /**
     * @brief All supported primitive types.
     */
    "int",
    "float",
    "bool",
    "string"
};

static const std::unordered_set<std::string> boolean_operator = {
    /**
     * @brief All supported boolean operators.
     */
    "!",                        /**< logical NOT */
    "||",                       /**< logical OR */
    "!||",                      /**< logical NOR */
    "&&",                       /**< logical AND */
    "!&&",                      /**< logical NAND */
    "==",                       /**< equality */
    "!=",                       /**< inequality */
    "<",                        /**< less than */
    "<=",                       /**< less than or equal to */
    ">",                        /**< greater than */
    ">=",                       /**< greater than or equal to */
    "=>",                       /**< implication */
    "!=>",                      /**< non-implication */
    "xor",                      /**< exclusive OR */
    "nxor"                      /**< non-exclusive OR */
};

enum class TokenType {
    /**
     * @enum TokenType
     * @brief Types of tokens recognized by the lexer.
     */
    Identifier,                 /**< Identifier (variable name, function, etc.) */
    Keyword,                    /**< Reserved keyword */
    Type,                       /**< Primitive type */
    Number,                     /**< Number (integer or floating point) */
    String,                     /**< String */
    Bool,                       /**< Boolean value */
    Symbol,                     /**< Punctuation symbol, non-boolean operator */
    BooleanOperator,            /**< Boolean operator */
    EndOfFile,                  /**< End of file */
    Unknown                     /**< Unknown or invalid token */
};

struct Token {
    /**
     * @struct Token
     * @brief Structure representing a token.
     */
    TokenType type;             /**< Type of the token */
    std::string value;          /**< Value of the token */
    int line;                   /**< Line number of the token */
    int column;                 /**< Column number of the token */
};

class ASTNode {
/**
 * @class ASTNode
 * @brief Base class for all AST nodes (Abstract Syntax Tree nodes).
 * @brief This class serves to represent different structures in the source code.
 */
public:
    virtual ~ASTNode() = default;   // @method Destructor virtual to garantee memory was properly released during the supression of derived classes.
};

class LiteralNode : public ASTNode {
/**
 * @class LiteralNode
 * @brief Represents a literal node, in other words, a simple value (e.g., an integer, float, string, or boolean).
 * @brief This class derives from ASTNode and contains the information about the type, value, and whether it is a reference.
 */
public:
    std::string type;               // @field Type of literal (e.g., "int", "float", "string", "bool").
    std::string value;              // @field value of the variable in string form
    bool is_reference = false;      // @field Indicates if the literal is a reference to a variable (true) or a value (false).

    // @constructor
    // Initializes the node with a type, value, and whether it is a reference (optional).
    // @param _type The type of the literal (e.g., "int", "float", "string", "bool").
    // @param _value The value of the literal as a string.
    // @param _is_reference Indicates if the literal is a reference to a variable (true) or a value (false).
    LiteralNode(const std::string& _type, const std::string& _value, bool _is_reference = false) 
        : type(_type), value(_value), is_reference(_is_reference) {}
};

class IntNode : public LiteralNode {
/**
 * @class IntNode
 * @brief Specific node to represent an integer in the AST.
 * @brief Inherits from LiteralNode and always has the type "int".
 */
public:
    // @constructor
    // Initializes the node how represent an integer.
    // @param _value The value of the integer as a string.
    IntNode(const std::string& _value)
        : LiteralNode("int", _value) {}
};

class FloatNode : public LiteralNode {
/**
 * @class FloatNode
 * @brief Specific node to represent a floating-point number in the AST.
 * @brief Inherit from LiteralNode and always has the type "float".
 */
public:
    // @constructor
    // Initializes the node how represent a floating-point number.
    // @param _value The value of the floating-point number as a string.
    FloatNode(const std::string& _value)
        : LiteralNode("float", _value) {}
};

class StringNode : public LiteralNode {
/**
 * @class StringNode
 * @brief Specific node to represent a string in the AST.
 * @brief Inherits from LiteralNode and always has the type "string".
 */
public:
    // @constructor
    // Initializes the node how represent a string.
    // @param _value The value of the string as a string.
    StringNode(const std::string& _value)
        : LiteralNode("string", _value) {}
};

class BoolNode : public LiteralNode {
/**
 * @class BoolNode
 * @brief Specific node to represent a boolean value in the AST.
 * @brief Inherits from LiteralNode and always has the type "bool".
 */
public:
    // @constructor
    // Initializes the node how represent a boolean value.
    // @param _value The value of the boolean as a string ("true" or "false").
    BoolNode(const std::string& _value)
        : LiteralNode("bool", _value) {}
};

class DeclarationNode : public ASTNode {
/**
 * @class DeclarationNode
 * @brief Represents a variable or constant declaration in the AST.
 * @brief Contains information about whether it is a constant, the type, name, and value of the declaration.
 * @brief Inherits from ASTNode and is used to represent variable declarations in the source code.
 * @note The "is_reference" field indicates if the declaration is a reference type.
 * @note The "value" field is a shared pointer to a LiteralNode, which can be an IntNode, FloatNode, StringNode, or BoolNode.
 * @note The "is_const" field indicates if the declaration is a constant (true) or a variable (false).
 * @note The "type" field indicates the type of the variable (e.g., "int", "float", "string", "bool").
 * @note The "name" field is the name of the variable or constant being declared.
 */
public:
    bool is_const;
    bool is_reference;
    std::string type;
    std::string name;
    std::shared_ptr<LiteralNode> value;

    // @constructor
    // Initializes a declartion node with the specified parameters.
    DeclarationNode(bool _is_const, std::string _type, std::string _name, std::shared_ptr<LiteralNode> _value, bool _is_reference)
        : is_const(_is_const), type(_type), name(_name), value(_value), is_reference(_is_reference) {}
};

class AssignNode : public ASTNode {
/**
 * @class AssignNode
 * @brief Represents an assignment operation in the AST (e.g., a = b or a = 5).
 */
public:
    std::string target_variable;
    std::string source_variable;
    bool is_reference;
    std::shared_ptr<ASTNode> expr;

    // @constructor
    // Initializes an assignment node with the specified target and source variables with or without reference.
    // @param _target The name of the target variable to which the value is assigned.
    // @param _source The name of the source variable or value being assigned.
    // @param _is_reference Indicates if the source is a reference to another variable (true) or a value (false).
    // @note If _expr is provided, it will be used as the source expression instead of a variable name.
    // @note If _expr is not provided, the source_variable will be used as the source value.
    AssignNode(const std::string& _target, const std::string& _source, bool _is_reference)
        : target_variable(_target), source_variable(_source), is_reference(_is_reference), expr(nullptr) {}

    // @constructor
    // Initializes an assignment node with the specified target and an expression without source.
    // @param _target The name of the target variable to which the value is assigned.
    // @param _expr The expression to be assigned to the target variable.
    AssignNode(const std::string& _target, std::shared_ptr<ASTNode> _expr)
        : target_variable(_target), source_variable(""), is_reference(false), expr(_expr) {}
};

class LogNode : public ASTNode {
/**
 * @class LogNode
 * @brief Node to represent a logging operation (log) in the AST.
 * @note This node can either represent a variable name to be logged or a literal value.
 */
public:
    std::shared_ptr<LiteralNode> value;
    std::string variable_name;
    bool is_variable;

    
    // @constructor
    // Initializes a log node with the specified variable name.
    // @param _var_name The name of the variable to be logged.
    LogNode(const std::string& _var_name) : value(nullptr), variable_name(_var_name), is_variable(true) {}
    
    // @costructor
    // Initializes a log node with the specified literal value.
    // @param _value A shared pointer to a LiteralNode representing the value to be logged
    LogNode(std::shared_ptr<LiteralNode> _value) : value(_value), is_variable(false) {}
};

class MultiOpNode : public ASTNode {
/**
 * @class MulitOpNode
 * @brief Represents a node for multiple operations in the AST (e.g., a + b + c).
 * @brief Contains a vector of operands and a vector of operators to represent multiple operations
 * @note The operands are stored as shared pointers to ASTNode, allowing for complex expressions.
 * @note The operators are stored as strings, allowing for different types of operations (e.g., "+", "-", "*", "/").
 * @note This node is used to represent expressions with multiple operands and operators, such as arithmetic expressions.
 */
public:
    std::vector<std::shared_ptr<ASTNode>> operands;
    std::vector<std::string> operators;

    // @constructor
    // Initializes a MultiOpNode with the specified operands and operators.
    // @param _operands A vector of shared pointers to ASTNode representing the operands in the expression.
    // @param _operators A vector of strings representing the operators to be applied between the operands.
    MultiOpNode(const std::vector<std::shared_ptr<ASTNode>>& _operands, const std::vector<std::string>& _operators)
        : operands(_operands), operators(_operators) {}
};

class MultiOpBoolNode : public ASTNode {
/**
 * @class MultiOpBoolNode
 * @brief Represents a node for multiple boolean operations in the AST (e.g., a && b || c).
 * @brief Contains a vector of operands and a vector of operators to represent multiple boolean operations.
 * @note The operands are stored as shared pointers to ASTNode, allowing for complex boolean expressions.
 * @note The operators are stored as strings, allowing for different types of boolean operations (e.g., "&&", "||", "!", "xor").
 * @note This node is used to represent boolean expressions with multiple operands and operators.
 */
public:
    std::vector<std::shared_ptr<ASTNode>> operands;
    std::vector<std::string> operators;

    // @constructor
    // Initializes a MultiOpBoolNode with the specified operands and operators.
    // @param _operands A vector of shared pointers to ASTNode representing the operands in the boolean expression.
    // @param _operators A vector of strings representing the boolean operators to be applied between the operands.
    // @note This node is used to represent boolean expressions with multiple operands and operators, such as logical expressions.
    MultiOpBoolNode(const std::vector<std::shared_ptr<ASTNode>>& _operands, const std::vector<std::string>& _operators)
        : operands(_operands), operators(_operators) {}
};

class ParseError : public std::runtime_error {
/**
 * @class ParseError
 * @brief Represents an error that occurs during parsing of the source code.
 * @brief Inherits from std::runtime_error and provides additional information about the error.
 * @note Contains the line and column where the error occurred, allowing for easier debugging.
 * @note This class is used to throw exceptions when the parser encounters unexpected tokens or syntax errors.
 * @note The error message is provided as a string when the ParseError is constructed.
 * @note The line and column are provided as integers to indicate the exact location of the error.
 */
public:
    int line;
    int column;

    // @constructor
    // Initializes a ParseError with the specified message, line, and column.
    // @param _message The error message describing the parsing error.
    // @param _line The line number where the error occurred.
    // @param _column The column number where the error occurred.
    ParseError(const std::string& _message, int _line, int _column) : std::runtime_error(_message), line(_line), column(_column) {}
};

#endif