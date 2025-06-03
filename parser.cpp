#include "parser.hpp"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

const Token& Parser::peek() const {
    if(pos >= tokens.size()) return tokens.back();
    return tokens[pos];
}

const Token& Parser::advance() {
    if(!is_at_end()) pos++;
    return tokens[pos - 1];
}

bool Parser::match(TokenType type, const std::string& lexeme) {
    if(is_at_end()) return false;
    const Token& token = peek();
    if(token.type != type) return false;
    if(!lexeme.empty() && token.lexeme != lexeme) return false;
    advance();
    return true;
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::END_OF_FILE;
}

std::shared_ptr<ASTNode> Parser::parse() {
    std::vector<std::shared_ptr<ASTNode>> declarations;
    while(!is_at_end()) {
        auto decl = declaration();
        if(decl) declarations.push_back(decl);
        else break;
    }
    return std::make_shared<ASTProgram>(declarations);
}

std::shared_ptr<ASTNode> Parser::declaration() {
    if (match(TokenType::KEYWORD, "class")) return class_declaration();
    if (match(TokenType::KEYWORD, "func")) return function_declaration();
    if (match(TokenType::KEYWORD, "let") || match(TokenType::KEYWORD, "const"))
        return variable_declaration();

    return statement();
}

std::shared_ptr<ASTNode> Parser::class_declaration() {
    if(!match(TokenType::IDENTIFIER)) {
        std::cerr << "Erreur : nom de classe attendu" << std::endl;
        return nullptr;
    }

    std::string className = tokens[pos - 1].lexeme;

    if(!match(TokenType::SEPARATOR, "{")) {
        std::cerr << "Erreur : { attendu après nom de classe" << std::endl;
        return nullptr;
    }

    std::vector<std::shared_ptr<ASTNode>> members;
    while(!match(TokenType::SEPARATOR, "}")) {
        auto member = declaration();
        if(!member) return nullptr;
        members.push_back(member);
    }

    return std::make_shared<ASTClass>(className, members);
}

std::shared_ptr<ASTNode> Parser::function_declaration() {
    if(!match(TokenType::IDENTIFIER)) {
        std::cerr << "Erreur : nom de fonction attendu" << std::endl;
        return nullptr;
    }

    std::string funcName = tokens[pos - 1].lexeme;

    if(!match(TokenType::SEPARATOR, "(")) {
        std::cerr << "Erreur : '(' attendu après nom de fonction" << std::endl;
        return nullptr;
    }

    std::vector<std::string> params;
    if(!match(TokenType::SEPARATOR, ")")) {
        do {
            if(!match(TokenType::IDENTIFIER)) {
                std::cerr << "Erreur : nom de paramètre attendu" << std::endl;
                return nullptr;
            }
            params.push_back(tokens[pos - 1].lexeme);
        } while (match(TokenType::SEPARATOR, ","));
        if(!match(TokenType::SEPARATOR, ")")) {
            std::cerr << "Erreur : ')' attendu après paramètres" << std::endl;
            return nullptr;
        }

    }
    if(!match(TokenType::SEPARATOR, "{")){
        std::cerr << "Erreur : '{' attendu après déclaration de fonction" << std::endl;
        return nullptr;
    }

    std::vector<std::shared_ptr<ASTNode>> body;
    while(!match(TokenType::SEPARATOR, "}")) {
        auto stmt = statement();
        if(!stmt) return nullptr;
        body.push_back(stmt);
    }
    return std::make_shared<ASTFunction>(funcName, params, body);
}

std::shared_ptr<ASTNode> Parser::variable_declaration() {
    bool is_const = tokens[pos - 1].lexeme == "const";

    if(!match(TokenType::IDENTIFIER)) {
        std::cerr << "Erreur : nom de variable attendu" << std::endl;
        return nullptr;
    }
    std::string varName = tokens[pos - 1].lexeme;

    if(!match(TokenType::OPERATOR, "=")) {
        std::cerr << "Erreur : '=' attendu après nom de variable" << std::endl;
        return nullptr;
    }

    auto initExpr = expression();

    if(!match(TokenType::SEPARATOR, ";")) {
        std::cerr << "Erreur : ';' attendu après déclaration" << std::endl;
        return nullptr;
    }

    return std::make_shared<ASTVarDecl>(varName, is_const, initExpr);
}

std::shared_ptr<ASTNode> Parser::statement() {
    if(match(TokenType::KEYWORD, "return")) {
        auto expr = expression();
        if(!match(TokenType::SEPARATOR, ";")) {
            std::cerr << "Erreur : ';' attendu après return" << std::endl;
            return nullptr;
        }
        return std::make_shared<ASTReturn>(expr);
    }
    if(match(TokenType::KEYWORD, "if")) {
        if(!match(TokenType::SEPARATOR, "(")) {
            std::cerr << "Erreur : '(' attendu après if" << std::endl;
            return nullptr;
        }
        auto cond = expression();
        if(!match(TokenType::SEPARATOR, ")")) {
            std::cerr << "Erreur : ')' attendu après condition" << std::endl;
            return nullptr;
        }
        if(!match(TokenType::SEPARATOR, "{")) {
            std::cerr << "Erreur : '{' attendu après if" << std::endl;
            return nullptr;
        }
        std::vector<std::shared_ptr<ASTNode>> thenBranch;
        while(!match(TokenType::SEPARATOR, "}")) {
            auto stmt = statement();
            if(!stmt) return nullptr;
            thenBranch.push_back(stmt);
        }

        std::vector<std::shared_ptr<ASTNode>> elseBranch;
        if(match(TokenType::KEYWORD, "else")) {
            if(!match(TokenType::SEPARATOR, "{")) {
                std::cerr << "Erreur : '{' attendu après else" << std::endl;
                return nullptr;
            }
            while(!match(TokenType::SEPARATOR, "}")) {
                auto stmt = statement();
                if(!stmt) return nullptr;
                elseBranch.push_back(stmt);
            }
        }
        return std::make_shared<ASTIf>(cond, thenBranch, elseBranch);
    }

    auto expr = expression();
    if(!match(TokenType::SEPARATOR, ";")) {
        std::cerr << "Erreur : ';' attendu après expression" << std::endl;
        return nullptr;
    }

    return expr;
}

