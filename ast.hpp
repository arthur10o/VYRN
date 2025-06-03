#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <sstream>

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

enum class ValueType {
    INT,
    DOUBLE,
    BOOL,
    STRING
};

class Value {
/**
 * @brief Represents a dynamically-typed value used during evaluation.
 * 
 * The Value class is used to store literal values (int, double, bool, or string)
 * in a unified structure that can be passed around during interpretation.
 * 
 * It supports implicit construction from C++ primitive types and includes accessors
 * that perform type checking before returning the stored value.
 * 
 * Internally, it uses a tagged union to store exactly one of the supported types.
 * 
 * Components:
 * - 'type': indicates which type is currently stored (INT, DOUBLE, BOOL, STRING).
 * - 'intVal': stores an integer value (if type is INT).
 * - 'doubleVal': stores a floating-point value (if type is DOUBLE).
 * - 'boolVal': stores a boolean value (if type is BOOL).
 * - 'stringVal': stores a string value (if type is STRING).
 */
public:
    ValueType type;
    union {
        int intVal;
        double doubleVal;
        bool boolVal;
    };
    std::string stringVal;

    Value() : type(ValueType::INT), intVal(0) {}
    Value(int v) : type(ValueType::INT), intVal(v) {}
    Value(double v) : type(ValueType::DOUBLE), doubleVal(v) {}
    Value(bool v) : type(ValueType::BOOL), boolVal(v) {}
    Value(const std::string& v) : type(ValueType::STRING), stringVal(v) {}

    int asInt() const {
        /**
         * @brief Returns the stored value as an integer.
         * @throws std::runtime_error if the stored type is not INT.
         */
        if (type == ValueType::INT) return intVal;
        throw std::runtime_error("Value is not an int");
    }

    double asDouble() const {
        /**
         * @brief Returns the stored value as a double.
         * @throws std::runtime_error if the stored type is not DOUBLE.
         */
        if (type == ValueType::DOUBLE) return doubleVal;
        throw std::runtime_error("Value is not a double");
    }

    bool asBool() const {
        /**
         * @brief Returns the stored value as a boolean.
         * @throws std::runtime_error if the stored type is not BOOL.
         */
        if (type == ValueType::BOOL) return boolVal;
        throw std::runtime_error("Value is not a bool");
    }

    std::string asString() const {
        /**
         * @brief Returns the stored value as a string.
         * @throws std::runtime_error if the stored type is not STRING.
         */
        if (type == ValueType::STRING) return stringVal;
        throw std::runtime_error("Value is not a string");
    }
};

class ASTProgram : public ASTNode {
/**
 * @brief Represents the root node of an entire program.
 * 
 * This node aggregates a list of top-level declarations or statements
 * that compose a complete source file.
 * 
 * Components:
 * - 'statements': a list of AST nodes representing functions, classes, or global statements.
 */
public:
    std::vector<ASTNodePtr> statements;

    ASTProgram(const std::vector<ASTNodePtr>& stmts) : statements(stmts) {}
};

class ASTFunction : public ASTNode {
/**
 * @brief Represents a function declaration in the AST.
 * 
 * This node defines a named function, including its parameter list and function body.
 * 
 * Components:
 * - 'name': the name of the function.
 * - 'params': a list of parameter names.
 * - 'body': a list of statements that make up the function body.
 */
public:
    std::string name;
    std::vector<std::string> params;
    std::vector<ASTNodePtr> body;

    ASTFunction(const std::string& n, const std::vector<std::string>& p, const std::vector<ASTNodePtr>& b)
        : name(n), params(p), body(b) {}
};

class ASTReturn : public ASTNode {
/**
 * @brief Represents a return statement in the AST.
 * 
 * This node is used to return a value from within a function.
 * 
 * Components:
 * - 'expr': the expression to be evaluated and returned.
 */
public:
    ASTNodePtr expr;

    ASTReturn(ASTNodePtr e) : expr(e) {}
};

class ASTIf : public ASTNode {
/**
 * @brief Represents an if-else conditional structure in the AST.
 * 
 * This node models a conditional branch with an optional else block.
 * 
 * Components:
 * - 'condition': the condition expression to be evaluated.
 * - 'thenBranch': a list of statements executed if the condition is true.
 * - 'elseBranch': a list of statements executed if the condition is false (optional).
 */
public:
    ASTNodePtr condition;
    std::vector<ASTNodePtr> thenBranch;
    std::vector<ASTNodePtr> elseBranch;

    ASTIf(ASTNodePtr cond, const std::vector<ASTNodePtr>& thenBr, const std::vector<ASTNodePtr>& elseBr)
        : condition(cond), thenBranch(thenBr), elseBranch(elseBr) {}
};

