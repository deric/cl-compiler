#ifndef MYASTNODE_H
#define MYASTNODE_H

// List of fields in a AST node.

//  kind: the kind of token.
//  text: the string of the token. 
//  line: line in the program where the token appeared.
//  scope: The scope for the case where the AST tree corresponds to
//         a function, a procedure, or the main program.  
//  ptype: The type of the expression (if appliable). 
//  ref: Whether the expression is a left-expression.
#define AST_FIELDS string kind;string text;int line;scope *sc;ptype tp;int ref;

// include rest of AST definitions.  Must go *after* AST_FIELDS
#include "ast.h"


/// create a new "list" AST node with one element
AST* createASTlist(AST *child);

/// create a new empty "list" AST node 
AST* createASTlist();

/// get nth child of a tree. Count starts at 0.
/// if no such child, returns NULL
AST* child(AST *a,int n);

/// print AST 
void ASTPrintIndent(AST *a,string s);
void ASTPrint(AST *a);

#endif

