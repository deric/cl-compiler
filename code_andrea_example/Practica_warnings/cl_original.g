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

typedef struct {
    string kind;
    string text;
    int line;
} Attrib;
void zzcr_attr(Attrib *attr,int type,char *text);

// Explanation about the AST fields: the kind of token.
// the string of the token. The scope for the case where the
// AST tree is the one of a function, a procedure, or
// the main program.  The type of the expression, just for
// expressions. Whether the expression is a left-expression.
#define AST_FIELDS string kind;string text;int line;scope *sc;ptype tp;int ref;

#define zzcr_ast(as,attr,tttype,textt) as=new AST;as->kind=(attr)->kind;as->text=(attr)->text;as->line=(attr)->line;as->right=NULL;as->down=NULL;as->sc=0;as->tp=create_type("error",0,0);as->ref=0;
#define createASTlist #0=new AST;(#0)->kind="list";(#0)->right=NULL;(#0)->down=_sibling;
#define createnullASTlist #0=new AST;(#0)->kind="list";(#0)->right=NULL;(#0)->down=NULL;
>>

<<
#include "semantic.hh"
#include "codegen.hh"

string lowercase(string s)
{
  for (int i=0;i<(int)s.size();i++) {
    if (s[i]>='A' && s[i]<='Z') {
      s[i]=s[i]-'A'+'a';
    }
  }
  return s;
}

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

void ASTPrintIndent(AST *a,string s)
{
  if (a==NULL) return;
  cout<<s<<" "<<a->kind;
  if (a->text!="") cout<<"("<<a->text<<")";
  cout<<endl;
  ASTPrintIndent(a->down,s+" |");
  ASTPrintIndent(a->right,s);
}

void ASTPrint(AST *a)
{
  cout<<endl;
  ASTPrintIndent(a,"");
}
extern int TypeError;

int main(int argc,char *argv[])
{
  AST *root=NULL;
  cout<<endl<<endl<<"  1: ";
  ANTLR(program(&root),stdin);
  if (zzLexErrCount>0) {
    cout<<endl<<"There were lexical errors."<<endl;
  } else if (zzSyntaxErrCount>0) {
    cout<<endl<<"There were syntax errors."<<endl;
  } else {
    cout<<endl<<endl;
    ASTPrint(root);
    cout<<endl<<endl<<"Type Checking:"<<endl<<endl;
    TypeCheck(root);
    if (TypeError) {
      cout<<"There are errors: no code generated"<<endl;
    } else {
      cout<<"Generating code:"<<endl;
      codeglobal cg;
      CodeGen(root,cg);
      writecodeglobal(cg);
      if (argc==2 && string(argv[1])=="execute") {
        cout<<endl<<"Executing code:"<<endl;
        executecodeglobal(cg,10000,0);
      } else if (argc==2 && string(argv[1])=="executestep") {
        cout<<endl<<"Executing code:"<<endl;
        executecodeglobal(cg,10000,1);
      } else if (argc>=2 && string(argv[1])=="writefile") {
        ofstream ofs;
        if (argc>=3) {
          ofs.open(argv[2]);
        } else {
          ofs.open("program.t");
        }
        writecodeglobal(cg,ofs);
      }
    }
  }
}


>>

#lexclass START
#token PROGRAM      "PROGRAM"
#token ENDPROGRAM   "ENDPROGRAM"   
#token VARS         "VARS"         
#token ENDVARS      "ENDVARS"      
#token INT          "INT"          
#token STRUCT       "STRUCT"
#token ENDSTRUCT    "ENDSTRUCT"
#token WRITELN      "WRITELN"      
#token PLUS          "\+"
#token OPENPAR      "\("
#token CLOSEPAR    "\)"
#token ASIG         ":="
#token DOT          "."
#token IDENT        "[a-zA-Z][a-zA-Z0-9]*"
#token INTCONST     "[0-9]+"
#token COMMENT   "//~[\n]*"            << printf(zzlextext);
                                               zzskip(); >>
#token WHITESPACE      "[\ \t]+"               << printf("%s",zzlextext);
                                               zzskip(); >>
#token NEWLINE   "\n"                    << zzline++;
                                               printf("\n%3d: ", zzline);
                                               zzskip(); >>
#token LEXICALERROR  "~[]"       << printf("Lexical error: symbol '%s' ignored!\n", zzlextext);
                                    zzLexErrCount++;
                                    zzskip(); >>
#token INPUTEND   "@"

program: PROGRAM^ dec_vars l_dec_blocs l_instrs ENDPROGRAM! INPUTEND!;

dec_vars: (VARS! l_dec_vars ENDVARS! | ) <<createASTlist>>;

l_dec_vars: (dec_var)* ;

dec_var: IDENT^ constr_type;

l_dec_blocs: ( dec_bloc )* <<createASTlist>> ;

dec_bloc: (PROCEDURE^ ENDPROCEDURE |
           FUNCTION^ ENDFUNCTION)<</*needs modification*/ >>;

constr_type: INT | STRUCT^ (field)* ENDSTRUCT!;

field: IDENT^ constr_type;

l_instrs: (instruction)* <<createASTlist>>;

instruction:
        IDENT ( DOT^ IDENT)* ASIG^ expression
      |	WRITELN^ OPENPAR! ( expression | STRING ) CLOSEPAR!;

expression: expsimple (PLUS^ expsimple)*;

expsimple:
        IDENT^ (DOT^ IDENT)*
      | INTCONST
      ;
