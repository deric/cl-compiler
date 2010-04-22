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

#lexclass START
#token PROGRAM      "PROGRAM"
#token ENDPROGRAM   "ENDPROGRAM"

#token PROCEDURE    "PROCEDURE"
#token ENDPROCEDURE "ENDPROCEDURE"
#token FUNCTION     "FUNCTION"
#token ENDFUNCTION  "ENDFUNCTION"
#token RETURN       "RETURN"

#token VARS         "VARS"
#token ENDVARS      "ENDVARS"
#token INT          "INT"

#token BOOL         "BOOL"
#token BOOL_TRUE    "true"
#token BOOL_FALSE   "false"
#token VAL          "VAL"
#token REF          "REF"

#token STRUCT       "STRUCT"
#token ENDSTRUCT    "ENDSTRUCT"
#token WRITELN      "WRITELN"
#token ARRAY        "ARRAY"
#token OF           "OF"

#token PLUS         "\+"
#token MINUS        "\-"
#token MULTIPLY     "\*"
#token DIVIDE       "/"
#token GREATERTHAN  ">"
#token LESSERTHAN   "<"
#token NEGATIVE     "\-"
#token EQUAL        "\="
#token AND          "AND"
#token OR           "OR"

#token OPENPAR      "\("
#token CLOSEPAR     "\)"
#token OPENSQUAB    "\["
#token CLOSESQUAB   "\]"
#token ASIG         ":="
#token DOT          "."

#token COMMA        ","
#token IF           "IF"
#token ENDIF        "ENDIF"
#token THEN         "THEN"
#token ELSE         "ELSE"
#token NOT          "NOT"
#token WHILE        "WHILE"
#token DO           "DO"
#token ENDWHILE     "ENDWHILE"


#token IDENT        "[a-zA-Z][a-zA-Z0-9]*"
#token INTCONST     "[0-9]+"
#token COMMENT      "//~[\n]*" << printf("%s",zzlextext); zzskip(); >>
#token WHITESPACE   "[\ \t]+"  << printf("%s",zzlextext); zzskip(); >>
#token NEWLINE      "\n"       << zzline++; printf("\n%3d: ", zzline); zzskip(); >>
#token LEXICALERROR "~[]"   << printf("Lexical error: symbol '%s' ignored!\n", zzlextext);
                               zzLexErrCount++;
                               zzskip(); >>
#token INPUTEND   "@"

// a program consists of variables blocks and instructions

program: PROGRAM^ dec_vars l_dec_blocs l_instrs ENDPROGRAM! INPUTEND!;

// declaration of variables

dec_vars: (VARS! l_dec_vars ENDVARS! | ) <<#0=createASTlist(_sibling);>>;

l_dec_vars: (dec_var)* ;

dec_var: IDENT^ constr_type;

// declaration of blocks ( procedure and functions )

l_dec_blocs: ( dec_bloc )* <<#0=createASTlist(_sibling);>> ;

// procedure or funcbloc
dec_bloc: proc_bloc | func_bloc; 
// proc bloc : declration, definition 
proc_bloc:PROCEDURE^ proc_dec proc_def ENDPROCEDURE!;
// proc declration : name and parameters
proc_dec: IDENT^ proc_l_params;
// proc definition : variables, more blocs, instructions
proc_def: dec_vars l_dec_blocs l_instrs;

// proc list of parameters ( proc params, proc params ) 
proc_l_params: OPENPAR! ( proc_params (COMMA! proc_params)* | ) CLOSEPAR! <<#0=createASTlist(_sibling);>> ;
// proc params by value or reference name and type
proc_params: (VAL^ | REF^) IDENT constr_type  ;

//function bloc : definition , return
func_bloc:FUNCTION^ func_dec func_def ENDFUNCTION!<</*needs modification*/ >>;
// func declaration (method signature):name & parameters and return type
func_dec: IDENT^ proc_l_params RETURN! constr_type;
// function definition ?
func_def: dec_vars l_dec_blocs l_instrs RETURN! expression;
// keep contruction types as primitive or complex
constr_type: prim_type | complex_type;

prim_type : INT | BOOL;
// a complex type is a struct or an Array [ int ] of type
complex_type: STRUCT^ (field)* ENDSTRUCT! | ARRAY^ OPENSQUAB! INTCONST CLOSESQUAB! OF! constr_type;

field: IDENT^ constr_type;

l_instrs: (instruction)* <<#0=createASTlist(_sibling);>>;

// instruction can be of type
// 1. assignment p:=3
// 2. function call   a) p() b) p(2)  c) p(r()) 
// 3. if instruction ; while instruction
instruction: IDENT (DOT^ IDENT | OPENSQUAB^ expression CLOSESQUAB!  )* ( ASIG^ expression | OPENPAR^ instrParams) | instrWriteln | if_instruction | while_instruction;
instrParams: (expression (COMMA! expression)* | ) CLOSEPAR! <<#0=createASTlist(_sibling);>>; 
instrWriteln: WRITELN^ OPENPAR! ( expression | STRING ) CLOSEPAR!;

// if instruction : if expression then instructions else instructions
if_instruction: IF^ expression THEN! l_instrs (ELSE! l_instrs | ) ENDIF!;


// while instruction : while expression do instructions
while_instruction: WHILE^ expression DO! l_instrs ENDWHILE!;

// condition: (not)* some expression ( >, <, =) expression 
//cond_compare ( (AND^ | OR^ ) cond_compare)*;
//cond_compare: expr_plus_sub ((GREATERTHAN^ | LESSERTHAN^ | EQUAL^) expr_plus_sub)*;

//condition: (NOT^)* expression ((GREATERTHAN^ | LESSERTHAN^ | EQUAL^) expression)* | (EQUAL^ condition); 

// expression : expsimple or expsimple  OPERATION expsimple
//expression: (NEGATIVE^)* expsimple (PLUS^ expsimple | DIVIDE^ expsimple | MINUS^ expsimple)*;

// expsimple : name, name.subname, name (expression , expression ) , int, true

// assigning precedence and associativity (from notes)
// lower precedence first 
// expr : expr ’+’ term    = term ( (+ | -) term )*
//       | expr ’-’ term
//       | term ;
// term : term ’*’ atom
//       | term ’/’ atom
//       | atom ;
// atom : NUMBER ;

// operator precedence:  { * / }  { + -}  { <  > } = 
// AND / OR ??

expression: expr_compare ( (AND^ | OR^ ) expr_compare)*;
expr_compare: expr_plus_sub ((GREATERTHAN^ | LESSERTHAN^ | EQUAL^) expr_plus_sub)*;

expr_plus_sub: expr_mult_div ( (PLUS^ | MINUS^ ) expr_mult_div)*;
expr_mult_div: not_negative ( ( MULTIPLY^ |  DIVIDE^ ) not_negative)*;
not_negative: (NOT^ not_negative | NEGATIVE^ not_negative | expsimple);

expsimple:  IDENT^ (DOT^ IDENT | OPENPAR^ instrParams | OPENSQUAB^ expression CLOSESQUAB! )* | INTCONST | OPENPAR! expression CLOSEPAR! | BOOL_TRUE | BOOL_FALSE  ;
