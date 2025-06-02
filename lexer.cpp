#include "lexer.hpp"
#include <cctype>

/**
 * @brief Lexer constructor.
 * 
 * Initializes the lexical analyzer with a given source.
 * 
 * @param src The source code to be analyzed.
 */

Lexer::Lexer(const std::string& src) : source(src), pos(0), line(1), column(1) {}

bool Lexer::is_at_end() const {
    /**
    * @brief Checks whether the end of the source file has been reached.
    * 
    * @return true if analysis complete, false otherwise.
    */
    return pos >= source.size();
}

char Lexer::peek() const {
    /**
    * @brief Look at the current character without reading further.
    * 
    * @return The current character, or '\0' if the end of the source is reached.
    */
    if(is_at_end()) return '\0';
    return source[pos];
}

char Lexer::advance() {
    /**
    * @brief Advance one character in the source stream.
    * 
    * Also updates row and column numbers.
    * 
    * @return The character read before the feed.
    */
    char c = peek();
    pos++;
    if(c == '\n') {
        line ++;
        column = 1;
    } else {
        column ++;
    }
    return c;
}

void Lexer::skip_white_space() {
    /**
    * @brief Ignores white space and comments.
    * 
    * Skip all spacing characters (spaces, tabs, line breaks)
    * as well as comments on a line beginning with `//`.
    */
    while(!is_at_end()) {
        char c = peek();
        if(c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance();
        } else if(c == '/' && pos + 1 < source.size() && source[pos + 1] == '/') {
            while (!is_at_end() && peek() != '\n') advance();
        } else {
            break;
        }
    }
}

