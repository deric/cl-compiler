/* ===== Cl.g: parser para el lenguaje fuente CL de Compiladors ===== */
#header
<<
#include <string>
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <fstream>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include "ptype.hh"
#include "symtab.hh"
#include "codegest.hh"

/// struct to contain information about a token.
typedef struct {
    string kind;
    string text;
    int line;
} Attrib;

/// function called by the scanner when a new token is found
/// Predeclared here, definition below.
void zzcr_attr(Attrib *attr,int type,char *text);

/// Include AST node fields declaration
#include "myASTnode.hh"

/// Macro called by the parser when a new AST node is to be created
/// createASTnode function doing the actual job is defined below.
#define zzcr_ast(as,attr,tttype,textt) as=createASTnode(attr,tttype,textt);
AST* createASTnode(Attrib* attr, int ttype, char *textt);
>>


<<
#include "semantic.hh"
#include "codegen.hh"
#include "util.hh"

/// -----------------------------------------------------------
/// function called by the scanner when a new token is found.
void zzcr_attr(Attrib *attr,int type,char *text)
{
  switch (type) {
  case IDENT:
    attr->kind="ident";
    attr->text=text;
    break;
  case INTCONST:
    attr->kind="intconst";
    attr->text=text;
    break;
  default:
    attr->kind=lowercase(text);
    attr->text="";
    break;
  }
  attr->line = zzline;
  if (type!=INPUTEND) cout<<text;

}


/// ----------- Several AST-related functions -----------

/// create a new AST node with given token information
AST* createASTnode(Attrib* attr, int ttype, char *textt) {
   AST *as = new AST;
   as->kind=(attr)->kind;
   as->text=(attr)->text;
   as->line=(attr)->line;
   as->right=NULL;
   as->down=NULL;
   as->sc=0;
   as->tp=create_type("error",0,0);
   as->ref=0;
   return as;
}

/// create a new "list" AST node with one element
AST* createASTlist(AST *child) {
 AST *as=new AST;
 as->kind="list";
 as->right=NULL;
 as->down=child;
 return as;
}


/// create a new empty "list" AST node
AST* createASTlist() {
 AST *as=new AST;
 as->kind="list";
 as->right=NULL;
 as->down=NULL;
 return as;
}


/// get nth child of a tree. Count starts at 0.
/// if no such child, returns NULL
AST* child(AST *a,int n) {
 AST *c=a->down;
 for (int i=0; c!=NULL && i<n; i++) c=c->right;
 return c;
}


void ASTPrintIndent(AST *a,string s)
{
  if (a==NULL) return;

  cout<<a->kind;
  if (a->text!="") cout<<"("<<a->text<<")";
  cout<<endl;

  AST *i = a->down;
  while (i!=NULL && i->right!=NULL) {
    cout<<s+"  \\__";
    ASTPrintIndent(i,s+"  |"+string(i->kind.size()+i->text.size(),' '));
    i=i->right;
  }

  if (i!=NULL) {
      cout<<s+"  \\__";
      ASTPrintIndent(i,s+"   "+string(i->kind.size()+i->text.size(),' '));
      i=i->right;
  }
}

/// print AST
void ASTPrint(AST *a)
{
  cout<<endl;
  ASTPrintIndent(a,"  ");
}




/// ------------------  MAIN PROGRAM ---------------------

/// Main program
int main(int argc,char *argv[])
{
  AST *root=NULL;
  cout<<endl<<endl<<"  1: ";

  ANTLR(program(&root),stdin);

  // if there are errors, say so and stop
  if (zzLexErrCount>0) {
    cout<<endl<<"There were lexical errors."<<endl;
    exit(0);
  }
  if (zzSyntaxErrCount>0) {
    cout<<endl<<"There were syntax errors."<<endl;
    exit(0);
  }

  // no lexical or syntax errors found. Check types
  cout<<endl<<endl;
  ASTPrint(root);

  cout<<endl<<endl<<"Type Checking:"<<endl<<endl;
  TypeCheck(root);

  // if there are type errors, say so and stop
  if (TypeError) {
    cout<<"There are errors: no code generated"<<endl;
    exit(0);
  }

  // no errors found. Generate code
  cout<<"Generating code:"<<endl;
  codeglobal cg;
  CodeGen(root,cg);
  writecodeglobal(cg);

  if (argc==2 && string(argv[1])=="execute") {
    cout<<endl<<"Executing code:"<<endl;
    executecodeglobal(cg,10000,0);
  }
  else if (argc==2 && string(argv[1])=="executestep") {
    cout<<endl<<"Executing code:"<<endl;
    executecodeglobal(cg,10000,1);
  }
  else if (argc>=2 && string(argv[1])=="writefile") {
    ofstream ofs;
    if (argc>=3) ofs.open(argv[2]);  // Open given filename or
    else ofs.open("program.t");      // use default if none given.
    writecodeglobal(cg,ofs);
  }
}

/// -----------------------------------------------------
>>

