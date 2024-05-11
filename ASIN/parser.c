#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "parser.h"
#include "/home/patricia/LFTC/COMPILER/AD/ad.h"
#include "/home/patricia/LFTC/COMPILER/UTILS/utils.h"

Token *iTk;        // the iterator in the tokens list
Token *consumedTk; // the last consumed token

bool isInStruct = false;

Symbol *owner = NULL;

void tkerr(const char *fmt, ...)
{
    fprintf(stderr, "error in line %d: ", iTk->line);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool consume(int code)
{
    if (iTk->code == code)
    { // dacă la poziția curentă avem codul cerut, consumăm atomul
        consumedTk = iTk;
        iTk = iTk->next;
        return true;
    }
    return false; // dacă la poziția curentă se află un atom cu un alt cod decât cel cerut, nu are loc nicio acțiune
}

// unit: ( structDef | fnDef | varDef )* END
bool unit()
{
    for (;;)
    {
        if (structDef())
        {
            printf("found struct\n");
            fflush(stdout);
        }
        else if (fnDef())
        {
            printf("found functiondef\n");
            fflush(stdout);
        }
        else if (varDef())
        {
            printf("found vardef\n");
            fflush(stdout);
        }
        else
            break;
    }
    /*if (consume(END))
    {
        return true;
    }
    return false;*/
    return consume(END);
}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(Type *t)
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(TYPE_INT))
    {
        t->tb = TB_INT;
        return true;
    }
    if (consume(TYPE_DOUBLE))
    {
        t->tb = TB_DOUBLE;
        return true;
    }
    if (consume(TYPE_CHAR))
    {
        t->tb = TB_CHAR;
        return true;
    }
    if (consume(STRUCT))
    {
        if (consume(ID))
        {
            t->tb = TB_STRUCT;
            t->s = findSymbol(consumedTk->text);
            if (!t->s)
                tkerr("Undefined struct %s.\n", consumedTk->text);
            return true;
        }
        else
        {
            tkerr("Missing identifier after 'struct'.\n");
        }
    }
    iTk = start;
    consumedTk = last;
    return false;
}
// structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef()
{
    isInStruct = true;
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(STRUCT))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (consume(LACC))
            {
                Symbol *s = findSymbolInDomain(symTable, tkName->text);
                if (s)
                    tkerr("symbol redefinition: %s", tkName->text);
                s = addSymbolToDomain(symTable, newSymbol(tkName->text, SK_STRUCT));
                s->type.tb = TB_STRUCT;
                s->type.s = s;
                s->type.n = -1;
                pushDomain();
                owner = s;
                while (varDef())
                {
                }
                if (consume(RACC))
                {
                    if (consume(SEMICOLON))
                    {
                        owner = NULL;
                        dropDomain();
                        isInStruct = false;
                        return true;
                    }
                    else
                    {
                        tkerr("Missing semicolon after struct definition.\n");
                    }
                }
                else
                {
                    tkerr("Missing '}' after struct definition.\n");
                }
            }
        }
        else
        {
            tkerr("Missing identifier after 'struct'.\n");
        }
    }
    iTk = start;
    consumedTk = last;
    isInStruct = false;
    return false;
}

// varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef()
{
    Token *start = iTk;
    Token *last = consumedTk;
    Type t;
    if (typeBase(&t))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (arrayDecl(&t))
            {
                if (t.n == 0)
                    tkerr("a vector variable must have a specified dimension");
            }
            if (consume(SEMICOLON))
            {
                Symbol *var = findSymbolInDomain(symTable, tkName->text);
                if (var)
                {
                    tkerr("Symbol redefinition: %s", tkName->text);
                }
                var = newSymbol(tkName->text, SK_VAR);
                var->type = t;
                var->owner = owner;
                addSymbolToDomain(symTable, var);
                if (owner)
                {
                    switch (owner->kind)
                    {
                    case SK_FN:
                        var->varIdx = symbolsLen(owner->fn.locals);
                        addSymbolToList(&owner->fn.locals, dupSymbol(var));
                        break;
                    case SK_STRUCT:
                        var->varIdx = typeSize(&owner->type);
                        addSymbolToList(&owner->structMembers, dupSymbol(var));
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    var->varMem = safeAlloc(typeSize(&t));
                }
                return true;
                if (consumedTk->code == RBRACKET)
                {
                    tkerr("Missing ';' after array declaration.\n");
                }
                else
                {
                    tkerr("Missing ';' after variable definition.\n");
                }
            }
            return true;
        }
        else
            tkerr("Missing identifier after typebase.\n");
    }
    else if (isInStruct)
    {
        if (consume(ID))
        {
            arrayDecl(&t);
            if (consume(SEMICOLON))
            {
                tkerr("Missing type of struct member.\n");
            }
        }
    }
    iTk = start;
    consumedTk = last;
    return false;
}

