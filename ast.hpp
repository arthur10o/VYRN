#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory>

class ASTNode {
/**
 * @brief Abstract base class for all nodes in the Abstract Syntax Tree (AST).
 * 
 * This class serves as the common interface for all types of nodes used in the
 * abstract syntax tree (AST) during parsing and semantic analysis.
 * 
 * It includes a virtual destructor to ensure proper cleanup of derived classes
 * when objects are deleted through base class pointers.
 * 
 * All concrete node types (e.g., ASTFunction, ASTAssign, etc.) should
 * inherit from this class.
 */
public:
    virtual ~ASTNode() = default;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

class ASTVarDecl : public ASTNode {
/**
 * @brief Represents a variable declaration node in the Abstract Syntax Tree (AST).
 * 
 * This node is used to declare a variable, optionally with an initialization expression.
 * It can represent both mutable ('var') and constant ('const') variables.
 * 
 * Components:
 * - 'name': the name of the variable being declared.
 * - 'isConst': whether the variable is declared as constant (true) or not (false).
 * - 'initExpr': the expression used to initialize the variable (can be a literal, variable, binary operation, etc.).
 */
public:
    std::string name;
    bool isConst;
    ASTNodePtr initExpr;
    /**
    * @brief Constructs a new ASTVarDecl node.
    * 
    * @param n The name of the variable.
    * @param c Whether the variable is constant (true = const, false = var).
    * @param expr The initialization expression (can be null).
    */
    ASTVarDecl(const std::string& n, bool c, ASTNodePtr expr) : name(n), isConst(c), initExpr(expr) {}
};

class ASTAssign : public ASTNode {
/**
 * @brief Represents an assignment operation node in the Abstract Syntax Tree (AST).
 * 
 * This node models the assignment of a value to a target variable or location.
 * 
 * Components:
 * - `target`: the left-hand side (LHS) of the assignment, typically an `ASTVariable` or a more complex target.
 * - `value`: the right-hand side (RHS) expression whose evaluated result is assigned to the target.
 */
public:
    ASTNodePtr target;
    ASTNodePtr value;
    /**
    * @brief Constructs a new ASTAssign node.
    * 
    * @param t The target node where the value is assigned.
    * @param v The expression node providing the value.
    */
    ASTAssign(ASTNodePtr t, ASTNodePtr v) : target(t), value(v) {}
};

class ASTLiteral : public ASTNode {
/**
 * @brief Represents a literal value node in the Abstract Syntax Tree (AST).
 * 
 * This node holds a constant literal value, such as a number, string, or boolean,
 * as it appears directly in the source code.
 * 
 * The literal value is stored as a string and can be later interpreted or converted
 * according to the expected type during semantic analysis or evaluation.
 */
public:
    std::string value;
    /**
    * @brief Constructs a new ASTLiteral node.
    * 
    * @param v The string representation of the literal value.
    */
    ASTLiteral(const std::string& v) : value(v) {}
};

class ASTVariable : public ASTNode {
/**
 * @brief Represents a variable node in the Abstract Syntax Tree (AST).
 * 
 * This node corresponds to the usage of a variable by its name
 * within expressions or statements.
 * 
 * It stores the variable's identifier as a string.
 */
public:
    std::string name;
    /**
     * @brief Constructs a new ASTVariable node.
     * 
     * @param n The name of the variable.
     */
    ASTVariable(const std::string& n) : name(n) {}
};

class ASTBinaryOp : public ASTNode {
/**
 * @brief Represents a binary operation node in the Abstract Syntax Tree (AST).
 * 
 * This node models an expression consisting of a binary operator
 * applied to two operands (left and right).
 * 
 * Examples of binary operators include arithmetic operators like
 * '+', '-', '*', '/', logical operators, comparison operators, etc.
 */
public:
    std::string op;
    ASTNodePtr left;
    ASTNodePtr right;
    /**
    * @brief Constructs a new ASTBinaryOp node.
    * 
    * @param o The operator string.
    * @param l Pointer to the left operand node.
    * @param r Pointer to the right operand node.
    */
    ASTBinaryOp(const std::string& o, ASTNodePtr l, ASTNodePtr r) : op(o), left(l), right(r) {}
};

#endif