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
 case STRING:
    attr->kind="string";
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
#token PROCEDURE    "PROCEDURE"
#token ENDPROCEDURE "ENDPROCEDURE"
#token FUNCTION     "FUNCTION"
#token RETURN	    "RETURN"
#token ENDFUNCTION  "ENDFUNCTION"
#token VAL          "VAL"
#token REF          "REF"
#token VARS         "VARS"
#token ENDVARS      "ENDVARS"
#token INT          "INT"
#token BOOL	    "BOOL"
#token ARRAY	    "ARRAY"
#token POINTER	    "POINTER"
#token TO	    "TO"
#token OF	    "OF"
#token STRUCT       "STRUCT"
#token ENDSTRUCT    "ENDSTRUCT"
#token WRITELN      "WRITELN"
#token WRITE        "WRITE"
#token READ         "READ"
#token IF	    "IF"
#token THEN	    "THEN"
#token ELSE	    "ELSE"
#token ENDIF	    "ENDIF"
#token WHILE	    "WHILE"
#token DO	    "DO"
#token ENDWHILE	    "ENDWHILE"
#token AND	    "AND"
#token OR	    "OR"
#token GT           "\>"
#token LT           "\<"
#token EQ           "\="
#token GE           "\>="
#token LE           "\<="
#token PLUS         "\+"
#token MINUS        "\-"
#token MUL          "\*"
#token DIV          "\/"
#token AMPERSAN     "&"
#token SOST	    "^"
#token NOT	    "NOT"
#token STRING	    "\"~[\"]*\""
#token OPENPAR      "\("
#token CLOSEPAR     "\)"
#token OPENBRA      "\["
#token CLOSEBRA     "\]"
#token ASIG         ":="
#token DOT          "\."
#token COMMA        "\,"
#token TRUe          "true"
#token FALSe         "false"
#token IDENT        "[a-zA-Z][a-zA-Z0-9]*"
#token INTCONST     "[0-9]+"
#token COMMENT      "//~[\n]*"            << printf(zzlextext);
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

dec_vars: (VARS! l_dec_vars ENDVARS! | ) <<createASTlist>>; //crear un node list fictici

l_dec_vars: (dec_var)* ;

dec_var: IDENT^ constr_type;

l_dec_blocs: ( dec_bloc )* <<createASTlist>> ;

dec_bloc: (PROCEDURE^ dec_cap dec_vars l_dec_blocs l_instrs ENDPROCEDURE! |
           FUNCTION^ dec_cap_func dec_vars l_dec_blocs l_instrs return_expr ENDFUNCTION!)<</*needs modification*/ >>;


dec_cap: IDENT^ l_def_params;
dec_cap_func: IDENT^ l_def_params RETURN! constr_type;

return_expr: RETURN! expression;

l_def_params: OPENPAR! (l_params | ) CLOSEPAR! <<createASTlist>>;

l_params: param (COMMA! param)*;

param: ( VAL^ | REF^ ) IDENT constr_type;

constr_type: INT | STRUCT^ (field)* ENDSTRUCT! | BOOL | ARRAY^ OPENBRA! INTCONST CLOSEBRA! OF! constr_type | POINTER^ TO! constr_type;

field: IDENT^ constr_type;

l_instrs: (instruction)* <<createASTlist>>;

instruction:
	IDENT^  ((OPENBRA^ expression CLOSEBRA! | DOT^ IDENT | SOST^)* ASIG^ expression | OPENPAR^ list_call_param CLOSEPAR!)
	|IF^ expression THEN! l_instrs (ELSE! l_instrs | ) ENDIF!
	|WHILE^ expression DO! l_instrs ENDWHILE!
	|READ^ OPENPAR! ( expression | STRING ) CLOSEPAR!
	|WRITE^ OPENPAR! ( expression | STRING ) CLOSEPAR!
	|WRITELN^ OPENPAR! ( expression | STRING ) CLOSEPAR!;

expression: expcomp ((AND^|OR^) expcomp)*;

expcomp: expsumsub ((GT^|LT^|EQ^|GE^|LE^) expsumsub)*;

expsumsub: term ((PLUS^|MINUS^) term)*;

term: expsimple ((MUL^|DIV^) expsimple)*;

expsimple:
	IDENT^ (OPENBRA^ expression CLOSEBRA! | DOT^ IDENT | OPENPAR^ list_call_param CLOSEPAR! | SOST^)*
	|INTCONST
	|TRUe
	|FALSe
	|NOT^ expsimple
	|MINUS^ expsimple
	|OPENPAR! expression CLOSEPAR!
	|AMPERSAN^ expsimple;

list_call_param: (expression (COMMA! expression)*|) <<createASTlist>>;

