/* ===== semantic.c ===== */
#include <string>
#include <iostream>
#include <map>
#include <list>
#include <vector>

using namespace std;

#include <stdio.h>
#include <stdlib.h>

#include "ptype.hh"
#include "symtab.hh"
#include "codegest.hh"

#include "myASTnode.hh"

#include "util.hh"
#include "semantic.hh"

#include "codegen.hh"

// symbol table with information about identifiers in the program
// declared in symtab.cc
extern symtab symboltable;

// When dealing with a list of instructions, it contains the maximum auxiliar space
// needed by any of the instructions for keeping non-referenceable non-basic-type values.
int maxoffsetauxspace;

// When dealing with one instruction, it contains the auxiliar space
// needed for keeping non-referenceable values.
int offsetauxspace;

// For distinghishing different labels for different if's and while's.

/* defines a new While label to keep track of the amount */
int newLabelWhile(bool initialize = false) {
  static int counter = 1;
  if (initialize) counter = 0;
  return counter++;
}
/* defines a new If label to keep track of the amount */
int newLabelIf(bool initialize = false) {
  static int counter = 1;
  if (initialize) counter = 0;
  return counter++;
}
/* defines a new NAND label to keep track of the amount */
int newLabelNand(bool initialize = false){
	static int counter = 1;
	if(initialize) counter = 0;
	return counter++;
}

codechain indirections(int jumped_scopes,int t) {
  codechain c;
  if (jumped_scopes==0) {
    c="aload static_link t"+itostring(t);
  }
  else {
    c="load static_link t"+itostring(t);
    for (int i=1;i<jumped_scopes;i++) {
      c=c||"load t"+itostring(t)+" t"+itostring(t);
    }
  }
  return c;
}

int compute_size(ptype tp) {
  if (isbasickind(tp->kind)) {
    tp->size=4;
  }
  else if (tp->kind=="array") {
    tp->size=tp->numelemsarray*compute_size(tp->down);
  }
  else if (tp->kind=="struct") {
    tp->size=0;
    for (list<string>::iterator it=tp->ids.begin();it!=tp->ids.end();it++) {
      tp->offset[*it]=tp->size;
      tp->size+=compute_size(tp->struct_field[*it]);
    }
  }
  return tp->size;
}

void gencodevariablesandsetsizes(scope *sc,codesubroutine &cs,bool isfunction=0) {
  if (isfunction) cs.parameters.push_back("returnvalue");
  for (list<string>::iterator it=sc->ids.begin();it!=sc->ids.end();it++) {
    if (sc->m[*it].kind=="idvarlocal") {
      variable_data vd;
      vd.name="_"+(*it);
      vd.size=compute_size(sc->m[*it].tp);
      cs.localvariables.push_back(vd);
    } else if (sc->m[*it].kind=="idparval" || sc->m[*it].kind=="idparref") {
      compute_size(sc->m[*it].tp);
      cs.parameters.push_back("_"+(*it));
    } else if (sc->m[*it].kind=="idfunc") {
      // Here it is assumed that in tp->right is kept the return value type
      // for the case of functions. If this is not the case you must
      // change this line conveniently in order to force the computation
      // of the size of the type returned by the function.
      compute_size(sc->m[*it].tp->right);
    } else if (sc->m[*it].kind=="idproc") {
      // Nothing to be done.
    }
  }
  cs.parameters.push_back("static_link");
}

codechain GenLeft(AST *a,int t);
codechain GenRight(AST *a,int t);

void CodeGenRealParams(AST *a,ptype tp,codechain &cpushparam,codechain &cremoveparam,int t) {
  if (!a) return;
 //cout<<"Starting with node \""<<a->kind<<"\""<<endl;

  //...to be done.

  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}

// ...to be completed:
codechain GenLeft(AST *a,int t) {
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  /* IDENT (creating new var) */
  if (a->kind=="ident") {
    c="aload _"+a->text+" t"+itostring(t);
  }
  else if (a->kind==".") {
    c=GenLeft(child(a,0),t)||
      "addi t"+itostring(t)+" "+
      itostring(child(a,0)->tp->offset[child(a,1)->text])+" t"+itostring(t);
  }
  else {
    cout<<"BIG PROBLEM! No case defined in GenLeft for kind  "<<a->kind<<endl;
  }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;
}