// arrayDecl: LBRACKET INT? RBRACKET
bool arrayDecl(Type *t)
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(LBRACKET))
    {
        if (consume(INT))
        {
            Token *tkSize = consumedTk;
            t->n = tkSize->i;
        }
        else
        {
            t->n = 0; // array fara dimensiune: int v[]
        }
        if (consume(RBRACKET))
        {
            return true;
        }
        else
        {
            tkerr("Missing closing square bracket in array declaration.\n");
        }
    }
    iTk = start;
    consumedTk = last;
    return false;
}

// fnParam: typeBase ID arrayDecl?

bool fnParam()
{
    Token *start = iTk;
    Token *last = consumedTk;
    Type t;
    if (typeBase(&t))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if(arrayDecl(&t)){
                t.n=0;
            }
            Symbol *param=findSymbolInDomain(symTable,tkName->text);
            if(param)tkerr("symbol redefinition: %s",tkName->text);
            param=newSymbol(tkName->text,SK_PARAM);
            param->type=t;
            param->owner=owner;
            param->paramIdx=symbolsLen(owner->fn.params);
            // parametrul este adaugat atat la domeniul curent, cat si la parametrii fn
            addSymbolToDomain(symTable,param);
            addSymbolToList(&owner->fn.params,dupSymbol(param));
            return true;
        }
        tkerr("Missing identifier after parameter type.\n");
    }
    iTk = start;
    consumedTk = last;
    return false;
}

/*fnDef: ( typeBase | VOID ) ID
LPAR ( fnParam ( COMMA fnParam )* )? RPAR
stmCompound*/
bool fnDef()
{
    Type t;
    Token *start = iTk;
    Token *last = consumedTk;

    if (typeBase(&t) || consume(VOID))
    {
        if (consumedTk->code == VOID) {
            t.tb = TB_VOID;
        }
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (consume(LPAR))
            {
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr("symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if (fnParam())
                {
                    while (consume(COMMA))
                    {
                        if (!fnParam())
                        {
                            tkerr("Missing parameter after comma in function definition.\n");
                        }
                    }
                    if (fnParam())
                    {
                        tkerr("Missing comma.\n");
                    }
                }

                if (consume(RPAR))
                {
                    if (stmCompound(false))
                    {
                        dropDomain();
                        owner=NULL;
                        return true;
                    }
                    else
                    {
                        tkerr("Missing compound statement.\n");
                    }
                }
                else
                {
                    tkerr("Missing closing parathesis in function definition or invalid expression.\n");
                }
            }
        }
        else
        {
            tkerr("Missing identifier after return type of function.\n");
        }
    }

    iTk = start;
    consumedTk = last;
    return false;
}

/*stm: stmCompound
| IF LPAR expr RPAR stm ( ELSE stm )?
| WHILE LPAR expr RPAR stm
| RETURN expr? SEMICOLON
| expr? SEMICOLON
*/
bool stm()
{
    Token *start = iTk;
    Token *last = consumedTk;

    if (stmCompound(true))
    {
        return true;
    }

    if (consume(IF))
    {
        if (consume(LPAR))
        {
            if (expr())
            {
                if (consume(RPAR))
                {
                    if (stm())
                    {
                        if (consume(ELSE))
                        {
                            if (stm())
                            {
                                return true;
                            }
                            else
                            {
                                tkerr("Missing statement after 'else'.\n");
                            }
                        }
                        return true;
                    }
                }
                else
                {
                    tkerr("Missing ')' after 'if' condition or invalid condition.\n");
                }
            }
        }
        else
        {
            tkerr("Missing '(' after 'if' condition.\n");
        }
    }

    if (consume(WHILE))
    {
        if (consume(LPAR))
        {
            if (expr())
            {
                if (consume(RPAR))
                {
                    if (stm())
                    {
                        return true;
                    }
                    else
                    {
                        tkerr("Missing statement after 'while' condition.\n");
                    }
                }
                else
                {
                    tkerr("Missing ')' after 'while' condition.\n");
                }
            }
        }
    }

    if (consume(RETURN))
    {
        expr();
        if (consume(SEMICOLON))
        {
            return true;
        }
        else
        {
            tkerr("Missing semicolon after 'return' statement.\n");
        }
    }

    if (expr())
    {
        if (consume(SEMICOLON))
        {
            return true;
        }
        else
            tkerr("Missing semicolon after expression.\n");
    }
    if (consume(SEMICOLON))
    {
        return true;
    }
    iTk = start;
    consumedTk = last;
    return false;
}


