#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

int main(void) {
    FILE* file = fopen("test.qb", "r");
    if (file == NULL) {
        printf("Error reading a file \n");
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_buff = (char*)malloc(fsize * sizeof(char) + 1);
    fread(file_buff, sizeof(char), fsize, file);
    file_buff[fsize] = 0;
    fclose(file);

    struct TokenList tokens = read_tokens(file_buff, fsize);
    printf("TokenList tokens is %ld \n", tokens.length);

    for (int i = 0;i < tokens.length;i++) {
        printf(
            "Token type is %s | %s | row %d | col %d \n", 
            get_token_type_string(tokens.tokens[i].token_type), 
            tokens.tokens[i].value, 
            tokens.tokens[i].row, 
            tokens.tokens[i].col
        );
    }

    // struct Program program = parse(tokens);

    // printf("program size is %ld \n", program.size);

    free(file_buff);
}