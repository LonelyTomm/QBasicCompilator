#ifndef LEXER_H_
#define LEXER_H_

enum TokenType {
    QUOTED_STRING,
    UNQUOTED_STRING,
    INTEGER,
    FLOAT,
    NEW_LINE,

    COMMA,
    SEMICOLON,

    ASSIGN_OPERATOR,

    EQUALS,
    GREATER_THAN,
    LESSER_THAN,

    OPEN_ROUND_BRACKET,
    CLOSE_ROUND_BRACKET,
};

struct Token {
    enum TokenType token_type;
    int row;
    int col;
    char* value;
};

struct TokenList {
    long length;
    long capacity;
    struct Token* tokens;
};

struct TokenList read_tokens(char* file_buff, long fsize);

const char* get_token_type_string(enum TokenType token_type);

#endif