#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "../UTILS/utils.h"

Token *tokens; // single linked list of tokens
Token *lastTk; // the last token in list

int line = 1; // the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code)
{
	Token *tk = safeAlloc(sizeof(Token));
	tk->code = code;
	tk->line = line;
	tk->next = NULL;
	if (lastTk)
	{
		lastTk->next = tk;
	}
	else
	{
		tokens = tk;
	}
	lastTk = tk;
	return tk;
}

char *extract(const char *begin, const char *end)
{
	int length = end - begin;
	char *result = malloc((length + 1) * sizeof(char)); // or safeAlloc(length+1)
	if (result == NULL)
	{
		perror("Error for allocating memory.\n");
	}
	strncpy(result, begin, length);
	result[length] = '\0';
	return result;
}

Token *tokenize(const char *pch)
{
	const char *start;
	Token *tk;
	for (;;)
	{
		switch (*pch)
		{
		case ' ':
		case '\t':
			pch++;
			break;
		case '\r': // handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
			if (pch[1] == '\n')
				pch++;
			// fallthrough to \n
		case '\n':
			line++;
			pch++;
			break;
		case '\0':
			addTk(END);
			return tokens;
		case ',':
			addTk(COMMA);
			pch++;
			break;
		case '(':
			addTk(LPAR);
			pch++;
			break;
		case ')':
			addTk(RPAR);
			pch++;
			break;
		case '{':
			addTk(LACC);
			pch++;
			break;
		case '}':
			addTk(RACC);
			pch++;
			break;
		case '[':
			addTk(LBRACKET);
			pch++;
			break;
		case ']':
			addTk(RBRACKET);
			pch++;
			break;
		case ';':
			addTk(SEMICOLON);
			pch++;
			break;
		case '+':
			addTk(ADD);
			pch++;
			break;
		case '*':
			addTk(MUL);
			pch++;
			break;
		case '-':
			addTk(SUB);
			pch++;
			break;
		case '.':
            addTk(DOT);
            pch++;
            break;
		case '!':
			if (pch[1] == '=')
			{
				addTk(NOTEQ);
				pch += 2;
				break;
			}
			else
			{
				addTk(NOT);
				pch += 1;
				break;
			}
		case '|':
			if (pch[1] == '|')
			{
				addTk(OR);
				pch += 2;
			}
			else 
				err("Error at |\n");
			break;
		case '<':
			if (pch[1] == '=')
			{
				addTk(LESSEQ);
				pch += 2;
				break;
			}
			else
			{
				addTk(LESS);
				pch++;
				break;
			}
		case '>':
			if (pch[1] == '=')
			{
				addTk(GREATEREQ);
				pch += 2;
				break;
			}
			else
			{
				addTk(GREATER);
				pch++;
				break;
			}

		case '=':
			if (pch[1] == '=')
			{
				addTk(EQUAL);
				pch += 2;
			}
			else
			{
				addTk(ASSIGN);
				pch++;
			}
			break;
		case '&':
			if (pch[1] == '&')
			{
				addTk(AND);
				pch += 2;
			}
			else
				err("Errror for &\n");
			break;
		case '/':
			if (pch[1] == '/')
			{
				while (*pch != '\0' && *pch != '\n')
				{
					pch++;
				}
			}
			else
			{
				addTk(DIV);
				pch++;
			}
			break;

		default:
			if (isalpha(*pch) || *pch == '_')
			{
				for (start = pch++; isalnum(*pch) || *pch == '_'; pch++)
				{
				}
				char *text = extract(start, pch);
				if (strcmp(text, "char") == 0)
				{
					addTk(TYPE_CHAR);
					break;
				}
				if (strcmp(text, "int") == 0)
				{
					addTk(TYPE_INT);
					break;
				}
				if (strcmp(text, "double") == 0)
				{
					addTk(TYPE_DOUBLE);
					break;
				}
				if (strcmp(text, "if") == 0)
				{
					addTk(IF);
					break;
				}
				if (strcmp(text, "else") == 0)
				{
					addTk(ELSE);
					break;
				}
				if (strcmp(text, "while") == 0)
				{
					addTk(WHILE);
					break;
				}
				if (strcmp(text, "struct") == 0)
				{
					addTk(STRUCT);
					break;
				}
				if (strcmp(text, "return") == 0)
				{
					addTk(RETURN);
					break;
				}
				if (strcmp(text, "void") == 0)
				{
					addTk(VOID);
					break;
				}
				else
				{
					tk = addTk(ID);
					tk->text = text;
				}
			}
			else if (isdigit(*pch)) // NUMBER
			{
				short isDouble = 0;
				for (start = pch++; isdigit(*pch) || *pch == '.' || *pch == 'e'|| *pch == 'E' || *pch == '-' || *pch == '+'; pch++)
				{
					if (*pch == '.' || *pch == 'e' || *pch == 'E')
					{
						isDouble = 1;
					}
				}
				char *text = extract(start, pch);

				if (isDouble) // DOUBLE
                {
                	if(!isdigit(*(pch-1)))
                	{
                   		if(*(pch-1) == '+' || *(pch-1) == '-')
                   		{
                   			err("miss number after eE+/-");
                   		}
                   		if(*(pch-1) == 'e' || *(pch-1) == 'E')
                   		{
                   			err("miss number after eE");
                   		}
                   		else
                   		{
                   			err("miss number after .");
                   		}
                	}
                    tk = addTk(DOUBLE);
                    char *endptr;
                    tk->d = strtod(text, &endptr);
                }
				else // INTEGER
				{
					tk = addTk(INT);
					tk->i = atoi(text);
				}
			}
			else if (*pch == '"') // STRING
			{
				pch++;
				for (start = pch++; isalnum(*pch) || (*pch == '"' && *pch != '\0'); pch++)
				{
				}
				char *text = extract(start, pch - 1);
				tk = addTk(STRING);
				tk->text = text;
			}
			else if (*pch == '\'') // CHARACTER
			{
				pch += 2;
				if(*pch == '\'')
					{char *character = extract(start, pch - 1);
					tk = addTk(CHAR);
					tk->c = *character;}
				else	err("Missing \' \n");
				pch +=1;
			}
			else
				err("invalid char: %c (%d)", *pch, *pch);
		}
	}
}