bool stm();
// stmCompound: LACC ( varDef | stm )* RACC

bool stmCompound(bool newDomain)
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(LACC))
    {
        if(newDomain)pushDomain();
        while (varDef() || stm())
        {
        }
        if (consume(RACC))
        {
            if(newDomain)dropDomain();
            return true;
        }
        else
        {
            tkerr("Missing closing curly brace '}' in compound statement.\n");
        }
    }

    iTk = start;
    consumedTk = last;
    return false;
}

// expr: exprAssign
bool expr()
{
    if (exprAssign())
    {
        return true;
    }
    return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (exprUnary())
    {
        if (consume(ASSIGN))
        {
            if (exprAssign())
            {
                return true;
            }
            else
                tkerr("Missing or invalid expression after '='.\n");
        }
    }
    iTk = start;
    consumedTk = last;

    if (exprOr())
    {
        return true;
    }

    iTk = start;
    consumedTk = last;
    return false;
}

// exprOrPrim: OR exprAnd exprOrPrim | ε
bool exprOrPrim()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(OR))
    {
        if (exprAnd())
        {
            if (exprOrPrim())
            {
                return true;
            }
        }
        else
            tkerr("missing expression after '||'");
    }

    iTk = start;
    consumedTk = last;
    return true;
}

// exprOr: exprAnd exprOrPrim
bool exprOr()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (exprAnd())
    {
        if (exprOrPrim())
        {
            return true;
        }
    }

    iTk = start;
    consumedTk = last;
    return false;
}

// exprAndPrim: AND exprEq exprAndPrim | ε
bool exprAndPrim()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(AND))
    {
        if (exprEq())
        {
            if (exprAndPrim())
            {
                return true;
            }
        }
        else
            tkerr("missing expression after '&&'");
    }

    iTk = start;
    consumedTk = last;
    return true;
}

// exprAnd: exprEq exprAndPrim
bool exprAnd()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (exprEq())
    {
        if (exprAndPrim())
        {
            return true;
        }
    }

    iTk = start;
    consumedTk = last;
    return false;
}

// exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | ε
bool exprEqPrim()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(EQUAL) || consume(NOTEQ))
    {
        if (exprRel())
        {
            if (exprEqPrim())
            {
                return true;
            }
        }
        else
        {
            if (consumedTk->code == EQUAL)
            {
                tkerr("missing expression after '=='");
            }
            else if (consumedTk->code == NOTEQ)
            {
                tkerr("missing expression after '!='");
            }
        }
    }
    iTk = start;
    consumedTk = last;
    return true;
}

// exprEq: exprRel exprEqPrim
bool exprEq()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (exprRel())
    {
        if (exprEqPrim())
        {
            return true;
        }
    }
    iTk = start;
    consumedTk = last;
    return false;
}

// exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | ε
bool exprRelPrim()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ))
    {
        if (exprAdd())
        {
            if (exprRelPrim())
            {
                return true;
            }
        }
        else
        {
            if (consumedTk->code == LESS)
            {
                tkerr("missing expression after '<'");
            }
            else if (consumedTk->code == LESSEQ)
            {
                tkerr("missing expression after '<='");
            }
            else if (consumedTk->code == GREATER)
            {
                tkerr("missing expression after '>'");
            }
            else if (consumedTk->code == GREATEREQ)
            {
                tkerr("missing expression after '>='");
            }
        }
    }
    iTk = start;
    consumedTk = last;
    return true;
}

// exprRel: exprAdd exprRelPrim
bool exprRel()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (exprAdd())
    {
        if (exprRelPrim())
        {
            return true;
        }
    }
    iTk = start;
    consumedTk = last;
    return false;
}

// exprAddPrim: ( ADD | SUB ) exprMul exprAddPrim | ε
bool exprAddPrim()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(ADD) || consume(SUB))
    {
        if (exprMul())
        {
            if (exprAddPrim())
            {
                return true;
            }
        }
        else
        {
            if (consumedTk->code == ADD)
            {
                tkerr("missing expression after '+'");
            }
            else if (consumedTk->code == SUB)
            {
                tkerr("missing expression after '-'");
            }
        }
    }
    iTk = start;
    consumedTk = last;
    return true;
}

