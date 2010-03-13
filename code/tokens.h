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
#define PROCEDURE 6
#define ENDPROCEDURE 7
#define IF 8
#define THEN 9
#define ENDIF 10
#define WHILE 11
#define DO 12
#define ENDWHILE 13
#define INT 14
#define BOOL 15
#define BOOL_VALUE 16
#define NOT 17
#define VAL 18
#define REF 19
#define STRUCT 20
#define ENDSTRUCT 21
#define WRITELN 22
#define PLUS 23
#define MINUS 24
#define TIMES 25
#define DIVIDE 26
#define OPENPAR 27
#define CLOSEPAR 28
#define ASIG 29
#define DOT 30
#define COMMA 31
#define IDENT 32
#define INTCONST 33
#define COMMENT 34
#define WHITESPACE 35
#define NEWLINE 36
#define LEXICALERROR 37
#define STRING 38

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
void l_dec_blocs(AST**_root);
#else
extern void l_dec_blocs();
#endif

#ifdef __USE_PROTOS
void dec_bloc(AST**_root);
#else
extern void dec_bloc();
#endif

#ifdef __USE_PROTOS
void dec_bloc_proc(AST**_root);
#else
extern void dec_bloc_proc();
#endif

#ifdef __USE_PROTOS
void dec_bloc_if(AST**_root);
#else
extern void dec_bloc_if();
#endif

#ifdef __USE_PROTOS
void dec_bloc_while(AST**_root);
#else
extern void dec_bloc_while();
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
void expressionvalue(AST**_root);
#else
extern void expressionvalue();
#endif

#endif
extern SetWordType zzerr1[];
extern SetWordType setwd1[];
extern SetWordType zzerr2[];
extern SetWordType zzerr3[];
extern SetWordType setwd2[];
extern SetWordType zzerr4[];
extern SetWordType zzerr5[];
extern SetWordType zzerr6[];
extern SetWordType zzerr7[];
extern SetWordType setwd3[];
extern SetWordType zzerr8[];
extern SetWordType zzerr9[];
extern SetWordType setwd4[];
