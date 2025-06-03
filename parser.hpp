#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <memory>
#include "lexer.hpp"
#include "ast.hpp"

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::shared_ptr<ASTNode> parse();

private:
    const std::vector<Token>& tokens;
    size_t pos;

    const Token& peek() const;
    const Token& advance();
    bool match(TokenType type, const std::string& lexeme = "");
    bool is_at_end() const;

    std::shared_ptr<ASTNode> declaration();
    std::shared_ptr<ASTNode> statement();
    std::shared_ptr<ASTNode> expression();

    std::shared_ptr<ASTNode> class_declaration();
    std::shared_ptr<ASTNode> function_declaration();
    std::shared_ptr<ASTNode> variable_declaration();

    std::shared_ptr<ASTNode> assignement();
    std::shared_ptr<ASTNode> logic_or();
    std::shared_ptr<ASTNode> logic_and();
    std::shared_ptr<ASTNode> equality();
    std::shared_ptr<ASTNode> comparison();
    std::shared_ptr<ASTNode> term();
    std::shared_ptr<ASTNode> factor();
    std::shared_ptr<ASTNode> unary();
    std::shared_ptr<ASTNode> primary();
};

#endif