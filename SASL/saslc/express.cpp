
/*  o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o

    CTool Library
    Copyright (C) 1998-2001	Shaun Flisakowski

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o  */
/*  o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    o+
    o+     File:         express.cpp
    o+
    o+     Programmer:   Shaun Flisakowski
    o+     Date:         Aug 9, 1998
    o+
    o+     A high-level view of expressions.
    o+
    o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o  */

#include <cstdio>
#include <cstring>
#include <cassert>
#include <stdlib.h>

#include <ctool/config.h>
#include <ctool/express.h>
#include <ctool/decl.h>

#include "gram.h"
#include <ctool/token.h>

//#define SHOW_TYPES

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
/* Print a char, converting chars to escape sequences. */
static
std::ostream&
MetaChar(std::ostream& out, char c, bool inString)
{
    switch (c)
    {
      case '\'':
        if (inString) 
          out << "'";
        else
          out << "\\'";
        break;

      case '"':
        if (inString) 
          out << "\\\"";
        else
          out << "\"";
        break;

      case '\0':
        out << "\\0";
        break;
      case '\\':
        out << "\\\\";
        break;
      case '\n':
        out << "\\n";
        break;
      case '\t':
        out << "\\t";
        break;
      case '\r':
        out << "\\r";
        break;
      case '\f':
        out << "\\f";
        break;
      case '\b':
        out << "\\b";
        break;
      case '\v':
        out << "\\v";
        break;
      case '\a':
        out << "\\a";
        break;
      case ESC_VAL:
        out << "\\e";
        break;

      default:
        // Show low and high ascii as octal
        if ((c < ' ') || (c >= 127))
        {
            char octbuf[8];
            sprintf(octbuf, "%03o", (unsigned char) c);
            out << "\\" << octbuf;
        }
        else
            out << c;
        break;
    }

    return out;
}

