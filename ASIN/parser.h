#pragma once

#include <stdbool.h>
#include "../ALEX/lexer.h"
#include "../AD/ad.h"
#include "../AT/at.h"

void parse(Token *tokens);
bool typeBase(Type *t);
bool stm();
bool expr(Ret *r);
bool exprAssign(Ret *r);
bool exprPrimary(Ret *r);
bool stmCompound(bool newDomain);
bool fnDef();
bool unit();
bool structDef();
bool arrayDecl(Type *t);
bool varDef();
bool fnParam();
bool exprOrPrim(Ret *r);
bool exprOr(Ret *r);
bool exprAndPrim(Ret *r);
bool exprAnd(Ret *r);
bool exprEqPrim(Ret *r);
bool exprEq(Ret *r);
bool exprRelPrim(Ret *r);
bool exprRel(Ret *r);
bool exprAddPrim(Ret *r);
bool exprAdd(Ret *r);
bool exprMulPrim(Ret *r);
bool exprMul(Ret *r);
bool exprCast(Ret *r);
bool exprUnary(Ret *r);
bool exprPostfixPrim(Ret *r);
bool exprPostfix(Ret *r);

