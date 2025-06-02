#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

enum class TokenType {
    IDENTIFIER,
    NUMBER,
    STRING,
    KEYWORD,
    OPERATOR,
    SEPARATOR,
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

class Lexer {
    public:
        Lexer(const std::string &source);
        std::vector<Token> tokenize();
        
    private:
        const std::string &source;
        size_t pos = 0;
        int line = 1;
        int column = 1;
        
        char peek() const;
        char advance();
        bool is_at_end() const;
        void skip_white_space();
        
        Token make_token(TokenType type, const std::string &lexeme);
        Token identifier();
        Token number();
        Token string_literal(char delimiter);
        Token unknown();

        bool isalpha(char c) const;
        bool isdigit(char c) const;
        bool is_alpha_numeric(char c) const;

        const std::vector<std::string> keywords {
            "let",
            "var",
            "bool",
            "true",
            "false",
            "const",
            "null",
            "class",
            "self",
            "func",
            "return",
            "if",
            "else if",
            "elif",
            "else",
            "for",
            "while",
            "match",
            "case",
            "break",
            "continue"
        };

        bool is_keyword(const std::string &text) const;
};

#endif