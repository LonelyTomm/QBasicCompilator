#include "lexer.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct StatementsList new_statements_list(void);
void add_statement_to_list(struct StatementsList* list, struct AstNode statement);

struct ExpessionsList new_expressions_list(void);
void add_expression_to_list(struct ExpessionsList* list, struct AstNode expression);

typedef struct TokenPeeker {
    long current_index;
    struct TokenList* tokens;
} TokenPeeker;

struct TokenPeeker new_token_peeker(struct TokenList* tokens);
struct Token* next(TokenPeeker* token_peeker);
struct Token* peek(TokenPeeker* token_peeker);

struct StatementsList parse_statements(struct TokenPeeker* token_peeker);
struct AstNode* parse_print_statement(struct TokenPeeker* token_peeker);
struct AstNode* parse_assign_statement(struct TokenPeeker* token_peeker);

struct AstNode* parse_infix_expression(struct TokenPeeker* token_peeker, struct AstNode* left);
struct AstNode* parse_expression(struct TokenPeeker* token_peeker, int precedence);
struct AstNode* parse_prefix_expression(struct TokenPeeker* token_peeker);
struct AstNode* parse_node_from_token(struct TokenPeeker* token_peeker);
int get_operator_precedence(struct Token* operator);
void print_node(struct AstNode* node);
void skip_newlines(struct TokenPeeker* token_peeker);


// END DECLARATIONS

void print_node(struct AstNode* node) {
    /// printf("ENTERED tt is %d\n", node->node_type);
    if (node == NULL) {
        printf("UNDEFINED ");
        return;
    }

    if (node->node_type == CONST_NUMBER_EXPRESSION) {
        printf("%s ", node->const_number_expression.token->value);
    } else if (node->node_type == CONST_STRING_EXPRESSION) {
        printf("%s ", node->const_string_expression.token->value);
    } else if (node->node_type == PREFIX_EXPRESSION) {
        printf("%s(", node->prefix_expression.operator->value);
        print_node(node->prefix_expression.value);
        printf(") ");
    } else if (node->node_type == INFIX_EXPRESSION) {
        printf("(");
        print_node(node->infix_expression.left);
        printf("%s", node->infix_expression.operator->value);
        print_node(node->infix_expression.right);
        printf(")");
    } else if (node->node_type == IDENTIFIER_EXPRESSION) {
        printf("IDENT(%s) ", node->identifier_expression.token->value);
    } else if (node->node_type == ASSIGN_STATEMENT) {
        print_node(node->assign_statement.identifier);
        printf("= ");
        print_node(node->assign_statement.expression);
    } else if (node->node_type == PRINT_STATEMENT) {
        printf("%s ( ", node->print_statement.token->value);
        for (int i = 0;i < node->print_statement.expressions.size;i++) {
            print_node(&node->print_statement.expressions.expressions[i]);
            printf(", ");
        }

        printf(" ) ");
    } else if (node->node_type == IF_STATEMENT) {
        printf("if ");
        print_node(node->if_statement.condition_expression);
        printf(" then\n");

        for(int i = 0; i < node->if_statement.body->size;i++) {
            printf("    ");
            print_node(&node->if_statement.body->statements[i]);
            printf("\n");
        }

        printf("end if\n");
    }
}

struct StatementsList new_statements_list(void) {
    struct StatementsList list;
    list.capacity = 0;
    list.size = 0;
    list.statements = NULL;

    return list;
}

void add_statement_to_list(struct StatementsList* list, struct AstNode statement) {
    int alloc_size = 20;
    if (list->capacity == 0) {
        list->statements = (struct AstNode*)malloc(alloc_size * sizeof(struct AstNode));
        list->capacity = alloc_size;
    }

    if (list->size >= list->capacity) {
        list->statements = (struct AstNode*)realloc(list->statements, (alloc_size + list->capacity) * sizeof(struct AstNode));
        list->capacity = list->capacity + alloc_size;
    }

    list->statements[list->size] = statement;
    list->size++;
}

struct ExpessionsList new_expressions_list(void) {
    struct ExpessionsList list;
    list.capacity = 0;
    list.size = 0;
    list.expressions = NULL;

    return list;
}

void add_expression_to_list(struct ExpessionsList* list, struct AstNode expression) {
    int alloc_size = 20;
    if (list->capacity == 0) {
        list->expressions = (struct AstNode*)malloc(alloc_size * sizeof(struct AstNode));
        list->capacity = alloc_size;
    }

    if (list->size >= list->capacity) {
        list->expressions = (struct AstNode*)realloc(list->expressions, (alloc_size + list->capacity) * sizeof(struct AstNode));
        list->capacity = list->capacity + alloc_size;
    }

    list->expressions[list->size] = expression;
    list->size++;
}

struct TokenPeeker new_token_peeker(struct TokenList* tokens) {
    struct TokenPeeker token_peeker;
    token_peeker.current_index = 0;
    token_peeker.tokens = tokens;