/*  ###############################################################  */
/* Print a string, converting chars to escape sequences. */
std::ostream&
MetaString(std::ostream& out, const std::string &string)
{
    for(unsigned i=0; i<string.size(); i++)
      MetaChar(out, string[i], true);

    return out;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
PrintAssignOp(std::ostream& out, AssignOp op)
{
    switch (op)
    {
      case AO_Equal:          //  =
        out << "=";
        break;

      case AO_PlusEql:        // +=
        out << "+=";
        break;

      case AO_MinusEql:       // -=
        out << "-=";
        break;

      case AO_MultEql:        // *=
        out << "*=";
        break;

      case AO_DivEql:         // /=
        out << "/=";
        break;

      case AO_ModEql:         // %=
        out << "%=";
        break;

      case AO_ShlEql:         // <<=
        out << "<<=";
        break;

      case AO_ShrEql:         // >>=
        out << ">>=";
        break;

      case AO_BitAndEql:      // &=
        out << "&=";
        break;

      case AO_BitXorEql:      // ^=
        out << "^=";
        break;

      case AO_BitOrEql:       // |=
        out << "|=";
        break;
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
PrintRelOp(std::ostream& out, RelOp op)
{
    switch (op)
    {
      case RO_Equal:          // ==
        out << "==";
        break;

      case RO_NotEqual:       // !=
        out << "!=";
        break;

      case RO_Less:           // < 
        out << "<";
        break;

      case RO_LessEql:        // <=
        out << "<=";
        break;

      case RO_Grtr:           // > 
        out << ">";
        break;

      case RO_GrtrEql:        // >=
        out << ">=";
        break;
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
PrintUnaryOp(std::ostream& out, UnaryOp op)
{
    switch (op)
    {
      case UO_Plus:           // +
        out << "+";
        break;

      case UO_Minus:          // -
        out << "-";
        break;

      case UO_BitNot:         // ~
        out << "~";
        break;

      case UO_Not:            // !
        out << "!";
        break;

      case UO_PreInc:         // ++x
      case UO_PostInc:        // x++
        out << "++";
        break;

      case UO_PreDec:         // --x
      case UO_PostDec:        // x--
        out << "--";
        break;

      case UO_AddrOf:         // &
        out << "&";
        break;

      case UO_Deref:          // *
        out << "*";
        break;
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
PrintBinaryOp(std::ostream& out, BinaryOp op)
{
    switch (op)
    {
      case BO_Plus:        // +
        out << "+";
        break;

      case BO_Minus:       // -
        out << "-";
        break;

      case BO_Mult:        // *
        out << "*";
        break;

      case BO_Div:         // /
        out << "/";
        break;

      case BO_Mod:         // %
        out << "%";
        break;

      case BO_Shl:         // <<
        out << "<<";
        break;

      case BO_Shr:         // >>
        out << ">>";
        break;

      case BO_BitAnd:      // &
        out << "&";
        break;

      case BO_BitXor:      // ^
        out << "^";
        break;

      case BO_BitOr:       // |
        out << "|";
        break;

      case BO_And:         // &&
        out << "&&";
        break;

      case BO_Or:          // ||
        out << "||";
        break;

      case BO_Comma:       // x,y
        out << ",";
        break;

      case BO_Member:      // x.y
        out << ".";
        break;

      case BO_PtrMember:   // x->y
        out << "->";
        break;

      default:
      //  case BO_Index        // x[y]
      case BO_Assign:      // An AssignExpr
      case BO_Rel:         // A RelExpr
        break;
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression::Expression(ExpressionType et, const Location& l )
           :location(l)
{
    etype = et;
    type = NULL;
    next = NULL;
}

Expression::~Expression()
{
    //delete type;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
Expression::print(std::ostream& out) const
{
    out << __PRETTY_FUNCTION__ << std::endl;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Constant::Constant(ConstantType ct, const Location& l)
         : Expression (ET_Constant, l)
{
    ctype = ct;
}
 
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Constant::~Constant()
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
IntConstant::IntConstant(long val, bool b, const Location& l )
            : Constant (CT_Int, l)
{
    lng = val;
    L = b;
    type = new BaseType(BT_Int);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
IntConstant::~IntConstant()
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
IntConstant::dup0() const
{
    IntConstant *ret = new IntConstant(lng, L, location);
    ret->type = type;    
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
IntConstant::print(std::ostream& out) const
{
    out << lng;
    if (L) out << "L";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
UIntConstant::UIntConstant(ulong val, bool b, const Location& l )
            : Constant (CT_UInt, l)
{
    ulng = val;
    L = b;
    type = new BaseType(BT_Int | BT_UnSigned);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
UIntConstant::~UIntConstant()
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
UIntConstant::dup0() const
{
    UIntConstant *ret = new UIntConstant(ulng, L, location);
    ret->type = type;    
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
UIntConstant::print(std::ostream& out) const
{
    out << ulng;
    if (L) out << "L";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
FloatConstant::FloatConstant(const std::string& val, const Location& l )
              : Constant (CT_Float, l)
{
    value = val;
    type = new BaseType(BT_Float);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
FloatConstant::~FloatConstant()
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
FloatConstant::dup0() const
{
    FloatConstant *ret = new FloatConstant(value, location);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
FloatConstant::print(std::ostream& out) const
{
    out << value;

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
CharConstant::CharConstant(char chr, const Location& l,
			   bool isWide /* =false */ )
  :Constant(CT_Char, l)
{
    ch = chr;
    wide = isWide;
    type = new BaseType(BT_Char);
}

CharConstant::~CharConstant()
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
CharConstant::dup0() const
{
    CharConstant *ret = new CharConstant(ch,location,wide);
    ret->type = type;
    return ret;
}
void
CharConstant::print(std::ostream& out) const
{
    if (wide)
        out << 'L';

    out << "'";
    MetaChar(out,ch,false);
    out << "'";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
StringConstant::StringConstant(const std::string &str,
                               const Location& l,
			       bool isWide /* =false */ )
               : Constant(CT_String, l), buff(str)
{
    wide = isWide;

    // Or should this be const char*?
    PtrType *ptrType = new PtrType();
    type = ptrType;
    ptrType->subType = new BaseType(BT_Char);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
StringConstant::~StringConstant()
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
int
StringConstant::length() const
{
    return buff.size();
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
StringConstant::dup0() const
{
    StringConstant *ret = new StringConstant(buff,location,wide);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
StringConstant::print(std::ostream& out) const
{
    if (wide)
        out << 'L';

    out << '"';
    MetaString(out,buff);
    out << '"';

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
ArrayConstant::ArrayConstant(const Location& l)
              : Constant(CT_Array, l)
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
ArrayConstant::~ArrayConstant()
{
    ExprVector::iterator    j;

    for (j=items.begin(); j != items.end(); j++)
    {
        delete *j;
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
ArrayConstant::addElement( Expression *expr )
{
    items.push_back(expr);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
ArrayConstant::dup0() const
{
    ArrayConstant *ret = new ArrayConstant(location);

    ExprVector::const_iterator    j;
    for (j=items.begin(); j != items.end(); j++)
    {
        ret->addElement((*j)->dup());
    }

    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
ArrayConstant::print(std::ostream& out) const
{
    out <<  "{ ";

    if (!items.empty())
    {
        ExprVector::const_iterator    j = items.begin();

        (*j)->print(out);

        for (j++; j != items.end(); j++)
        {
            out << ", ";
            (*j)->print(out);
        }
    }

    out <<  " }";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
ArrayConstant::findExpr( fnExprCallback cb )
{
    (cb)(this);

    ExprVector::const_iterator    j;

    for (j =items.begin(); j != items.end(); j++)
    {
        (*j)->findExpr(cb);
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
EnumConstant::EnumConstant(Symbol* nme, Expression* val,
                           const Location& l)
               : Constant(CT_Enum, l) 
{
    name = nme;
    value = val;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
EnumConstant::~EnumConstant()
{
    delete name;
    delete value;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
EnumConstant::dup0() const
{
    EnumConstant *ret = new EnumConstant(name->dup(),value->dup(),location);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
EnumConstant::print(std::ostream& out) const
{
    out << *name;

    if (value)
    {
        out << " = " << *value;
    }

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Variable::Variable(Symbol *varname, const Location& l)
         : Expression( ET_Variable, l)
{
    name = varname;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Variable::~Variable()
{
    delete name;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
Variable::dup0() const
{
    Variable *ret = new Variable(name->dup(), location);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
Variable::print(std::ostream& out) const
{
    out << *name;

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
FunctionCall::FunctionCall( Expression *func, const Location& l)
             : Expression( ET_FunctionCall, l)
{
    function = func;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
FunctionCall::~FunctionCall()
{
    delete function;

    ExprVector::iterator    j;

    for (j=args.begin(); j != args.end(); j++)
        delete *j;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
FunctionCall::addArg( Expression *arg )
{
    args.push_back(arg);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
FunctionCall::addArgs( Expression *argList )
{
    Expression    *arg = argList;

    while (argList != NULL)
    {
        argList = argList->next;
        arg->next = NULL;
        addArg(arg);
        arg = argList; 
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
FunctionCall::dup0() const
{
    FunctionCall *ret = new FunctionCall(function->dup(), location);

    ExprVector::const_iterator    j;
    for (j=args.begin(); j != args.end(); j++)
    {
        ret->addArg((*j)->dup());
    }
    
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
FunctionCall::print(std::ostream& out) const
{
    if (function->precedence() < precedence())
    {
        out << "(";
        function->print(out);
        out << ")";
    }
    else
        function->print(out);

    out << "(";

    if (!args.empty())
    {
        ExprVector::const_iterator    j = args.begin();

        if (((*j)->etype == ET_BinaryExpr)
                && (((BinaryExpr*)(*j))->op() == BO_Comma))
        {
            out << "(";
            (*j)->print(out);
            out << ")";
        }
        else
            (*j)->print(out);

        for (j++; j != args.end(); j++)
        {
            out << ",";

            if (((*j)->etype == ET_BinaryExpr)
                    && (((BinaryExpr*)(*j))->op() == BO_Comma))
            {
                out << "(";
                (*j)->print(out);
                out << ")";
            }
            else
                (*j)->print(out);
        }
    }

    out << ")";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
FunctionCall::findExpr( fnExprCallback cb )
{
    (cb)(this);

    function->findExpr(cb);

    ExprVector::iterator    j = args.begin();

    for (j = args.begin(); j != args.end(); j++)
    {
        (*j)->findExpr(cb);
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
UnaryExpr::UnaryExpr( UnaryOp _op, Expression *expr, const Location& l)
          : Expression( ET_UnaryExpr, l)
{
    uOp = _op;
    _operand = expr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
UnaryExpr::~UnaryExpr()
{
    delete _operand;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
UnaryExpr::dup0() const
{
    UnaryExpr *ret = new UnaryExpr(uOp,_operand->dup(), location);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
UnaryExpr::print(std::ostream& out) const
{
    switch (uOp)
    {
      default:
        PrintUnaryOp(out,uOp);

        if (operand()->precedence() < precedence())
            out << "(" << *operand() << ")";
        else
            out << *operand();
        break;

      case UO_Minus:
        PrintUnaryOp(out,uOp);
        if ( (operand()->precedence() < precedence())
            || ((operand()->etype == ET_UnaryExpr)
                && (((UnaryExpr*) operand())->op() == UO_Minus)) )
            out << "(" << *operand() << ")";
        else
            out << *operand();
        break;

      case UO_PostInc:
      case UO_PostDec:
        if (operand()->precedence() < precedence())
            out << "(" << *operand() << ")";
        else
            out << *operand();

        PrintUnaryOp(out,uOp);
        break;
    }

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
int
UnaryExpr::precedence() const
{
    switch (uOp)
    {
      case UO_Plus:           // +
      case UO_Minus:          // -
      case UO_BitNot:         // ~
      case UO_Not:            // !

      case UO_PreInc:         // ++x
      case UO_PreDec:         // --x

      case UO_AddrOf:         // &
      case UO_Deref:          // *
        return 15;

      case UO_PostInc:        // x++
      case UO_PostDec:        // x--
        return 16;
    }

    /* Not reached */
    return 16;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
UnaryExpr::findExpr( fnExprCallback cb )
{
    (cb)(this);

    _operand->findExpr(cb);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BinaryExpr::BinaryExpr( BinaryOp _op, Expression *lExpr, Expression *rExpr,
			const Location& l)
           : Expression( ET_BinaryExpr, l)
{
    bOp = _op;
    _leftExpr  = lExpr;
    _rightExpr = rExpr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BinaryExpr::~BinaryExpr()
{
    delete _leftExpr;
    delete _rightExpr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
BinaryExpr::dup0() const
{
    BinaryExpr *ret = new BinaryExpr(bOp,_leftExpr->dup(),_rightExpr->dup(), location);
    ret->type = type;
    
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BinaryExpr::print(std::ostream& out) const
{
    assert(leftExpr());

    if (leftExpr()->precedence() < precedence())
        out << "(" << *leftExpr() << ")";
    else
        out << *leftExpr();

    bool useSpace = !((bOp == BO_Member) || (bOp == BO_PtrMember));

    if (useSpace)
        out << " ";

    PrintBinaryOp(out,bOp);

    if (useSpace)
        out << " ";

    assert(rightExpr());

    if ( (rightExpr()->precedence() < precedence())
         || ( (rightExpr()->precedence() == precedence())
              && (rightExpr()->etype != ET_Variable)) )
    {
        out << "(" << *rightExpr() << ")";
    }
    else
        out << *rightExpr();

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
int
BinaryExpr::precedence() const
{
    switch (bOp)
    {
      case BO_Plus:        // +
      case BO_Minus:       // -
        return 12;

      case BO_Mult:        // *
      case BO_Div:         // /
      case BO_Mod:         // %
        return 13;

      case BO_Shl:         // <<
      case BO_Shr:         // >>
        return 11;

      case BO_BitAnd:      // &
        return 8;

      case BO_BitXor:      // ^
        return 7;

      case BO_BitOr:       // |
        return 6;

      case BO_And:         // &&
        return 5;

      case BO_Or:          // ||
        return 4;

      case BO_Comma:       // x,y
        return 1;

      case BO_Member:      // x.y
      case BO_PtrMember:   // x->y
        return 16;

      case BO_Assign:      // An AssignExpr
      case BO_Rel:         // A RelExpr
        break;
    }

    return 1;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BinaryExpr::findExpr( fnExprCallback cb )
{
    (cb)(this);

    _leftExpr->findExpr(cb);
    _rightExpr->findExpr(cb);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
TrinaryExpr::TrinaryExpr( Expression *cExpr,
                          Expression *tExpr, Expression *fExpr,
			  const Location& l )
            : Expression( ET_TrinaryExpr, l)
{
    _condExpr  = cExpr;
    _trueExpr  = tExpr;
    _falseExpr = fExpr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
TrinaryExpr::~TrinaryExpr()
{
    delete _condExpr;
    delete _trueExpr;
    delete _falseExpr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
TrinaryExpr::dup0() const
{
    TrinaryExpr *ret = new TrinaryExpr(_condExpr->dup(),
                                       _trueExpr->dup(),
                                       _falseExpr->dup(), location);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
TrinaryExpr::print(std::ostream& out) const
{
    out << "(" << *condExpr() << ")";

    out << " ? ";
    out << "(" << *trueExpr() << ")";

    out << " : ";
    out << "(" << *falseExpr() << ")";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
TrinaryExpr::findExpr( fnExprCallback cb )
{
    (cb)(this);

    _condExpr->findExpr(cb);
    _trueExpr->findExpr(cb);
    _falseExpr->findExpr(cb);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
AssignExpr::AssignExpr( AssignOp _op, Expression *lExpr, Expression *rExpr,
			const Location& l )
           : BinaryExpr( BO_Assign, lExpr, rExpr, l )
{
    aOp = _op;
    type = lExpr->type;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
AssignExpr::~AssignExpr()
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
AssignExpr::dup0() const
{
    AssignExpr *ret = new AssignExpr(aOp,_leftExpr->dup(),_rightExpr->dup(), location);
    ret->type = type;    
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
AssignExpr::print(std::ostream& out) const
{
    if (lValue()->precedence() < precedence())
        out << "(" << *lValue() << ")";
    else
        out << *lValue();

    out << " ";
    PrintAssignOp(out,aOp);
    out << " ";

    if (rValue()->precedence() < precedence())
        out << "(" << *rValue() << ")";
    else
        out << *rValue();

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
RelExpr::RelExpr( RelOp _op, Expression *lExpr, Expression *rExpr,
		  const Location& l )
        : BinaryExpr( BO_Rel, lExpr, rExpr, l )
{
    rOp = _op;
    type = new BaseType(BT_Int);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
RelExpr::~RelExpr()
{
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
RelExpr::dup0() const
{
    RelExpr *ret = new RelExpr(rOp,_leftExpr->dup(),_rightExpr->dup(), location);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
RelExpr::print(std::ostream& out) const
{
    if (leftExpr()->precedence() <= precedence())
        out << "(" << *leftExpr() << ")";
    else
        out << *leftExpr();

    out << " ";
    PrintRelOp(out,rOp);
    out << " ";

    if (rightExpr()->precedence() <= precedence())
        out << "(" << *rightExpr() << ")";
    else
        out << *rightExpr();

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
int
RelExpr::precedence() const
{
    switch (rOp)
    {
      case RO_Equal:
      case RO_NotEqual:
        return 9;

      default:
      case RO_Less:
      case RO_LessEql:
      case RO_Grtr:
      case RO_GrtrEql:
        return 10;
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
CastExpr::CastExpr(Type *typeExpr, Expression *operand,
                   const Location& l )
         : Expression( ET_CastExpr, l )
{
    castTo = typeExpr;
    expr = operand;
    type = typeExpr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
CastExpr::~CastExpr()
{
    //delete castTo;
    delete expr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
CastExpr::dup0() const
{
    CastExpr *ret = new CastExpr(castTo,expr->dup(), location);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
CastExpr::print(std::ostream& out) const
{
    out << "(";
    castTo->printType(out,NULL,true,0); 
    out << ") ";

    out << "(";
    expr->print(out);
    out << ")";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
CastExpr::findExpr( fnExprCallback cb )
{
    (cb)(this);

    expr->findExpr(cb);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
SizeofExpr::SizeofExpr( Expression *operand, const Location& l )
         : Expression( ET_SizeofExpr, l )
{
    sizeofType = NULL;
    expr = operand;
    type = new BaseType(BT_UnSigned|BT_Long);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
SizeofExpr::SizeofExpr( Type *operand, const Location& l )
         : Expression( ET_SizeofExpr, l )
{
    sizeofType = operand;
    expr = NULL;
    type = new BaseType(BT_UnSigned|BT_Long);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
SizeofExpr::~SizeofExpr()
{
    // delete sizeofType;
    delete expr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
SizeofExpr::dup0() const
{
    SizeofExpr *ret ;
    
    if (sizeofType != NULL)
        ret = new SizeofExpr(sizeofType, location);
    else
        ret = new SizeofExpr(expr->dup(), location);
    
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
SizeofExpr::print(std::ostream& out) const
{
    out << "sizeof(";
    if (sizeofType != NULL)
        sizeofType->printType(out,NULL,true,0); 
    else
        expr->print(out);
    out << ") ";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
SizeofExpr::findExpr( fnExprCallback cb )
{
    (cb)(this);

    if (expr != NULL)
        expr->findExpr(cb);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
IndexExpr::IndexExpr( Expression *_array, Expression *sub,
                      const Location& l )
          : Expression(ET_IndexExpr, l)
{
    array     = _array;
    _subscript = sub;
}

IndexExpr::~IndexExpr()
{
    delete array;
    delete _subscript;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
IndexExpr::dup0() const
{
    IndexExpr *ret = new IndexExpr(array->dup(),_subscript->dup(), location);
    ret->type = type;
    return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
IndexExpr::print(std::ostream& out) const
{
    if (array->precedence() < precedence())
    {
        out << "(";
        array->print(out);
        out << ")";
    }
    else
        array->print(out);

    out << "[";
    _subscript->print(out);
    out << "]";

#ifdef    SHOW_TYPES
    if (type != NULL)
    {
        out << "/* ";
        type->printType(out,NULL,true,0);
        out << " */";
    }
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
IndexExpr::findExpr( fnExprCallback cb )
{
    (cb)(this);

    array->findExpr(cb);
    _subscript->findExpr(cb);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
std::ostream&
operator<< (std::ostream& out, const Expression& expr)
{
    expr.print(out);
    return out;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression*
ReverseList( Expression* eList )
{
    Expression*    head = NULL;

    while (eList != NULL)
    {
        Expression*    exp = eList;

        eList = eList->next;

        exp->next = head;
        head = exp;
    }

    return head; 
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
#define    SHOW(X)    case X: return #X
char*
nameOfExpressionType( ExpressionType type )
{
    switch (type)
    {
        default:
            return "Unknown ExpressionType";

        SHOW(ET_VoidExpr);

        SHOW(ET_Constant);
        SHOW(ET_Variable);
        SHOW(ET_FunctionCall);

        SHOW(ET_AssignExpr);
        SHOW(ET_RelExpr);

        SHOW(ET_UnaryExpr);
        SHOW(ET_BinaryExpr);
        SHOW(ET_TrinaryExpr);

        SHOW(ET_CastExpr);
        SHOW(ET_IndexExpr);
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
char*
nameOfConstantType(ConstantType type)
{
    switch (type)
    {
        default:
            return "Unknown ConstantType";

        SHOW(CT_Char);
        SHOW(CT_Int);
        SHOW(CT_Float);

        SHOW(CT_String);
        SHOW(CT_Array);
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
char*
nameOfAssignOp(AssignOp op)
{
    switch (op)
    {
        default:
            return "Unknown AssignOp";

        SHOW(AO_Equal);          //  =

        SHOW(AO_PlusEql);        // +=
        SHOW(AO_MinusEql);       // -=
        SHOW(AO_MultEql);        // *=
        SHOW(AO_DivEql);         // /=
        SHOW(AO_ModEql);         // %=

        SHOW(AO_ShlEql);         // <<=
        SHOW(AO_ShrEql);         // >>=

        SHOW(AO_BitAndEql);      // &=
        SHOW(AO_BitXorEql);      // ^=
        SHOW(AO_BitOrEql);       // |=
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
char*
nameOfRelOp(RelOp op)
{
    switch (op)
    {
        default:
            return "Unknown RelOp";

        SHOW(RO_Equal);          // ==
        SHOW(RO_NotEqual);       // !=

        SHOW(RO_Less);           // < 
        SHOW(RO_LessEql);        // <=
        SHOW(RO_Grtr);           // > 
        SHOW(RO_GrtrEql);        // >=
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
char*
nameOfUnaryOp(UnaryOp op)
{
    switch (op)
    {
        default:
            return "Unknown UnaryOp";

        SHOW(UO_Plus);           // +
        SHOW(UO_Minus);          // -
        SHOW(UO_BitNot);         // ~
        SHOW(UO_Not);            // !

        SHOW(UO_PreInc);         // ++x
        SHOW(UO_PreDec);         // --x
        SHOW(UO_PostInc);        // x++
        SHOW(UO_PostDec);        // x--

        SHOW(UO_AddrOf);         // &
        SHOW(UO_Deref);          // *
    }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
char*
nameOfBinaryOp(BinaryOp op)
{
    switch (op)
    {
        default:
            return "Unknown BinaryOp";

        SHOW(BO_Plus);        // +
        SHOW(BO_Minus);       // -
        SHOW(BO_Mult);        // *
        SHOW(BO_Div);         // /
        SHOW(BO_Mod);         // %

        SHOW(BO_Shl);         // <<
        SHOW(BO_Shr);         // >>
        SHOW(BO_BitAnd);      // &
        SHOW(BO_BitXor);      // ^
        SHOW(BO_BitOr);       // |

        SHOW(BO_And);         // &&
        SHOW(BO_Or);          // ||

        SHOW(BO_Comma);       // x,y

        SHOW(BO_Member);      // x.y
        SHOW(BO_PtrMember);   // x->y

        SHOW(BO_Assign);      // An AssignExpr
        SHOW(BO_Rel);         // A RelExpr
    }
}

#undef SHOW
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