codechain GenRight(AST *a,int t) {
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  /*Is referenceable*/
  if (a->ref) {
    if (a->kind=="ident" && symboltable.jumped_scopes(a->text)==0 &&
      isbasickind(symboltable[a->text].tp->kind) && symboltable[a->text].kind!="idparref") {
      c="load _"+a->text+" t"+itostring(t);
    }
    /* Basic types */
    else if (isbasickind(a->tp->kind)) {
      c=GenLeft(a,t)||
      "load t"+itostring(t)+" t"+itostring(t);
    }
    /* Not basic types! */
    else {
      int size = compute_size(a->tp);
      c=GenLeft(a,t+1)||
      "aload aux_space t"+itostring(t)||
      "addi t"+itostring(t)+" "+ itostring(offsetauxspace)+ " t"+itostring(t)||
      "copy t"+itostring(t+1) +" t"+itostring(t) +" "+ itostring(size);
      offsetauxspace+=size;
    }
  /* Non referencable types from this point */
  /* INTCONST */
  } else if (a->kind=="intconst") {
    c="iload "+a->text+" t"+itostring(t);
  /* BOOLEAN - TRUE */
  } else if (a->kind=="true") {
    c = "iload 1 t"+itostring (t);
  /* BOOLEAN - FALSE */
  } else if (a->kind=="false") {
    c = "iload 0 t"+itostring (t);
  }
  else if (a->kind=="+") {
    c=GenRight(child(a,0),t) ||
    GenRight(child(a,1),t+1) ||
    "addi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* Reversing the sign (Unary expression) */
  else if (a->kind == "-" && a->down->right == 0) {
    c = GenRight (a->down, t) || "mini t" + itostring (t) + " t" + itostring (t);
  }
  /* SUM */
  else if (a->kind=="+") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "addi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* REST (Binary ) */
  else if (a->kind=="-" && a->down->right != 0) {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "subi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* Multiplication */
  else if (a->kind=="*") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "muli t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* Division */
  else if (a->kind=="/") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "divi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* Less then */
  else if (a->kind == "<") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "lesi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  /* Less or equal then */
  }
  else if (a->kind == "<=") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "lesi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* Bigger then */
  else if (a->kind == ">") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "grti t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* Bigger or equal then */
  else if (a->kind == ">=") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "grti t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* Equal to */
  else if (a->kind == "=") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "equi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  
  }
  /* AND */
  else if (a->kind == "and") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "land t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* Not AND */
  else if (a->kind == "nand") {
	int lab = newLabelNand(false);
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"land t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t)||
	"fjmp t"+itostring(t)+" nandzero_"+itostring(lab)||
	"iload 0 t"+itostring (t)||
	"ujmp endnand_"+itostring(lab)||
	"etiq nandzero_"+itostring(lab)||
	"iload 1 t"+itostring (t)||
	"etiq endnand_"+itostring(lab);
  }
  /* OR */
  else if (a->kind == "or") {
    c = GenRight(a->down,t)||
    GenRight(a->down->right,t+1)||
    "loor t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  /* NOT */
  else if (a->kind == "not") {
    c = GenRight(a->down,t)||
    "lnot t" + itostring (t) + " t" + itostring (t);
  }
  /* Access to a struct */
  else if (a->kind == ".") {
    ptype typ = symboltable[a->down->down->text].tp;
    CodeGenRealParams(a->down->down,typ,c,c,t);
    c = c||"addi t"+itostring(t)+" "+itostring(a->down->tp->offset[a->down->right->text])+" t"+itostring(t);
  }
  /* Identifier of a function */
  else if (a->kind == "idfunc") {  
    codechain cpush, ckill;
    CodeGenRealParams(a->down, symboltable[a->down->text].tp, c, ckill, t);  
  }
  else {
    cout<<"BIG PROBLEM! No case defined in GenRight for kind "<<a->kind<<endl;
  }

  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;

}

codechain CodeGenInstruction(AST *a,string info="")
{
  codechain c;

  if (!a) {
    return c;
  }
  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  offsetauxspace=0;
  /* LIST */
  if (a->kind=="list") {
    /* for every instruction in the list - call CodeGenInstruction */
    for (AST *a1=a->down;a1!=0;a1=a1->right) { 
      c=c||CodeGenInstruction(a1,info);
    }
  }
  /* assigning a identifier */
  else if (a->kind==":=") {
    /* Basic types */
    if (isbasickind(child(a,0)->tp->kind)) {
      c=GenLeft(child(a,0),0)||GenRight(child(a,1),1)||"stor t1 t0";
    }
    /* Complex or referencables types */
    else if (child(a,1)->ref) {
      c=GenLeft(child(a,0),0)||GenLeft(child(a,1),1)||"copy t1 t0 "+itostring(child(a,1)->tp->size);
    }
    /* complex but not referencable types */
    else {
      c=GenLeft(child(a,0),0)||GenRight(child(a,1),1)||"copy t1 t0 "+itostring(child(a,1)->tp->size);
    }
  }
  /* WRITE/WRITELN */
  else if (a->kind=="write" || a->kind=="writeln") {
    /*If it is a string */
    if (child(a,0)->kind=="string") {
      c="wris "+a->down->text;
    }
    /* otherwise we write integer */
    else {//Exp
      c=GenRight(child(a,0),0)||"wrii t0";
    }
    if (a->kind=="writeln") {
      c=c||"wrln";
    }
  }
  /* READ */
  else if (a->kind=="read") {
    if (child(a,0)->kind=="string") {
      c="reai "+a->down->text;
    } else {//Exp
      c=GenRight(a->down,0)||"reai t0";
    }
  
  }
  /* IF */
  else if (a->kind=="if") {
    int lab = newLabelIf(false);
    // There is an ELSE
    if (a->down->right->right) {
        c=GenRight(a->down,0)||
        "fjmp t0 else_"+itostring(lab)||
        CodeGenInstruction(a->down->right,"instruction")||
        "ujmp endif_"+itostring(lab)||
        "etiq else_"+itostring(lab)||
        CodeGenInstruction(a->down->right->right,"instruction")||
        "etiq endif_"+itostring(lab);
    }
    // There is no ELSE
    else {
      c=GenRight(a->down,0)||
      "fjmp t0 endif_"+itostring(lab)||
      CodeGenInstruction(a->down->right,"instruction")||
      "etiq endif_"+itostring(lab);
    }
  }
  /* WHILE */
  else if (a->kind=="while") {
    int lab = newLabelWhile(false);
    c="etiq while_"+itostring(lab)||
    GenRight(a->down,0)||
    "fjmp t0 endwhile_"+itostring(lab)||
    CodeGenInstruction(a->down->right,"instruction")||
    "ujmp while_"+itostring(lab)||
    "etiq endwhile_"+itostring(lab);
  }
  /* ID PROCEDURE */
  else if (a->kind=="idproc") {
    codechain kill;
    CodeGenRealParams(a->down,symboltable[a->down->text].tp,c,kill,0);
  }
  /* LABEL PROCEDURE */
  else if (a->kind=="label") {
    c="etiq label_"+a->down->text;
  /* PROCEDURE */
  }
  /* GOTO instruction */
  else if (a->kind=="goto") {  // call of procedure
    c="ujmp label_"+a->down->text;

  }
  else {
    cout<<"BIG PROBLEM! No case defined in CodeGenInstruction for kind "<<a->kind<< "("<<a->text<<") in line "<<a->line<<endl;
  }
  /* FIGURE OUT THIS LINE */
  if (offsetauxspace > maxoffsetauxspace) 	maxoffsetauxspace = offsetauxspace;
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;

  return c;
}

void CodeGenSubroutine(AST *a,list<codesubroutine> &l) {
  codesubroutine cs;

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  string idtable=symboltable.idtable(child(a,0)->text);
  cs.name=idtable+"_"+child(a,0)->text;
  symboltable.push(a->sc);
  symboltable.setidtable(idtable+"_"+child(a,0)->text);

  //...to be done.

  symboltable.pop();
  l.push_back(cs);
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;

}

void CodeGen(AST *a,codeglobal &cg) {
  initnumops();
  securemodeon();
  cg.mainsub.name="program";
  symboltable.push(a->sc);
  symboltable.setidtable("program");
  gencodevariablesandsetsizes(a->sc,cg.mainsub);
  for (AST *a1=child(child(a,1),0);a1!=0;a1=a1->right) {
    CodeGenSubroutine(a1,cg.l);
  }
  maxoffsetauxspace=0; newLabelIf(true); newLabelWhile(true);
  cg.mainsub.c=CodeGenInstruction(child(a,2))||"stop";
  if (maxoffsetauxspace>0) {
    variable_data vd;
    vd.name="aux_space";
    vd.size=maxoffsetauxspace;
    cg.mainsub.localvariables.push_back(vd);
  }
  symboltable.pop();
}