    return token_peeker;
}

struct Token* peek(TokenPeeker* token_peeker) {
    if (token_peeker->current_index >= token_peeker->tokens->length) {
        return NULL;
    }

    return &(token_peeker->tokens->tokens[token_peeker->current_index]);
}

struct Token* next(TokenPeeker* token_peeker) {
    if (token_peeker->current_index < token_peeker->tokens->length) {
        token_peeker->current_index++;
    }

    return peek(token_peeker);
}

struct AstNode* parse_expression(struct TokenPeeker* token_peeker, int precedence) {
    struct Token* token = peek(token_peeker);

    struct AstNode* leftExpr = NULL;
    if (token->token_type == OPEN_ROUND_BRACKET) {
        next(token_peeker);
        leftExpr = parse_expression(token_peeker, -1);
    } else {
        leftExpr = parse_node_from_token(token_peeker);
        if (leftExpr == NULL) {
            if (token->token_type == MINUS || token->token_type == BANG) {
                leftExpr = parse_prefix_expression(token_peeker);
            }
        }
    }

    if (leftExpr == NULL) {
        return NULL;
    }

    if (peek(token_peeker) == NULL || peek(token_peeker)->token_type == CLOSE_ROUND_BRACKET) {
        next(token_peeker);
        return leftExpr;
    }

    while (peek(token_peeker) != NULL && peek(token_peeker)->token_type != NEW_LINE && precedence < get_operator_precedence(peek(token_peeker))) {
        leftExpr = parse_infix_expression(token_peeker, leftExpr);
    }

    return leftExpr;
}

struct AstNode* parse_prefix_expression(struct TokenPeeker* token_peeker) {
    struct Token* operator = peek(token_peeker);
    struct AstNode* node = (struct AstNode*)malloc(sizeof(struct AstNode));

    node->node_type = PREFIX_EXPRESSION;
    node->prefix_expression.operator = operator;
    node->prefix_expression.value = parse_expression(token_peeker, -1);

    return node;
}

int get_operator_precedence(struct Token* operator) {
    switch (operator->token_type)
    {
    case PLUS:
        return 0;
    case MINUS:
        return 0;
    case SLASH:
        return 1;
    case ASTERISK:
        return 1;
    default:
        return -1;
    }
}

struct AstNode* parse_infix_expression(struct TokenPeeker* token_peeker, struct AstNode* left) {
    struct AstNode* node = (struct AstNode*)malloc(sizeof(struct AstNode));
    node->node_type = INFIX_EXPRESSION;
    node->infix_expression.left = left;

    struct Token* operator = peek(token_peeker);
    node->infix_expression.operator = operator;

    next(token_peeker);

    node->infix_expression.right = parse_expression(token_peeker, get_operator_precedence(operator));

    return node;
}

struct AstNode* parse_node_from_token(struct TokenPeeker* token_peeker) {
    struct Token* token = peek(token_peeker);

    if (token->token_type == QUOTED_STRING) {
        next(token_peeker);
        struct AstNode* node = (struct AstNode*)malloc(sizeof(struct AstNode));
        node->node_type = CONST_STRING_EXPRESSION;
        node->const_string_expression.token = token;
        return node;
    }

    if (token->token_type == NUMBER) {
        next(token_peeker);
        struct AstNode* node = (struct AstNode*)malloc(sizeof(struct AstNode));
        node->node_type = CONST_NUMBER_EXPRESSION;
        node->const_number_expression.token = token;
        return node;
    }

    if (token->token_type == UNQUOTED_STRING) {
        next(token_peeker);
        struct AstNode* node = (struct AstNode*)malloc(sizeof(struct AstNode));
        node->node_type = IDENTIFIER_EXPRESSION;
        node->identifier_expression.token = token;
        return node;
    }

    return NULL;
}

struct AstNode* parse_string_const(struct TokenPeeker* token_peeker) {
    struct Token* token = peek(token_peeker);
    struct AstNode* node = (struct AstNode*)malloc(sizeof(struct AstNode));

    node->node_type = CONST_STRING_EXPRESSION;
    node->const_string_expression.token = token;

    return node;
}

struct AstNode* parse_assign_statement(struct TokenPeeker* token_peeker) {
    struct AstNode* statement = (struct AstNode*)malloc(sizeof(struct AstNode));
    statement->node_type = ASSIGN_STATEMENT;
    statement->assign_statement.identifier = parse_node_from_token(token_peeker);
    
    struct Token* token = peek(token_peeker);
    if (token == NULL || token->token_type != ASSIGN_OPERATOR) {
        exit(1);
    }

    next(token_peeker);
    struct AstNode* arg = parse_expression(token_peeker, -1);
    statement->assign_statement.expression = arg;

    return statement;
}

