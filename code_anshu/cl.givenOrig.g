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
#token INT          "INT"
#token STRUCT       "STRUCT"
#token ENDSTRUCT    "ENDSTRUCT"
#token WRITELN      "WRITELN"
#token PLUS         "\+"
#token OPENPAR      "\("
#token CLOSEPAR     "\)"
#token ASIG         ":="
#token DOT          "."
#token IDENT        "[a-zA-Z][a-zA-Z0-9]*"
#token INTCONST     "[0-9]+"
#token COMMENT      "//~[\n]*" << printf("%s",zzlextext); zzskip(); >>
#token WHITESPACE   "[\ \t]+"  << printf("%s",zzlextext); zzskip(); >>
#token NEWLINE      "\n"       << zzline++; printf("\n%3d: ", zzline); zzskip(); >>
#token LEXICALERROR "~[]"   << printf("Lexical error: symbol '%s' ignored!\n", zzlextext);
                               zzLexErrCount++;
                               zzskip(); >>
#token INPUTEND   "@"

program: PROGRAM^ dec_vars l_dec_blocs l_instrs ENDPROGRAM! INPUTEND!;

dec_vars: (VARS! l_dec_vars ENDVARS! | ) <<#0=createASTlist(_sibling);>>;

l_dec_vars: (dec_var)* ;

dec_var: IDENT^ constr_type;

l_dec_blocs: ( dec_bloc )* <<#0=createASTlist(_sibling);>> ;

dec_bloc: (PROCEDURE^ ENDPROCEDURE |
           FUNCTION^ ENDFUNCTION)<</*needs modification*/ >>;

constr_type: INT | STRUCT^ (field)* ENDSTRUCT!;

field: IDENT^ constr_type;

l_instrs: (instruction)* <<#0=createASTlist(_sibling);>>;

instruction:
        IDENT ( DOT^ IDENT)* ASIG^ expression
      |	WRITELN^ OPENPAR! ( expression | STRING ) CLOSEPAR!;

expression: expsimple (PLUS^ expsimple)*;

expsimple:
        IDENT^ (DOT^ IDENT)*
      | INTCONST
      ;
