#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static struct TokenList new_token_list(void);
static void add_token(struct TokenList* tokens, struct Token token);

struct StringReallocator {
    char* string;
    int capacity;
    int length;
};

static void add_char(struct StringReallocator* reallocator, char ch);
static struct StringReallocator new_string_reallocator(void);

struct CharPeeker {
    char* file_buff;
    long file_size;
    long current_pos;
    int row;
    int col;
};

static struct CharPeeker new_char_peeker(char* file_buff, long file_size);
static char* next_char(struct CharPeeker* peeker);
static char* peek_char(struct CharPeeker* peeker);

static long read_special_char(struct CharPeeker* peeker, struct Token* token);
static long read_number_token(struct CharPeeker* peeker, struct Token* token);
static long read_new_line_token(struct CharPeeker* peeker, struct Token* token);
static long read_quoted_string_token(struct CharPeeker* peeker, struct Token* token);
static long read_unquoted_string_token(struct CharPeeker* peeker, struct Token* token);
static long skip_whitespaces(struct CharPeeker* peeker);
static void string_to_lowercase(char * string);

static struct TokenList new_token_list(void) {
    struct TokenList token_list;
    token_list.length = 0;
    token_list.capacity = 0;
    token_list.tokens = NULL;

    return token_list;
}

static void add_token(struct TokenList* tokens, struct Token token) {
    int alloc_size = 20;
    if (tokens->capacity == 0) {
        tokens->tokens = (struct Token*)malloc(alloc_size * sizeof(struct Token));
        tokens->capacity += alloc_size;
    }

    if (tokens->length >= tokens->capacity) {
        tokens->tokens = (struct Token*)realloc(tokens->tokens, (tokens->capacity + alloc_size) * sizeof(struct Token));
        tokens->capacity += alloc_size;
    }

    tokens->tokens[tokens->length] = token;
    tokens->length++;
}

static struct StringReallocator new_string_reallocator(void) {
    struct StringReallocator reallocator;
    reallocator.capacity = 0;
    reallocator.length = 0;
    reallocator.string = NULL;

    return reallocator;
}

static void add_char(struct StringReallocator* reallocator, char ch) {
    int alloc_size = 20;
    if (reallocator->capacity == 0) {
        reallocator->string = (char*)malloc(alloc_size * sizeof(char));
        reallocator->capacity += alloc_size;
    }

    if (reallocator->length >= reallocator->capacity) {
        reallocator->string = (char*)realloc(reallocator->string, (reallocator->capacity + alloc_size) * sizeof(char));
        reallocator->capacity += alloc_size;
    }

    reallocator->string[reallocator->length] = ch;
    reallocator->length++;
}

static struct CharPeeker new_char_peeker(char* file_buff, long file_size) {
    struct CharPeeker peeker;

    peeker.file_buff = file_buff;
    peeker.file_size = file_size;
    peeker.current_pos = 0;
    peeker.row = 1;
    peeker.col = 1;

    return peeker;
}

static char* next_char(struct CharPeeker* peeker) {
    if (peeker->current_pos < peeker->file_size) {
        peeker->current_pos++;
    }

    return peek_char(peeker);
}

static char* peek_char(struct CharPeeker* peeker) {
    if (peeker->current_pos >= peeker->file_size) {
        return NULL;
    }

    return &(peeker->file_buff[peeker->current_pos]);
}

const char* get_token_type_string(enum TokenType token_type) {
    switch (token_type) {
        case QUOTED_STRING: return "QUOTED_STRING";
        case UNQUOTED_STRING: return "UNQUOTED_STRING";
        case NUMBER: return "NUMBER";
        case NEW_LINE: return "NEW_LINE";

        case COMMA: return "COMMA";
        case SEMICOLON: return "SEMICOLON";

        case ASSIGN_OPERATOR: return "ASSIGN_OPERATOR";

        case EQUALS: return "EQUALS";
        case GREATER_THAN: return "GREATER_THAN";
        case LESSER_THAN: return "LESSER_THAN";

        case OPEN_ROUND_BRACKET: return "OPEN_ROUND_BRACKET";
        case CLOSE_ROUND_BRACKET: return "CLOSE_ROUND_BRACKET";

        case PLUS: return "PLUS";
        case MINUS: return "MINUS";

        case ASTERISK: return "ASTERISK";
        case SLASH: return "SLASH";
    }

    return "UNKNOWN TOKEN";
}