struct AstNode* parse_print_statement(struct TokenPeeker* token_peeker) {
    struct AstNode* statement = (struct AstNode*)malloc(sizeof(struct AstNode));
    statement->node_type = PRINT_STATEMENT;
    statement->print_statement.expressions = new_expressions_list();
    statement->print_statement.token = peek(token_peeker);

    next(token_peeker);

    struct AstNode* arg = parse_expression(token_peeker, -1);
    while (arg != NULL) {
        add_expression_to_list(&statement->print_statement.expressions, *arg);

        struct Token* token = peek(token_peeker);
        if (token == NULL || token->token_type == NEW_LINE) {
            skip_newlines(token_peeker);
            break;
        } else if (token->token_type == SEMICOLON || token->token_type == COMMA) {
            next(token_peeker);
            arg = parse_expression(token_peeker, -1);
            continue;
        } else {
            exit(1);
        }
    }

    return statement;
}

struct AstNode* parse_if_statement(struct TokenPeeker* token_peeker) {
    struct AstNode* node = (struct AstNode*)malloc(sizeof(struct AstNode));
    node->node_type = IF_STATEMENT;
    node->if_statement.token = peek(token_peeker);

    next(token_peeker);
    node->if_statement.condition_expression = parse_expression(token_peeker, -1);

    struct Token* token = peek(token_peeker);
    if (token->token_type != UNQUOTED_STRING || strcmp(token->value, "then") != 0) {
        exit(20);
    }
    next(token_peeker);

    skip_newlines(token_peeker);
    struct StatementsList list = parse_statements(token_peeker);
    node->if_statement.body = &list;
    skip_newlines(token_peeker);

    token = peek(token_peeker);
    if (token == NULL) {
        exit(15);
    } else if (
        token->token_type == UNQUOTED_STRING && 
        (
            strcmp(token->value, "elseif") == 0 ||
            strcmp(token->value, "else") == 0
        )
    ) {
        while (token != NULL && token->token_type == UNQUOTED_STRING && 
        (
            strcmp(token->value, "elseif") == 0 ||
            strcmp(token->value, "else") == 0
        )) {
            if (strcmp(token->value, "else") == 0) {
                next(token_peeker);
                skip_newlines(token_peeker);

                struct StatementsList list = parse_statements(token_peeker);
                skip_newlines(token_peeker);
            }


        }
    }

    if (token->token_type == UNQUOTED_STRING && strcmp(token->value, "end") == 0) {
    
    } else {
        exit(353);
    }

    token = next(token_peeker);
    if (token->token_type != UNQUOTED_STRING || strcmp(token->value, "if") != 0) {
        exit(19);
    }

    next(token_peeker);
    skip_newlines(token_peeker);

    return node;
}

void skip_newlines(struct TokenPeeker* token_peeker) {
    while(peek(token_peeker) != NULL && peek(token_peeker)->token_type == NEW_LINE) {
        next(token_peeker);
    }
}

struct StatementsList parse_statements(struct TokenPeeker* token_peeker) {
    struct StatementsList list = new_statements_list();
    while (peek(token_peeker) != NULL) {
        struct Token* first_token = peek(token_peeker);
        
        if (
            first_token->token_type == UNQUOTED_STRING &&
            strcmp(first_token->value, "print") == 0
        ) {
            struct AstNode* print_statement = parse_print_statement(token_peeker);
            print_node(print_statement);
            add_statement_to_list(&list, *print_statement);
            continue;
        }

        if (
            first_token->token_type == UNQUOTED_STRING &&
            strcmp(first_token->value, "if") == 0
        ) {
            struct AstNode* if_statement = parse_if_statement(token_peeker);
            print_node(if_statement);
            add_statement_to_list(&list, *if_statement);
            continue;
        }

        // if (
        //     first_token->token_type == UNQUOTED_STRING &&
        //     strcmp(first_token->value, "if") == 0
        // ) {
        //     // struct AstNode print_statement = parse_print_statement(&token_peeker);
        //     // add_statement_to_program(&program, print_statement);
        //     continue;
        // }

        if (
            first_token->token_type == UNQUOTED_STRING &&
            (
                strcmp(first_token->value, "end") == 0 ||
                strcmp(first_token->value, "else") == 0
            )
        ) {
            return list;
        }

        // COULDN'T RECOGNIZE OPERATOR SO PROBABLY IS ASSIGNMENT
        if (first_token->token_type == UNQUOTED_STRING) {
            struct AstNode* assign_statement = parse_assign_statement(token_peeker);
            print_node(assign_statement);
            add_statement_to_list(&list, *assign_statement);
            continue;
        }

        exit(1);
    }

    return list;
}

struct Program parse(struct TokenList tokens) {
    struct Program program;
    struct TokenPeeker token_peeker = new_token_peeker(&tokens);

    struct StatementsList list = parse_statements(&token_peeker);
    program.list = &list;

    return program;
}