std::shared_ptr<ASTNode> Parser::expression() {
    return assignement();
}

std::shared_ptr<ASTNode> Parser::assignement() {
    auto expr = logic_or();
    if(match(TokenType::OPERATOR, "=")) {
        auto value = assignement();
        if (auto var = std::dynamic_pointer_cast<ASTVariable>(expr)) {
            return std::make_shared<ASTAssign>(var, value);
        } else {
            std::cerr << "Erreur : affectation invalide" << std::endl;
            return nullptr;
        }
    }
    return expr;
}

std::shared_ptr<ASTNode> Parser::logic_or() {
    auto expr = logic_and();
    
    while(match(TokenType::OPERATOR, "||")) {
        auto right = logic_and();
        expr = std::make_shared<ASTBinaryOp>("||", expr, right);
    }
    return expr;
}

std::shared_ptr<ASTNode> Parser::logic_and() {
    auto expr = equality();
    while(match(TokenType::OPERATOR, "&&")) {
        auto right = equality();
        expr = std::make_shared<ASTBinaryOp>("&&", expr, right);
    }
    return expr;
}

std::shared_ptr<ASTNode> Parser::equality() {
    auto expr = comparison();

    while(true) {
        if(match(TokenType::OPERATOR, "==")) {
            auto right = comparison();
            expr = std::make_shared<ASTBinaryOp>("==", expr, right);
        } else if(match(TokenType::OPERATOR, "!=")) {
            auto right = comparison();
            expr = std::make_shared<ASTBinaryOp>("!=", expr, right);
        } else {
            break;
        }
    }
    return expr;
}

std::shared_ptr<ASTNode> Parser::comparison() {
    auto expr = term();

    while (true)
    {
        if(match(TokenType::OPERATOR, "<")) {
            auto right = term();
            expr = std::make_shared<ASTBinaryOp>("<", expr, right);
        } else if(match(TokenType::OPERATOR, ">")) {
            auto right = term();
            expr = std::make_shared<ASTBinaryOp>(">", expr, right);
        } else if(match(TokenType::OPERATOR, "<=")) {
            auto right = term();
            expr = std::make_shared<ASTBinaryOp>("<=", expr, right);
        } else if(match(TokenType::OPERATOR, ">=")) {
            auto right = term();
            expr = std::make_shared<ASTBinaryOp>(">=", expr, right);
        } else {
            break;
        }
    }
    return expr;
}

std::shared_ptr<ASTNode> Parser::term() {
    auto expr = factor();

    while(true) {
        if(match(TokenType::OPERATOR, "+")) {
            auto right = factor();
            expr = std::make_shared<ASTBinaryOp>("+", expr, right);
        } else if(match(TokenType::OPERATOR, "-")) {
            auto right = factor();
            expr = std::make_shared<ASTBinaryOp>("-", expr, right);
        } else {
            break;
        }
    }
    return expr;
}

std::shared_ptr<ASTNode> Parser::factor() {
    auto expr = unary();

    while(true) {
        if(match(TokenType::OPERATOR, "*")) {
            auto right  = unary();
            expr = std::make_shared<ASTBinaryOp>("*", expr, right);
        } else if(match(TokenType::OPERATOR, "/")) {
            auto right  = unary();
            expr = std::make_shared<ASTBinaryOp>("/", expr, right);
        } else if(match(TokenType::OPERATOR, "%")) {
            auto right  = unary();
            expr = std::make_shared<ASTBinaryOp>("%", expr, right);
        } else {
            break;
        }
    }
    return expr;
}

std::shared_ptr<ASTNode> Parser::unary() {
    if(match(TokenType::OPERATOR, "!")) {
        auto right = unary();
        return std::make_shared<ASTUnaryOp>("!", right);
    } else if(match(TokenType::OPERATOR, "-")) {
        auto right = unary();
        return std::make_shared<ASTUnaryOp>("-", right);
    }
    return primary();
}

std::shared_ptr<ASTNode> Parser::primary() {
    if(match(TokenType::NUMBER)) {
        return std::make_shared<ASTLiteral>(tokens[pos - 1].lexeme);
    }
    if(match(TokenType::STRING)) {
        return std::make_shared<ASTLiteral>(tokens[pos - 1].lexeme);
    }
    if(match(TokenType::KEYWORD, "true")) {
        return std::make_shared<ASTLiteral>("true");
    }
    if(match(TokenType::KEYWORD, "false")) {
        return std::make_shared<ASTLiteral>("false");
    }
    if(match(TokenType::IDENTIFIER)) {
        return std::make_shared<ASTVariable>(tokens[pos - 1].lexeme);
    }
    if(match(TokenType::SEPARATOR, "(")) {
        auto expr = expression();
        if(!match(TokenType::SEPARATOR, ")")) {
            std::cerr << "Erreur : ')' attendu" << std::endl;
            return nullptr;
        }
        return expr;
    }
    std::cerr << "Erreur : expression inattendue" << std::endl;
    return nullptr;
}