#ifndef PARSER_H_
#define PARSER_H_

enum AstNodeType {
    ASSIGN_STATEMENT,
    PRINT_STATEMENT,

    IDENTIFIER_EXPRESSION,
    CONST_STRING_EXPRESSION,
    CONST_NUMBER_EXPRESSION,

    PREFIX_EXPRESSION,
    INFIX_EXPRESSION,

    IF_STATEMENT,
};

typedef struct Program {
    struct StatementsList* list;
} Program;

typedef struct ExpessionsList {
    long size;
    long capacity;
    struct AstNode* expressions;
} ExpessionsList;

typedef struct StatementsList {
    long size;
    long capacity;
    struct AstNode* statements;
} StatementsList;

typedef struct AssignStatement {
    struct AstNode* identifier;
    struct AstNode* expression;
} AssignStatement;

typedef struct PrintStatement {
    struct Token* token;
    struct ExpessionsList expressions;
} PrintStatement;

typedef struct ConstStringExpression {
    struct Token* token;
} ConstStringExpression;

typedef struct ConstNumberExpression {
    struct Token* token;
} ConstNumberExpression;

typedef struct IdentifierExpression {
    struct Token* token;
} IdentifierExpression;

typedef struct PrefixExpression {
    struct Token* operator;
    struct AstNode* value;
} PrefixExpression;

typedef struct InfixExpression {
    struct Token* operator;
    struct AstNode* left;
    struct AstNode* right; 
} InfixExpression;

typedef struct IfStatement {
    struct Token* token;
    struct AstNode* condition_expression;
    struct StatementsList* body;
    struct StatementsList* elses;
} IfStatement;

typedef struct AstNode {
    enum AstNodeType node_type;
    union {
        AssignStatement assign_statement;
        PrintStatement print_statement;
        ConstStringExpression const_string_expression;
        ConstNumberExpression const_number_expression;
        IdentifierExpression identifier_expression;
        PrefixExpression prefix_expression;
        InfixExpression infix_expression;
        IfStatement if_statement;
    };
} AstNode;

struct Program parse(struct TokenList tokens);

#endif