// exprAdd: exprMul exprAddPrim
bool exprAdd()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (exprMul())
    {
        if (exprAddPrim())
        {
            return true;
        }
    }
    iTk = start;
    consumedTk = last;
    return false;
}

// exprMulPrim: ( MUL | DIV ) exprCast exprMulPrim | ε
bool exprMulPrim()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(MUL) || consume(DIV))
    {
        if (exprCast())
        {
            if (exprMulPrim())
            {
                return true;
            }
        }
        else
        {
            if (consumedTk->code == MUL)
            {
                tkerr("missing expression after '*'");
            }
            else if (consumedTk->code == DIV)
            {
                tkerr("missing expression after '/'");
            }
        }
    }
    iTk = start;
    consumedTk = last;
    return true;
}

// exprMul: exprCast exprMulPrim
bool exprMul()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (exprCast())
    {
        if (exprMulPrim())
        {
            return true;
        }
    }
    iTk = start;
    consumedTk = last;
    return false;
}

// exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(LPAR))
    {
        Type t;
        if (typeBase(&t))
        {
            if (arrayDecl(&t))
            {
            }
            if (consume(RPAR))
            {
                if (exprCast())
                {
                    return true;
                }
                else
                    tkerr("missing expression after casting");
            }
            else
                tkerr("missing ')' after type in casting");
        }
        else
            tkerr("missing type after '(' in casting");
    }
    iTk = start;
    consumedTk = last;
    if (exprUnary())
    {
        return true;
    }
    iTk = start;
    consumedTk = last;
    return false;
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(SUB) || consume(NOT))
    {
        if (exprUnary())
        {
            return true;
        }
        else
        {
            if (consumedTk->code == SUB)
            {
                tkerr("missing expression after '-");
            }
            else if (consumedTk->code == NOT)
            {
                tkerr("missing expression after '!");
            }
        }
    }
    iTk = start;
    consumedTk = last;
    if (exprPostfix())
    {
        return true;
    }
    iTk = start;
    consumedTk = last;
    return false;
}

/* exprPostfixPrim: LBRACKET expr RBRACKET exprPostfixPrim
        | DOT ID exprPostfixPrim
        | ε
*/
bool exprPostfixPrim()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(LBRACKET))
    {
        if (expr())
        {
            if (consume(RBRACKET))
            {
                if (exprPostfixPrim())
                {
                    return true;
                }
            }
            else
                tkerr("Missing ']' after '[' in array indexing");
        }
        else
            tkerr("missing expression after array indexing");
    }
    iTk = start;
    consumedTk = last;
    if (consume(DOT))
    {
        if (consume(ID))
        {
            if (exprPostfixPrim())
            {
                return true;
            }
        }
        else
            tkerr("missing identifier after '.' for struct member access");
    }
    iTk = start;
    consumedTk = last;
    return true;
}

// exprPostfix: exprPrimary exprPostfixPrim
bool exprPostfix()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (exprPrimary())
    {
        if (exprPostfixPrim())
        {
            return true;
        }
    }
    iTk = start;
    consumedTk = last;
    return false;
}

/* exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
| INT | DOUBLE | CHAR | STRING | LPAR expr RPAR */
bool exprPrimary()
{
    Token *start = iTk;
    Token *last = consumedTk;
    if (consume(ID))
    {
        if (consume(LPAR))
        {
            if (expr())
            {
                for (;;)
                {
                    if (consume(COMMA))
                    {
                        if (expr())
                        {
                        }
                        else
                            tkerr("Missing expression after ',' in function argument list");
                    }
                    else
                        break;
                }
            }
            if (consume(RPAR))
            {
            }
            else
                tkerr("Missing ')' after '(' in function call");
        }
        return true;
    }
    if (consume(INT))
    {
        return true;
    }
    if (consume(DOUBLE))
    {
        return true;
    }
    if (consume(CHAR))
    {
        return true;
    }
    if (consume(STRING))
    {
        return true;
    }
    if (consume(LPAR))
    {
        if (expr())
        {
            if (consume(RPAR))
            {
                return true;
            }
            else
                tkerr("missing closing ')' after expression");
        }
        else
            tkerr("missing expression after '('");
    }
    iTk = start;
    consumedTk = last;
    return false;
}

void parse(Token *tokens)
{
    iTk = tokens;
    pushDomain();
    if (!unit())
        tkerr("syntax error");
}