static long skip_whitespaces(struct CharPeeker* peeker) {
    long read_bytes = 0;
    char* char_at_pos = peek_char(peeker);
    while (char_at_pos != NULL && (*char_at_pos == ' ' || *char_at_pos == '\t')) {
        read_bytes++;
        char_at_pos = next_char(peeker);
    }

    peeker->col += read_bytes;
    return read_bytes;
}

static long read_unquoted_string_token(struct CharPeeker* peeker, struct Token* token) {
    long read_bytes = 0;

    char* char_at_pos = peek_char(peeker);
    if (!isalpha(*char_at_pos)) {
        return read_bytes;
    }

    token->col = peeker->col;
    token->row = peeker->row;

    struct StringReallocator value = new_string_reallocator();

    while (char_at_pos != NULL && isalpha(*char_at_pos)) {
        add_char(&value, *char_at_pos);

        read_bytes++;
        char_at_pos = next_char(peeker);
    }

    add_char(&value, 0);

    token->token_type = UNQUOTED_STRING;

    string_to_lowercase(value.string);
    token->value = value.string;

    peeker->col += read_bytes;
    return read_bytes;
}

static long read_quoted_string_token(struct CharPeeker* peeker, struct Token* token) {
    long read_bytes = 0;

    char* char_at_pos = peek_char(peeker);
    if (*char_at_pos != '"') {
        return read_bytes;
    }

    token->col = peeker->col;
    token->row = peeker->row;

    read_bytes++;
    char_at_pos = next_char(peeker);
    struct StringReallocator value = new_string_reallocator();

    while (char_at_pos != NULL && *char_at_pos != '"') {
        add_char(&value, *char_at_pos);
        read_bytes++;

        char_at_pos = next_char(peeker);
    }

    add_char(&value, 0);
    read_bytes++;
    next_char(peeker);
    
    token->token_type = QUOTED_STRING;
    token->value = value.string;

    peeker->col += read_bytes;
    return read_bytes;
}

static void string_to_lowercase(char * string) {
    for(char *p=string; *p; p++) *p=tolower(*p);
}

static long read_new_line_token(struct CharPeeker* peeker, struct Token* token) {
    char* char_at_pos = peek_char(peeker);
    if (*char_at_pos != '\n') {
        return 0;
    }

    token->token_type = NEW_LINE;
    token->value = NULL;
    token->col = peeker->col;
    token->row = peeker->row;

    next_char(peeker);
    peeker->row++;
    peeker->col = 1;

    return 1;
}

static long read_number_token(struct CharPeeker* peeker, struct Token* token) {
    long read_bytes = 0;
    char* char_at_pos = peek_char(peeker);

    if (!isdigit(*char_at_pos)) {
        return read_bytes;
    }

    token->col = peeker->col;
    token->row = peeker->row;

    struct StringReallocator value = new_string_reallocator();

    while (char_at_pos != NULL && (isdigit(*char_at_pos) || *char_at_pos == '.')) {
        add_char(&value, *char_at_pos);
        read_bytes++;

        char_at_pos = next_char(peeker);
    }

    add_char(&value, 0);

    token->token_type = NUMBER;
    token->value = value.string;

    peeker->col += read_bytes;
    return read_bytes;
}

