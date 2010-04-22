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
int newLabelWhile(bool inicialitzar = false){
  static int comptador = 1;
  if (inicialitzar) comptador = 0;
  return comptador++;
}

int newLabelIf(bool inicialitzar = false){
  static int comptador = 1;
  if (inicialitzar) comptador = 0;
  return comptador++;
}


codechain indirections(int jumped_scopes,int t)
{
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

int compute_size(ptype tp)
{
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

void gencodevariablesandsetsizes(scope *sc,codesubroutine &cs,bool isfunction=0)
{
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

void CodeGenRealParams(AST *a,ptype tp,codechain &cpushparam,codechain &cremoveparam,int t)
{
  if (!a) return;
  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;

  //...to be done.

  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}

// ...to be completed:
codechain GenLeft(AST *a,int t)
{
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  if (a->kind=="ident") {
    c="aload _"+a->text+" t"+itostring(t);
  }
  else if (a->kind=="."){
    c=GenLeft(child(a,0),t)||
      "addi t"+itostring(t)+" "+
      itostring(child(a,0)->tp->offset[child(a,1)->text])+" t"+itostring(t);
  }
  else {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;
}


// ...to be completed:
codechain GenRight(AST *a,int t)
{
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  if (a->ref) {
    if (a->kind=="ident" && symboltable.jumped_scopes(a->text)==0 &&
	isbasickind(symboltable[a->text].tp->kind) && symboltable[a->text].kind!="idparref") {
	c="load _"+a->text+" t"+itostring(t);
    }
    else if (isbasickind(a->tp->kind)) {
      c=GenLeft(a,t)||"load t"+itostring(t)+" t"+itostring(t);
    }
    else {//...to be done
    }    
  } 
  else if (a->kind=="intconst") {
    c="iload "+a->text+" t"+itostring(t);
  }
  else if (a->kind=="+") {
    c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "addi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;
}

// ...to be completed:
codechain CodeGenInstruction(AST *a,string info="")
{
  codechain c;

  if (!a) {
    return c;
  }
  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  offsetauxspace=0;
  if (a->kind=="list") {
    for (AST *a1=a->down;a1!=0;a1=a1->right) {
      c=c||CodeGenInstruction(a1,info);
    }
  }
  else if (a->kind==":=") {
    if (isbasickind(child(a,0)->tp->kind)) {
      c=GenLeft(child(a,0),0)||GenRight(child(a,1),1)||"stor t1 t0";
    }
    else if (child(a,1)->ref) {
      c=GenLeft(child(a,0),0)||GenLeft(child(a,1),1)||"copy t1 t0 "+itostring(child(a,1)->tp->size);
    }
    else {
      c=GenLeft(child(a,0),0)||GenRight(child(a,1),1)||"copy t1 t0 "+itostring(child(a,1)->tp->size);
    }
  } 
  else if (a->kind=="write" || a->kind=="writeln") {
    if (child(a,0)->kind=="string") {
      //...to be done.
    } 
    else {//Exp
      c=GenRight(child(a,0),0)||"wrii t0";
    }

    if (a->kind=="writeln") {
      c=c||"wrln";
    }
  }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;

  return c;
}

void CodeGenSubroutine(AST *a,list<codesubroutine> &l)
{
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

void CodeGen(AST *a,codeglobal &cg)
{
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

