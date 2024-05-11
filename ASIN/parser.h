#pragma once


#include "/home/patricia/LFTC/COMPILER/ALEX/lexer.h"
#include <stdbool.h>
#include "/home/patricia/LFTC/COMPILER/AD/ad.h"

void parse(Token *tokens);
bool typeBase(Type *t);
bool stm();
bool expr();
bool exprAssign();
bool exprPrimary();
bool stmCompound(bool newDomain);
bool fnDef();
bool unit();
bool structDef();
bool arrayDecl(Type *t);
bool varDef();
bool fnParam();
bool exprOrPrim();
bool exprOr();
bool exprAndPrim();
bool exprAnd();
bool exprEqPrim();
bool exprEq();
bool exprRelPrim();
bool exprRel();
bool exprAddPrim();
bool exprAdd();
bool exprMulPrim();
bool exprMul();
bool exprCast();
bool exprUnary();
bool exprPostfixPrim();
bool exprPostfix();

