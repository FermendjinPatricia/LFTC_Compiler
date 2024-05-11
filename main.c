#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ALEX/lexer.h"
#include "ASIN/parser.h"
#include "UTILS/utils.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        perror("Invalid argument.\n");
        exit(-1);
    }

    char *defaultPath = "./tests/";
    char *file = malloc(sizeof(char) * (strlen(defaultPath) + strlen(argv[1]))); // or safeAlloc(strlen(defaultPath) + strlen(argv[1]))
    if(file == NULL){
        perror("Error at alocating memory.\n");
        exit(-1);
    }
    sprintf(file, "./tests/%s", argv[1]);
    char *inbuffer = loadFile(file);
    Token *tokens = tokenize(inbuffer);
    //free(inbuffer);
    showTokens(tokens);
    parse(tokens);
    free(inbuffer);

    return 0;
}
