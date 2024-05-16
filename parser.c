#include "lexer.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

struct Program parse(struct TokenList tokens) {
    struct Program program;
    struct AstNode* statements = (struct AstNode*)malloc(20 * sizeof(struct AstNode));
    program.size = 0;
    program.capacity = 20;

    TokenPeeker t_p;
    t_p.current_index = 0;
    t_p.tokens = &tokens;

    TokenPeeker* token_peeker = &t_p;

    while (peek(token_peeker) != NULL) {
        struct Token* first_token = peek(token_peeker);
        struct AstNode statement;

        if (first_token->token_type == UNQUOTED_STRING) {
            if (strcmp(first_token->value, "print") == 0) {
                next(token_peeker);
                statement.node_type = COMMAND_STATEMENT;

                statement.data.command_statement.capacity = 10;
                statement.data.command_statement.expressions = (AstNode*)malloc(10 * sizeof(AstNode));
                statement.data.command_statement.size = 0;

                while (peek(token_peeker) != NULL && peek(token_peeker)->token_type != NEW_LINE) {
                    struct Token* print_arg = peek(token_peeker);

                    if (print_arg->token_type == QUOTED_STRING) {
                        ConstStringExpression string;
                        string.value = print_arg->value;

                        AstNode string_ast;
                        string_ast.node_type = CONST_STRING_EXPRESSION;
                        string_ast.data.const_string_expression = string;

                        statement.data.command_statement.expressions[statement.data.command_statement.size] = string_ast;
                        statement.data.command_statement.size++;
                    }

                    struct Token* next_t = next(token_peeker);

                    if (next_t != NULL && next_t->token_type == NEW_LINE) {
                        next(token_peeker);
                        break;                        
                    }

                    if (next_t == NULL){
                        break;
                    }
                    
                    if (next_t->token_type != COMMA || next_t->token_type != SEMICOLON) {
                        printf("Expected to get ; or , after print arg");
                        exit(1);
                    }
                }

                statements[program.size] = statement;
                program.size++;
            }
        }
    }

    program.statements = statements;
    return program;
}