static long read_special_char(struct CharPeeker* peeker, struct Token* token) {
    char* char_at_pos = peek_char(peeker);

    if (*char_at_pos == ',') {
        struct StringReallocator value = new_string_reallocator();
        add_char(&value, ',');
        add_char(&value, 0);

        next_char(peeker);
        token->col = peeker->col;
        token->row = peeker->row;
        token->token_type = COMMA;
        token->value = value.string;

        peeker->col++;
        return 1;
    }

    if (*char_at_pos == ';') {
        struct StringReallocator value = new_string_reallocator();
        add_char(&value, ';');
        add_char(&value, 0);

        next_char(peeker);
        token->col = peeker->col;
        token->row = peeker->row;
        token->token_type = SEMICOLON;
        token->value = value.string;

        peeker->col++;
        return 1;
    }

    if (*char_at_pos == '=') {
        struct StringReallocator value = new_string_reallocator();
        add_char(&value, '=');
        add_char(&value, 0);

        next_char(peeker);
        token->col = peeker->col;
        token->row = peeker->row;
        token->token_type = ASSIGN_OPERATOR;
        token->value = value.string;

        peeker->col++;
        return 1;
    }

    if (*char_at_pos == '+') {
        struct StringReallocator value = new_string_reallocator();
        add_char(&value, '+');
        add_char(&value, 0);

        next_char(peeker);
        token->col = peeker->col;
        token->row = peeker->row;
        token->token_type = PLUS;
        token->value = value.string;

        peeker->col++;
        return 1;
    }

    if (*char_at_pos == '/') {
        struct StringReallocator value = new_string_reallocator();
        add_char(&value, '/');
        add_char(&value, 0);

        next_char(peeker);
        token->col = peeker->col;
        token->row = peeker->row;
        token->token_type = SLASH;
        token->value = value.string;

        peeker->col++;
        return 1;
    }

    if (*char_at_pos == '*') {
        struct StringReallocator value = new_string_reallocator();
        add_char(&value, '*');
        add_char(&value, 0);

        next_char(peeker);
        token->col = peeker->col;
        token->row = peeker->row;
        token->token_type = ASTERISK;
        token->value = value.string;

        peeker->col++;
        return 1;
    }

    if (*char_at_pos == '(') {
        struct StringReallocator value = new_string_reallocator();
        add_char(&value, '(');
        add_char(&value, 0);

        next_char(peeker);
        token->col = peeker->col;
        token->row = peeker->row;
        token->token_type = OPEN_ROUND_BRACKET;
        token->value = value.string;

        peeker->col++;
        return 1;
    }

    if (*char_at_pos == ')') {
        struct StringReallocator value = new_string_reallocator();
        add_char(&value, ')');
        add_char(&value, 0);

        next_char(peeker);
        token->col = peeker->col;
        token->row = peeker->row;
        token->token_type = CLOSE_ROUND_BRACKET;
        token->value = value.string;

        peeker->col++;
        return 1;
    }

    return 0;
}

struct TokenList read_tokens(char* file_buff, long fsize) {
    struct TokenList tokens = new_token_list();
    struct CharPeeker peeker = new_char_peeker(file_buff, fsize);

    while (peek_char(&peeker) != NULL) {
        long read_bytes = 0;
        read_bytes = skip_whitespaces(&peeker);
        if (read_bytes != 0) {
            continue;
        }

        struct Token token;
        read_bytes = read_unquoted_string_token(&peeker, &token);
        if (read_bytes != 0) {
            add_token(&tokens, token);
            continue;
        }

        read_bytes = read_quoted_string_token(&peeker, &token);
        if (read_bytes != 0) {
            add_token(&tokens, token);
            continue;
        }

        read_bytes = read_new_line_token(&peeker, &token);
        if (read_bytes != 0) {
            add_token(&tokens, token);
            continue;
        }

        read_bytes = read_number_token(&peeker, &token);
        if (read_bytes != 0) {
            add_token(&tokens, token);
            continue;
        }

        read_bytes = read_special_char(&peeker, &token);
        if (read_bytes != 0) {
            add_token(&tokens, token);
            continue;
        }

        printf("Undefined token at position %ld \n", peeker.current_pos);
        exit(1);
    }

    return tokens;
}