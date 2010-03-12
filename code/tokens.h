#ifndef tokens_h
#define tokens_h
/* tokens.h -- List of labelled tokens and stuff
 *
 * Generated from: cl.g
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-2001
 * Purdue University Electrical Engineering
 * ANTLR Version 1.33MR33
 */
#define zzEOF_TOKEN 1
#define INPUTEND 1
#define PROGRAM 2
#define ENDPROGRAM 3
#define VARS 4
#define ENDVARS 5
#define INT 6
#define BOOL 7
#define STRUCT 8
#define ENDSTRUCT 9
#define WRITELN 10
#define PROCEDURE 11
#define ENDPROCEDURE 12
#define VAL 13
#define REF 14
#define PLUS 15
#define MINUS 16
#define TIMES 17
#define DIVIDE 18
#define OPENPAR 19
#define CLOSEPAR 20
#define ASIG 21
#define DOT 22
#define COMMA 23
#define BOOL_VAL 24
#define IDENT 25
#define INTCONST 26
#define COMMENT 27
#define WHITESPACE 28
#define NEWLINE 29
#define LEXICALERROR 30
#define FUNCTION 31
#define ENDFUNCTION 32
#define STRING 33

#ifdef __USE_PROTOS
void program(AST**_root);
#else
extern void program();
#endif

#ifdef __USE_PROTOS
void dec_vars(AST**_root);
#else
extern void dec_vars();
#endif

#ifdef __USE_PROTOS
void l_dec_vars(AST**_root);
#else
extern void l_dec_vars();
#endif

#ifdef __USE_PROTOS
void var_def(AST**_root);
#else
extern void var_def();
#endif

#ifdef __USE_PROTOS
void l_dec_blocs(AST**_root);
#else
extern void l_dec_blocs();
#endif

#ifdef __USE_PROTOS
void dec_param(AST**_root);
#else
extern void dec_param();
#endif

#ifdef __USE_PROTOS
void l_param(AST**_root);
#else
extern void l_param();
#endif

#ifdef __USE_PROTOS
void dec_bloc(AST**_root);
#else
extern void dec_bloc();
#endif

#ifdef __USE_PROTOS
void proc_decl(AST**_root);
#else
extern void proc_decl();
#endif

#ifdef __USE_PROTOS
void constr_type(AST**_root);
#else
extern void constr_type();
#endif

#ifdef __USE_PROTOS
void field(AST**_root);
#else
extern void field();
#endif

#ifdef __USE_PROTOS
void l_instrs(AST**_root);
#else
extern void l_instrs();
#endif

#ifdef __USE_PROTOS
void instruction(AST**_root);
#else
extern void instruction();
#endif

#ifdef __USE_PROTOS
void func_param(AST**_root);
#else
extern void func_param();
#endif

#ifdef __USE_PROTOS
void calling_func(AST**_root);
#else
extern void calling_func();
#endif

#ifdef __USE_PROTOS
void expression(AST**_root);
#else
extern void expression();
#endif

#ifdef __USE_PROTOS
void term(AST**_root);
#else
extern void term();
#endif

#ifdef __USE_PROTOS
void expsimple(AST**_root);
#else
extern void expsimple();
#endif

#endif
extern SetWordType zzerr1[];
extern SetWordType setwd1[];
extern SetWordType zzerr2[];
extern SetWordType zzerr3[];
extern SetWordType zzerr4[];
extern SetWordType setwd2[];
extern SetWordType zzerr5[];
extern SetWordType zzerr6[];
extern SetWordType setwd3[];
extern SetWordType zzerr7[];
extern SetWordType zzerr8[];
extern SetWordType setwd4[];