class ASTClass : public ASTNode {
/**
 * @brief Represents a class declaration in the AST.
 * 
 * This node defines a named class, containing member declarations such as methods or fields.
 * 
 * Components:
 * - 'name': the name of the class.
 * - 'members': a list of class members (functions, fields, etc.).
 */
public:
    std::string name;
    std::vector<ASTNodePtr> members;

    ASTClass(const std::string& n, const std::vector<ASTNodePtr>& m) : name(n), members(m) {}
};

class ASTUnaryOp : public ASTNode {
/**
 * @brief Represents a unary operation node in the AST.
 * 
 * This node models expressions with a unary operator, such as negation (-) or logical not (!).
 * 
 * Components:
 * - 'op': the unary operator (e.g., "-", "!").
 * - 'operand': the operand expression to which the operator is applied.
 */
public:
    std::string op;
    ASTNodePtr operand;

    ASTUnaryOp(const std::string& o, ASTNodePtr expr) : op(o), operand(expr) {}
};

struct VariableInfo {
/**
 * @brief Represents information about a variable during interpretation.
 * 
 * This struct stores the value of a variable and whether it is constant.
 * Used by the interpreter's environment to track variable state.
 * 
 * Components:
 * - 'value': the current value of the variable.
 * - 'isConst': whether the variable was declared as constant.
 */
    Value value;
    bool isConst;

    VariableInfo(const Value& v, bool c) : value(v), isConst(c) {}
};

using Environment = std::unordered_map<std::string, VariableInfo>;

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
 * - 'target': the left-hand side (LHS) of the assignment, typically an 'ASTVariable' or a more complex target.
 * - 'value': the right-hand side (RHS) expression whose evaluated result is assigned to the target.
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

inline Value eval(ASTNodePtr node, Environment& env) {
    /**
     * @brief Recursively evaluates a node in the Abstract Syntax Tree (AST).
     * 
     * This function interprets a given AST node and returns its computed value.
     * It supports literal values, variables, binary operations, assignments, and declarations.
     * 
     * The function uses an 'Environment' to track variable values and constness.
     * Each variable is stored as a 'VariableInfo' object, containing its value and a flag indicating
     * whether it is declared as 'const'.
     * 
     * @param node A pointer to the AST node to evaluate.
     * @param env A reference to the current environment of variables.
     * @return Value The evaluated result of the node.
     * 
     * @throws std::runtime_error If a variable is undefined, a constant is modified, or
     *         an unsupported or mistyped operation is encountered.
     */
    if (auto lit = std::dynamic_pointer_cast<ASTLiteral>(node)) {
        const std::string& val = lit->value;
        if (val == "true") return Value(true);
        if (val == "false") return Value(false);

        try {
            size_t idx;
            int ival = std::stoi(val, &idx);
            if (idx == val.size()) return Value(ival);
        } catch (...) {}

        try {
            size_t idx;
            double dval = std::stod(val, &idx);
            if (idx == val.size()) return Value(dval);
        } catch (...) {}

        return Value(val);
    }

    else if (auto var = std::dynamic_pointer_cast<ASTVariable>(node)) {
        auto it = env.find(var->name);
        if (it == env.end()) throw std::runtime_error("Variable not defined: " + var->name);
        return it->second.value;
    }

    else if (auto binop = std::dynamic_pointer_cast<ASTBinaryOp>(node)) {
        Value left = eval(binop->left, env);
        Value right = eval(binop->right, env);

        if (left.type == ValueType::INT && right.type == ValueType::INT) {
            int li = left.asInt();
            int ri = right.asInt();

            if (binop->op == "+") return Value(li + ri);
            if (binop->op == "-") return Value(li - ri);
            if (binop->op == "*") return Value(li * ri);
            if (binop->op == "/") {
                if (ri == 0) throw std::runtime_error("Division by zero");
                return Value(li / ri);
            }
        }

        throw std::runtime_error("Unsupported binary operation or type mismatch");
    }

    else if (auto assign = std::dynamic_pointer_cast<ASTAssign>(node)) {
        auto varNode = std::dynamic_pointer_cast<ASTVariable>(assign->target);
        if (!varNode) throw std::runtime_error("Assignment target must be a variable");

        auto it = env.find(varNode->name);
        if (it == env.end()) throw std::runtime_error("Variable not defined: " + varNode->name);
        if (it->second.isConst) throw std::runtime_error("Cannot assign to constant variable: " + varNode->name);

        Value val = eval(assign->value, env);
        it->second.value = val;
        return val;
    }

    else if (auto decl = std::dynamic_pointer_cast<ASTVarDecl>(node)) {
        Value val = Value(0);
        if (decl->initExpr) {
            val = eval(decl->initExpr, env);
        }

        if (env.find(decl->name) != env.end()) {
            throw std::runtime_error("Variable already declared: " + decl->name);
        }

        env[decl->name] = VariableInfo(val, decl->isConst);
        return val;
    }

    throw std::runtime_error("Unknown AST node type");
}

#endif