void showTokens(const Token *tokens)
{
	int line_counter = 1;
	for (const Token *tk = tokens; tk; tk = tk->next)
	{
		printf("%d\t", tk->line);
		switch (tk->code)
		{
		case TYPE_INT:
			printf("TYPE_INT\n");
			break;
		case TYPE_CHAR:
			printf("TYPE_CHAR\n");
			break;
		case TYPE_DOUBLE:
			printf("TYPE_DOUBLE\n");
			break;
		case ID:
			printf("ID:%s\n", tk->text);
			break;
		case COMMA:
			printf("COMMA\n");
			break;
		case DOT:
			printf("DOT\n");
			break;
		case LPAR:
			printf("LPAR\n");
			break;
		case RPAR:
			printf("RPAR\n");
			break;
		case LACC:
			printf("LACC\n");
			break;
		case RACC:
			printf("RACC\n");
			break;
		case LBRACKET:
			printf("LBRACKET\n");
			break;
		case RBRACKET:
			printf("RBRACKET\n");
			break;
		case SEMICOLON:
			printf("SEMICOLON\n");
			break;
		case INT:
			printf("INT:%d\n", tk->i);
			break;
		case DOUBLE:
			printf("DOUBLE:%f\n", tk->d);
			break;
		case STRING:
			printf("STRING:%s\n", tk->text);
			break;
		case CHAR:
			printf("CHAR:%c\n", tk->c);
			break;
		case WHILE:
			printf("WHILE\n");
			break;
		case LESS:
			printf("LESS\n");
			break;
		case DIV:
			printf("DIV\n");
			break;
		case ADD:
			printf("ADD\n");
			break;
		case AND:
			printf("AND\n");
			break;
		case MUL:
			printf("MUL\n");
			break;
		case IF:
			printf("IF\n");
			break;
		case ASSIGN:
			printf("ASSIGN\n");
			break;
		case EQUAL:
			printf("EQUAL\n");
			break;
		case RETURN:
			printf("RETURN\n");
			break;
		case END:
			printf("END\n");
			break;
		case ELSE:
			printf("ELSE\n");
			break;
		case STRUCT:
			printf("STRUCT\n");
			break;
		case VOID:
			printf("VOID\n");
			break;
		case SUB:
			printf("SUB\n");
			break;
		case OR:
			printf("OR\n");
			break;
		case NOT:
			printf("NOT\n");
			break;
		case NOTEQ:
			printf("NOTEQ\n");
			break;
		case LESSEQ:
			printf("LESSEQ\n");
			break;
		case GREATER:
			printf("GREATER\n");
			break;
		case GREATEREQ:
			printf("GREATEREQ\n");
			break;

		default:
			printf("Unknown, %d.\n", tk->code);
			break;
		}
		line_counter += 1;
	}
}