bool Lexer::isalpha(char c) const {
    /**
    * @brief Checks whether a character is alphabetic or an underscore.
    *
    * This function is used to identify valid characters.
    * in an identifier (variable name, function, etc.).
    * Returns true if the character is an upper or lower case letter.
    * according to locale C, or an underscore ('_').
    *
    * @param c The character to be tested.
    * @return true if c is a letter (A-Z, a-z) or '_', false otherwise.
    */
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool Lexer::isdigit(char c) const {
    /**
    * @brief Checks if a character is a digit (0–9).
    *
    * This function determines whether the given character represents a numeric digit.
    * It is typically used to recognize the beginning or continuation of numeric literals
    * during lexical analysis.
    *
    * @param c The character to check.
    * @return true if the character is a digit (i.e., '0' to '9'), false otherwise.
    */
    return std::isdigit(static_cast<unsigned char>(c));
}

bool Lexer::is_alpha_numeric(char c) const {
    /**
    * @brief Checks if a character is alphanumeric or an underscore.
    *
    * This function returns true if the character is either:
    * - an alphabetic letter (A–Z, a–z),
    * - an underscore ('_'),
    * - or a digit (0–9).
    *
    * It is used to determine if a character can be part of an identifier
    * (e.g., variable names, function names) during lexical analysis.
    *
    * Internally, it calls Lexer::isalpha and Lexer::isdigit to perform the check.
    *
    * @param c The character to evaluate.
    * @return true if the character is a letter, digit, or underscore; false otherwise.
    */
    return isalpha(c) || isdigit(c);
}

bool Lexer::is_keyword(const std::string &text) const {
    /**
    * @brief Checks if a given string is a reserved keyword.
    *
    * This function compares the input string against the list of reserved keywords
    * defined in the lexer. It returns true if the string matches any keyword,
    * indicating that the token should be classified as a keyword rather than an identifier.
    *
    * @param text The string to check.
    * @return true if the string is a reserved keyword; false otherwise.
    */
    for(auto& kw : keywords) {
        if(kw == text) return true;
    }
    return false;
}

Token Lexer::make_token(TokenType type, const std::string& lexeme) {
    /**
    * @brief Creates and returns a Token object with the specified type and lexeme.
    *
    * This function constructs a Token representing a lexical unit found in the source code.
    * It sets the token's type and lexeme (the exact substring matched),
    * and records the line and column position where the token begins.
    * The column position is adjusted by subtracting the length of the lexeme
    * because the current column points to the character immediately after the token.
    *
    * @param type The TokenType enum value representing the token's type.
    * @param lexeme The string content of the token (the matched substring).
    * @return A fully initialized Token object with position information.
    */
    return Token{type, lexeme, line, column - (int)lexeme.size()};
}

Token Lexer::identifier() {
    /**
    * @brief Extracts an identifier or keyword token from the source code.
    *
    * This function reads characters starting from the current position while they
    * are alphanumeric (letters, digits, or underscore). It collects the substring
    * that forms the identifier.
    *
    * After extracting the text, it checks if the string matches any reserved keyword.
    * If it is a keyword, the function returns a token of type `KEYWORD`,
    * otherwise, it returns a token of type `IDENTIFIER`.
    *
    * @return A Token representing either an identifier or a keyword, including
    *         its lexeme and position information.
    */
    size_t start = pos;
    while(!is_at_end() && is_alpha_numeric(peek())) advance();

    std::string text = source.substr(start, pos - start);
    if(is_keyword(text)) {
        return make_token(TokenType::KEYWORD, text);
    }
    return make_token(TokenType::IDENTIFIER, text);
}

Token Lexer::number() {
    /**
    * @brief Extracts a numeric token (integer, floating-point, or exponent) from the source code.
    *
    * This function reads consecutive digit characters to form an integer part.
    * If a decimal separator ('.' or ',') is found, it consumes it and continues
    * reading digits to form the fractional part.
    *
    * The extracted substring represents a number literal with optional exponent.
    *
    * @return A Token of type NUMBER containing the lexeme and position information.
    */
    size_t start = pos;
    while(!is_at_end() && isdigit(peek())) advance();

    if(!is_at_end() && (peek() == '.' || peek() == ',')) {
        advance();
        while(!is_at_end() && isdigit(peek())) advance();
    }

    if(!is_at_end() && (peek() == 'e' || peek() == 'E' || peek() == '^')) {
        advance();

        if(!is_at_end() && (peek() == '+' || peek() == '-')) {
            advance();
        }

        while(!is_at_end() && isdigit(peek())) {
            advance();
        }
    }

    std::string text = source.substr(start, pos - start);
    return make_token(TokenType::NUMBER, text);
}

Token Lexer::string_literal(char delimiter) {
    /**
    * @brief Extracts a string literal token from the source code.
    *
    * This function assumes the current character is the opening delimiter (e.g., '"' or '\'').
    * It consumes the opening delimiter, then reads characters until it finds the matching
    * closing delimiter or reaches the end of the source.
     * The content between the delimiters is extracted as the string literal value.
    *
    * Note: Escape sequences inside the string are not handled in this implementation.
    *
    * @param delimiter The delimiter used for the string literal (either '\"' or '\'').
    * @return A Token of type STRING containing the extracted string literal without delimiters.
    */
    advance();
    size_t start = pos;
    while(!is_at_end() && peek() != delimiter) advance();
    std::string text = source.substr(start, pos - start);
    if(!is_at_end()) advance();
    return make_token(TokenType::STRING, text);
}

Token Lexer::unknown() {
    /**
    * @brief Creates an UNKNOWN token for an unrecognized character.
    * 
    * This function consumes (advances past) the current character in the source,
    * then creates a token of type UNKNOWN containing that character as a single-character string.
    * 
    * @return Token A token representing an unknown or invalid character.
    */
    char c = advance();
    return make_token(TokenType::UNKNOWN, std::string(1, c));
}

std::vector<Token> Lexer::tokenize() {
    /**
    * @brief Tokenizes the entire source code.
    * 
    * This function reads through the source code character by character,
    * skipping whitespace and comments, and classifying character sequences
    * into meaningful tokens such as identifiers, numbers, string literals,
    * operators, and separators.
    * 
    * The recognized token types include:
    * - **Identifiers** (e.g., variable or function names)
    * - **Numbers** (integer or floating-point)
    * - **String literals** (quoted text)
    * - **Operators** (`+`, `-`, `==`, `!=`, `&&`, `||`, etc.)
    * - **Separators** (`(`, `)`, `{`, `}`, `,`, `;`, etc.)
    * 
    * If an unrecognized character is encountered, an UNKNOWN token is produced.
    * At the end of the process, an END_OF_FILE token is appended.
    * 
    * @return std::vector<Token> The complete list of tokens extracted from the source.
    */
    std::vector<Token> tokens;
    while(!is_at_end()) {
        skip_white_space();
        if(is_at_end()) break;

        char c = peek();

        if(isalpha(c)) {
            tokens.push_back(identifier());
        } else if(isdigit(c)) {
            tokens.push_back(number());
        } else if(c == '"' || c == '\'') {
            tokens.push_back(string_literal(c));
        } else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
                   c == '=' || c == '<' || c == '>' || c == '!' || c == '&' || c == '|') {
            std::string op;
            op += advance();
            if(!is_at_end()) {
                char next = peek();
                if ((op == "=" && next == '=') || (op == "!" && next == '=') || (op == "&" && next == '&') ||
                    (op == "|" && next == '|') || (op == "<" && next == '=') || (op == ">" && next == '=')) {
                    op += advance();
                }
            }
            tokens.push_back(make_token(TokenType::OPERATOR, op));
        } else if (c == '(' || c == ')' || c == '{' || c == '}' ||
                   c == ',' || c == ';' || c == ':' || c == '.') {
            std::string sep;
            sep += advance();
            tokens.push_back(make_token(TokenType::SEPARATOR, sep));
        } else {
            tokens.push_back(unknown());
        }
    }
    tokens.push_back(Token{TokenType::END_OF_FILE,"", line, column});
    return tokens;
}