#lexclass START
#token PROGRAM      "PROGRAM"
#token ENDPROGRAM   "ENDPROGRAM"
#token VARS         "VARS"
#token ENDVARS      "ENDVARS"
#token PROCEDURE    "PROCEDURE"
#token ENDPROCEDURE "ENDPROCEDURE"

#token IF           "IF"
#token THEN         "THEN"
#token ENDIF        "ENDIF"

#token WHILE        "WHILE"
#token DO           "DO"
#token ENDWHILE     "ENDWHILE"

#token INT          "INT"
#token BOOL         "BOOL"
#token BOOL_VALUE   "TRUE|FALSE"

#token NOT          "NOT"
#token OR          "OR"
#token AND          "AND"

#token VAL          "VAL"
#token REF          "REF"
#token STRUCT       "STRUCT"
#token ENDSTRUCT    "ENDSTRUCT"
#token WRITELN      "WRITELN"

#token PLUS         "\+"
#token MINUS        "\-"
#token TIMES        "\*"
#token DIVIDE       "\/"

#token OPENPAR      "\("
#token CLOSEPAR     "\)"

#token SMALLER      "\<"
#token GREATER      "\>"

#token UNARY         "\-\-"
#token EQUAL         "\="

#token ASIG         ":="
#token DOT          "."
#token COMMA        ","
#token IDENT        "[a-zA-Z][a-zA-Z0-9]*"
#token INTCONST     "[0-9]+"
#token COMMENT      "//~[\n]*" << printf("%s",zzlextext); zzskip(); >>
#token WHITESPACE   "[\ \t]+"  << printf("%s",zzlextext); zzskip(); >>
#token NEWLINE      "\n"       << zzline++; printf("\n%3d: ", zzline); zzskip(); >>
#token LEXICALERROR "~[]"   << printf("Lexical error: symbol '%s' ignored!\n", zzlextext);
                               zzLexErrCount++;
                               zzskip(); >>
#token INPUTEND   "@"

///The start and the end of the program code
program: PROGRAM^ dec_vars l_dec_blocs l_instrs ENDPROGRAM! INPUTEND!;

///Inside the program variables can be declared
dec_vars: (VARS! l_dec_vars ENDVARS!)* <<#0=createASTlist(_sibling);>>;
l_dec_vars: (field)* ;

///a block is a procedure or a function including variables, declarations or more blocks
l_dec_blocs: ( dec_bloc )* <<#0=createASTlist(_sibling);>> ;
dec_bloc: ( dec_bloc_proc |
           dec_bloc_if | dec_bloc_while);

dec_bloc_proc: PROCEDURE^ proc_decl dec_vars l_dec_blocs l_instrs ENDPROCEDURE!;
dec_bloc_if: IF^ expr THEN! dec_vars l_dec_blocs l_instrs ENDIF! ;
dec_bloc_while: WHILE^ expr DO! dec_vars l_dec_blocs l_instrs ENDWHILE! ;

///used to recognize parameters inside a function
///ex. test(VAL arg INT)
dec_param: (VAL^ | REF^) IDENT field_type;
l_param: (dec_param)* (COMMA! dec_param)*  <<#0=createASTlist(_sibling);>>;

///function call
///ex. test(VAL arg1 INT, VAL arg2 BOOL)
proc_decl: IDENT^ OPENPAR! l_param CLOSEPAR! ;

///defintion of a variable
///ex. var1 INT
field: IDENT^ field_type;
field_type: (INT^ | STRUCT^ (field)* ENDSTRUCT! | BOOL);

///list of instructions
l_instrs: (instruction)* <<#0=createASTlist(_sibling);>>;

///an instruction can contain
///- an assigment to a variable
///- a function
///- a newline with a function or a STRING
instruction:
        IDENT (( DOT^ IDENT)* ASIG^ expr | OPENPAR^ (calling_func) CLOSEPAR!)
          | WRITELN^ OPENPAR! ( calling_func) CLOSEPAR!;

///function parameters can be calculations such as 3+a or 3+3
func_param: expr (COMMA! expr)*;

///a list of function parameters
///must generate an empty list even if no parameter
calling_func: (func_param)* <<#0=createASTlist(_sibling);>>;

///adding the OR statement
///expr: (NOT^)* (expr_q ((OR^|AND^) expr_q)*);

///operator + and - has lowest priority
expr: elem (PLUS^ elem | MINUS^ elem)*;

///higher priority of * and /
elem: faktor (TIMES^ faktor | DIVIDE^ faktor)*;

///unary minus
faktor: (MINUS^|NOT^ prim) | prim;


///expr_op: elem (TIMES^ expressionvalue | DIVIDE^ expressionvalue | PLUS^ expressionvalue | MINUS^ expressionvalue | ///SMALLER^ expressionvalue | BIGGER^ expressionvalue | EQUAL^ expressionvalue)*;

///anything that can be validated to be inside an expressions
///such as a variable, a function, a constant or a boolean

///brackets round expression
///prim: term| OPENPAR^ expr CLOSEPAR!;
prim: term;

term:
    IDENT ((DOT^ IDENT)* | OPENPAR^ calling_func CLOSEPAR!)
          | INTCONST | BOOL_VALUE;