#ifndef LEXER_HPP
#define LEXER_HPP

#include "ast.hpp"

#include <unordered_set>

/**
 * @file lexer.hpp
 * @brief Define the lexer to analyze the source code and create tokens.
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

class Lexer {
    /**
     * @class Lexer
     * @brief Analyzes lexical of a chain of characters source to produce tokens.
     *
     * The lexer reads source source code character by character and identifies tokens,
     * while managing comments, whitespace, keywords, types, operators, etc.
     */
    const std::string& input;   /**< Reference to the source code to analyse */
    size_t pos = 0;             /**< Current position in the source code */
    int line = 1;               /**< Current line number */
    int column = 1;             /**< Current column number */

    void skip_white_space() {
        /**
         * @brief Skip whitespace characters and comments in the source code.
         *
         * Updates the line and column counters.
         */
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
                    // Single-line comment
                    while (pos < input.size() && input[pos] != '\n') {
                        pos++;
                    }
                } else if (input[pos + 1] == '*') {
                    // Multi-line comment
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
                        pos += 2;   // End of multi-line comment //
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
        /**
         * @brief Look at the current character without advancing the position.
         * @return Current character or '\0' if at the end of the source code.
         */
        return pos < input.size() ? input[pos] : '\0';
    }

    char advance() {
        /**
         * @brief Advance to the next character in the source code and update line and column counters.
         * @return Advanced character.
         */
        if (pos >= input.size()) return '\0';
        char character_to_analyse = input[pos++];
        if (character_to_analyse == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        return character_to_analyse;
    }

    bool is_identifier_character(char _character_to_identified) {
        /**
         * @brief Check if a character is valid for an identifier (letter, digit, or underscore).
         * @param _character_to_identified Character to check.
         * @return True if the character is valid or false otherwise.
         */
        return std::isalnum(_character_to_identified) || _character_to_identified == '_';
    }

public:
    /**
     * @brief Constructor for the lexer.
     * @param _input Reference to the source code to analyze.
     */
    Lexer(const std::string& _input) : input(_input) {}

    Token next_token() {
        /**
         * @brief Get the next token from the source code.
         *
         * This method ignores whitespace and comments, then identifies and returns the next token (keyword, identifier, digit, operator, etc.).
         * 
         * @return Next token with its type, value, line, and column.
         */
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

        // Checking three-character Boolean operators
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

        // Checking two-character Boolean operators
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

        // Identifier, keyword, type, boolean, or boolean operator
        if (std::isalpha(character_to_analyse) || character_to_analyse == '_') {
            size_t start = pos;
            while(pos < input.size() && is_identifier_character(input[pos])) advance();
            std::string word = input.substr(start, pos - start);

            if (keywords.find(word) != keywords.end()) {
                return {TokenType::Keyword, word, tok_line, tok_column};
            } else if (types.find(word) != types.end()) {
                return {TokenType::Type, word, tok_line, tok_column};
            } else if (word == "true" || word == "false") {
                return {TokenType::Bool, word, tok_line, tok_column};
            } else if (boolean_operator.find(word) != boolean_operator.end()) {
                return {TokenType::BooleanOperator, word, tok_line, tok_column};
            } else {
                return {TokenType::Identifier, word, tok_line, tok_column};
            }
        }

        // String in double quotes
        if (character_to_analyse == '"') {
            advance();
            size_t start = pos;
            while (pos < input.size() && peek() != '"') advance();
            std::string string = input.substr(start, pos - start);
            advance();
            return {TokenType::String, string, tok_line, tok_column};
        }

        // Number (integer of floating point)
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

        // Operators '<' or '>'
        if (character_to_analyse == '<' || character_to_analyse == '>') {
            pos++;
            column++;
            return {TokenType::BooleanOperator, std::string(1, character_to_analyse), tok_line, tok_column};
        }

        // Operator '!' (logical NOT)
        if (character_to_analyse == '!') {
            pos++;
            column++;
            return {TokenType::BooleanOperator, "!", tok_line, tok_column};
        }

        // All other symbols considered as symbols
        advance();
        return {TokenType::Symbol, std::string(1, character_to_analyse), tok_line, tok_column};
    }
};
#endif