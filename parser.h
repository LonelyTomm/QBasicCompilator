#ifndef PARSER_H_
#define PARSER_H_

enum AstNodeType {
    EXPRESSION,
    ASSIGN_STATEMENT,
    CONST_NUM_EXPRESSION,
    CONST_STRING_EXPRESSION,

    COMMAND_STATEMENT,
};

typedef struct CommandStatement {
    long size;
    long capacity;
    char* var_name;
    struct AstNode* expressions;
} CommandStatement;

typedef struct ConstNumExpression {
    double value;
} ConstNumExpression;

typedef struct ConstStringExpression {
    char* value;
} ConstStringExpression;

typedef struct Program {
    long size;
    long capacity;
    struct AstNode* statements;
} Program;

typedef struct AssignStatement {
    char* var_name;
    struct AstNode* assign_value;
} AssignStatement;

typedef struct AstNode {
    enum AstNodeType node_type;
    union {
        CommandStatement command_statement; 
        ConstStringExpression const_string_expression;
    } data;
} AstNode;

struct Program parse(struct TokenList tokens);

typedef struct TokenPeeker {
    long current_index;
    struct TokenList* tokens;
} TokenPeeker;

struct Token* next(TokenPeeker* token_peeker);
struct Token* peek(TokenPeeker* token_peeker);

#endif