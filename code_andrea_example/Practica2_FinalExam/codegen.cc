/*************************/
/* ===== codegen.c ===== */
/*************************/

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

#define AST_FIELDS string kind;string text;int line;scope *sc;ptype tp;int ref;
#define zzcr_ast(as,attr,tttype,textt) as=new AST;as->kind=(attr)->kind;as->text=(attr)->text;as->line=(attr)->line;as->right=NULL;as->down=NULL;as->sc=0;as->tp=0;as->ref=0;
#define createASTlist (*_root)=new AST;((*_root))->kind="list";((*_root))->right=NULL;((*_root))->down=_sibling;
#define GENAST
#include "ast.h"

//#include "semantic.hh"

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
	if(inicialitzar) comptador = 0;
	return comptador++;
}
int newLabelIf(bool inicialitzar = false){
	static int comptador = 1;
	if(inicialitzar) comptador = 0;
	return comptador++;
}

int newLabelNand(bool inicialitzar = false){
	static int comptador = 1;
	if(inicialitzar) comptador = 0;
	return comptador++;
}

string itostring(int x)
{
  string s;
  if (x==0) return "0";

  while(x>0) {
    s=char((x%10)+'0')+s;
    x=x/10;
  }
  return s;
}

/** INDI **/
codechain indirections(int jumped_scopes,int t)
{
  codechain c;
  //Mateix scope
  if (jumped_scopes==0) {
    	c="aload static_link t"+itostring(t);
  } else {
  // Ha de saltar scopes
    	c="load static_link t"+itostring(t);
    	for (int i=1;i<jumped_scopes;i++) {
     		 c = c
      		||"load t"+itostring(t)+" t"+itostring(t);
   	 }
  }
  return c;
}

bool isbasickind(string kind);

int compute_size(ptype tp)
{
  if (isbasickind(tp->kind)) {
    tp->size=4;
  } else if (tp->kind=="array") {
    tp->size=tp->numelemsarray*compute_size(tp->down);
  } else if (tp->kind=="struct") {
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

/** DECLARACIO DE CAPSALERES **/
void CodeGenRealParams(AST *a,ptype tp,codechain &cpushparam,codechain &cremoveparam,int t);
codechain GenLeft(AST *a,int t);
codechain GenRight(AST *a,int t);


/** GENRIGHT **/
codechain GenRight(AST *a,int t)
{
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;

 /*Es referenciable*/
  if (a->ref) {
	/* Es un identificador, esta al mateix scope i no es un parametre per referencia*/
	if (a->kind=="ident" && symboltable.jumped_scopes(a->text)==0 && isbasickind(symboltable[a->text].tp->kind) && symboltable[a->text].kind!="idparref") {
		c = "load _"+a->text+" t"+itostring(t);
	
	/* De tipus basic)*/
	} else if (isbasickind(a->tp->kind)) {

		c = GenLeft(a,t)
		||"load t"+itostring(t)+" t"+itostring(t);

	/* NO es tipus basic - utilitzacio del aux_space */
	} else {
		int size = compute_size(a->tp);
		 c=GenLeft(a,t+1)||
		"aload aux_space t"+itostring(t)||
		"addi t"+itostring(t)+" "+ itostring(offsetauxspace)+ " t"+itostring(t)||
		"copy t"+itostring(t+1) +" t"+itostring(t) +" "+ itostring(size);
		offsetauxspace+=size;
	}

  /* No son referenciables a partir d'aquest punt */
  /* INTCONST */
  } else if (a->kind=="intconst") {
	c="iload "+a->text+" t"+itostring(t);

  /* BOOLEAN - TRUE */
  } else if (a->kind=="true") {
	c = "iload 1 t"+itostring (t);

  /* BOOLEAN - FALSE */
  } else if (a->kind=="false") {
	c = "iload 0 t"+itostring (t);

  /* CANVI DE SIGNE (Operacio unaria) */
  } else if (a->kind == "-" && a->down->right == 0)
    {
      	c = GenRight (a->down, t) ||
	"mini t" + itostring (t) + " t" + itostring (t);

  /* SUMA */
  } else if (a->kind=="+") {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"addi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* RESTA (Operacio binaria) */
  } else if (a->kind=="-" && a->down->right != 0) {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"subi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* MULTIPLICACIO */
  } else if (a->kind=="*") {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"muli t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* DIVISIO */
  } else if (a->kind=="/") {
		c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"divi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* MENOR */
  } else if (a->kind == "<")
    {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"lesi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* MENOR-IGUAL */
  } else if (a->kind == "<=")
    {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"lesi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* MAJOR */
  } else if (a->kind == ">")
    {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"grti t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* MAJOR-IGUAL */
  } else if (a->kind == ">=")
    {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"grti t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* IGUALTAT */
  } else if (a->kind == "=")
    {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"equi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

/* AND */
  } else if (a->kind == "and")
    {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"land t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

/* OR */
  } else if (a->kind == "or")
    {
	c = GenRight(a->down,t)||
	GenRight(a->down->right,t+1)||
	"loor t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);

  /* NOT */
  } else if (a->kind == "not")
    {
	c = GenRight(a->down,t)||
	"lnot t" + itostring (t) + " t" + itostring (t);

  /* ACCES A STRUCT */
  } else if (a->kind == ".")
    {		
	ptype typ = symboltable[a->down->down->text].tp;

	//Nomes arriba a aqui si es NO es referenciable (acces normal x.y ref ja tractat al primer if)
	//Primer parametre es el tipus de l'acces a l'struct
	CodeGenRealParams(a->down->down,typ,c,c,t);

	//Afegim l'offset segons el camp que estem accedint
	c = c||"addi t"+itostring(t)+" "+itostring(a->down->tp->offset[a->down->right->text])+" t"+itostring(t);

  /* IDENTIFICADOR DE FUNCIO */
  } else if (a->kind == "idfunc")
    {
	codechain cpush, ckill;

	CodeGenRealParams(a->down, symboltable[a->down->text].tp, c, ckill, t);

/* AND */
  } else if (a->kind == "nand")
    {
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

 
  } else {
    		cout<<"BIG PROBLEM! No case defined in GenRight for kind "<<a->kind<<endl;
  }

  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;
}



/** GENLEFT **/
codechain GenLeft(AST *a,int t)
{
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;

  /* IDENTIFICADOR */
  if (a->kind=="ident") {
	/* No esta  al meu ambit */
	if (symboltable.jumped_scopes(a->text)>0) {  
		//Si no es un parametre (ni per valor ni per referencia)
		if (symboltable[a->text].kind!="idparref" && (symboltable[a->text].kind != "idparval")){

			c = "load static_link t"+itostring(t);
			for (int i=1;i<symboltable.jumped_scopes(a->text);i++) 
			{
				c=c||"load t"+itostring(t)+" t"+itostring(t);
			}

			c=c||
			"addi t"+itostring(t)+" offset("+symboltable.idtable(a->text)+":_"+a->text+") t"+itostring(t);

		}
		//Parametre per referencia
		else if (symboltable[a->text].kind=="idparref"){
			c="load static_link t"+itostring(t)||
			"addi t"+itostring(t)+" offset("+symboltable.idtable(a->text)+":_"+a->text+") t"+itostring(t)||
			"load t"+itostring(t)+" t"+itostring(t);
		}

		//Parametre per valor
		else if(symboltable[a->text].kind=="idparval"){

			c="load static_link t"+itostring(t);
			for (int i=1;i<symboltable.jumped_scopes(a->text);i++) {	
				c=c||"load t"+itostring(t)+" t"+itostring(t);
			}
			c=c||
			"addi t"+itostring(t)+" offset("+symboltable.idtable(a->text)+":_"+a->text+") t"+itostring(t);
		}


	/* Si esta al meu ambit */
	}else{ 
		//Es un parametre per valor
		if ((symboltable[a->text].kind=="idparval" )) {  
			// Es un tipus basic
			if(isbasickind(a->tp->kind))	c="aload _"+a->text+" t"+itostring(t);
			// Es una variable local
			else	c="load _"+a->text+" t"+itostring(t);

		//Es un parametre per referencia
		}else if(symboltable[a->text].kind=="idparref") {
			c="load _"+a->text+" t"+itostring(t);

		//Es de tipus complex (carreguem direccio)
		}else {
			c="aload _"+a->text+" t"+itostring(t);
    		}

    	}

  /* ACCES A ARRAY */
  }else if (a->kind=="["){
	int lab = newLabelNand(false);		//Reaprofito la funcio que he creat per l'apartat 1
	c=GenLeft(a->down,t)||
        GenRight(a->down->right,t+1)||

	//Modificacio en cas de ser un index negatiu
	"iload 0 t"+itostring (t+2)||
	"lesi t"+itostring(t+1)+" t"+itostring(t+2)+" t"+itostring(t+2)||	//Si es menor que 0: no salta 
	"fjmp t"+itostring(t+2)+" isneg_"+itostring(lab)||
	"iload 0 t"+itostring (t+1)||						//Si no salta, força a 0 l'index
	"etiq isneg_"+itostring(lab)||

	//Modificacio en cas de ser superior al maxim
	"iload "+itostring(a->down->tp->numelemsarray-1)+" t"+itostring (t+2)||
	"grti t"+itostring(t+1)+" t"+itostring(t+2)+" t"+itostring(t+2)||	//Si es major que el max: no salta 
	"fjmp t"+itostring(t+2)+" isgreater_"+itostring(lab)||
	"iload "+itostring(a->down->tp->numelemsarray-1)+" t"+itostring (t+1)||		//Si no salta, força a max l'index
	"etiq isgreater_"+itostring(lab)||


        "muli t"+itostring(t+1)+" "+itostring(a->down->tp->down->size)+" t"+itostring(t+1)||	//Offset segons index de l'array
        "addi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);				//Sumem l'offset

  /* ACCES A STRUCT */
  } else if (a->kind=="."){
	c=GenLeft(a->down,t)||
	"addi t"+itostring(t)+" "+itostring(a->down->tp->offset[a->down->right->text])+" t"+itostring(t);

  } else {
	cout<<"BIG PROBLEM! No case defined in GenLeft for kind "<<a->kind<<endl;
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
    for (AST *a1=a->down;a1!=0;a1=a1->right) {	//Per a tota instruccio de la llista: crida a CodeGenInstruction
      c=c||CodeGenInstruction(a1,info);
    }

  /* ASSIGNACIO */
  } else if (a->kind==":=") {

    // PART DRETA TIPUS BASIC
    if (isbasickind(a->down->tp->kind)) {
      c=GenLeft(a->down,0)||
	GenRight(a->down->right,1)||
	"stor t1 t0";

    // PART DRETA TIPUS COMPLEXE I REFERENCIABLE
    } else if (a->down->right->ref) {
      c=GenLeft(a->down,0)||
	GenLeft(a->down->right,1)||
	"copy t1 t0 "+itostring(a->down->right->tp->size);

    // PART DRETA TIPUS COMPLEXE I NO REFERENCIABLE
    } else {
      c=GenLeft(a->down,0)||
	GenRight(a->down->right,1)||
	"copy t1 t0 "+itostring(a->down->right->tp->size);
    }

  /* WRITE/WRITELN */
  } else if (a->kind=="write" || a->kind=="writeln") {
    if (a->down->kind=="string") {
	c="wris "+a->down->text;
    } else {//Exp
      c=GenRight(a->down,0)||"wrii t0";
    }
    if (a->kind=="writeln") {
      c=c||"wrln";
    }

  /* READ */
  } else if (a->kind=="read") {
    if (a->down->kind=="string") {
	c="reai "+a->down->text;
    } else {//Exp
      c=GenRight(a->down,0)||"reai t0";
    }

  /* IF */
  } else if (a->kind=="if") {
    int lab = newLabelIf(false);
    // Hi ha ELSE
    if (a->down->right->right) {
        c=GenRight(a->down,0)||
	"fjmp t0 else_"+itostring(lab)||
        CodeGenInstruction(a->down->right,"instruction")||
	"ujmp endif_"+itostring(lab)||
        "etiq else_"+itostring(lab)||
        CodeGenInstruction(a->down->right->right,"instruction")||
	"etiq endif_"+itostring(lab);
  	}
    // NO hi ha ELSE
    else {
      	c=GenRight(a->down,0)||
	"fjmp t0 endif_"+itostring(lab)||
	CodeGenInstruction(a->down->right,"instruction")||
	"etiq endif_"+itostring(lab);
    }

  /* WHILE */
  } else if (a->kind=="while") {
    int lab = newLabelWhile(false);
    	c="etiq while_"+itostring(lab)||
	GenRight(a->down,0)||
	"fjmp t0 endwhile_"+itostring(lab)||
      	CodeGenInstruction(a->down->right,"instruction")||
	"ujmp while_"+itostring(lab)||
	"etiq endwhile_"+itostring(lab);

  /* PROCEDURE */
  } else if (a->kind=="idproc") {  // call of procedure

    codechain kill;
    CodeGenRealParams(a->down,symboltable[a->down->text].tp,c,kill,0);

  /* PROCEDURE */
  } else if (a->kind=="label") {  // call of procedure

    c="etiq label_"+a->down->text;

  /* PROCEDURE */
  } else if (a->kind=="goto") {  // call of procedure
    c="ujmp label_"+a->down->text;

  } else {
    cout<<"BIG PROBLEM! No case defined in CodeGenInstruction for kind "<<a->kind<< "("<<a->text<<") in line "<<a->line<<endl;
  }


   if (offsetauxspace > maxoffsetauxspace) 	maxoffsetauxspace = offsetauxspace;
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;

  return c;
}



void CodeGenRealParams(AST *a,ptype tp,codechain &cpushparam,codechain &cremoveparam,int t)
{

  if (!a) return;

  //Per les funcions cal reservar espai pel return:
  if (symboltable[a->text].tp->kind=="function") {

        if (isbasickind(tp->right->kind)) {
            cpushparam="pushparam 0";
        } else {
            //Utilitzar aux_space: push per a que l'utilitzi la subrutina en el retorn del param
            cpushparam="aload aux_space t"+itostring(t)+" addi t"+itostring(t)+" "+itostring(offsetauxspace)+" t"+itostring(t)+" pushparam t"+itostring(t);
            offsetauxspace+=compute_size(tp->right);
            t++;
        }
    }

    ptype paux = tp->down;
    int num_param = 1;

    for (AST *a1=a->right->down; a1!=0;a1=a1->right, paux=paux->right) {
	//Carreguem a la pila segons si el parametre es per valor o per referencia
        if (paux->kind=="parval") {
            cpushparam=cpushparam || GenRight(a1,t) || "pushparam t"+itostring(t);
        } else {
            cpushparam=cpushparam || GenLeft(a1,t) || "pushparam t"+itostring(t);
        }

	num_param++;
    }

	//Fem el push del static_link segons quin escope sigui (funionalitat de indirections)
 	cpushparam=cpushparam||
	indirections(symboltable.jumped_scopes(a->text),t)||
	"pushparam t"+itostring(t);

	//Crida a la funcio
	cpushparam=cpushparam || 
	"call "+symboltable.idtable(a->text) + "_" + a->text;

	//Fem tants killparam com params teniem
	for(int i=0;i<num_param;i++)
	{
		cpushparam=cpushparam||"killparam";
	}

	//Si es funicio fem el pop del resultat, si no fem el killparam per treure'l
        if (symboltable[a->text].tp->kind=="function") {
           if (isbasickind(tp->right->kind)) {
               cpushparam=cpushparam||"popparam t"+itostring(t);
           } else {
               cpushparam=cpushparam||"killparam";
           }
       }

  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}



void CodeGenSubroutine(AST *a,list<codesubroutine> &l)
{
  codesubroutine cs;

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  string idtable=symboltable.idtable(a->down->text);
  cs.name=idtable+"_"+a->down->text;
  symboltable.push(a->sc);
  symboltable.setidtable(idtable+"_"+a->down->text);

  //To be done...
  gencodevariablesandsetsizes(a->sc,cs,a->kind=="function"?1:0);
  for (AST *a1=a->down->right->right->down;a1!=0;a1=a1->right) {
    CodeGenSubroutine(a1,l);
  }
  maxoffsetauxspace=0;
  //Reiniciem comptatge d'etiquetes 
  newLabelIf(true); 
  newLabelWhile(true);
newLabelNand(true);

  codechain returncode;
  //Fem el retorn de la funcio segons el tipus del parametre
  if (a->kind=="function") {
    if (isbasickind(a->down->right->right->right->right->tp->kind))
	returncode = GenRight(a->down->right->right->right->right,0)||
	"stor t0 returnvalue";
    else {
      returncode = GenLeft(a->down->right->right->right->right,1)||
	"load returnvalue t0"||
	"copy t1 t0 "+itostring(a->down->right->right->right->right->tp->size);
    }
  }

  //Fi de la subrutina
  cs.c=CodeGenInstruction(a->down->right->right->right)||returncode||"retu";

  if (maxoffsetauxspace>0) {
    variable_data vd;
    vd.name="aux_space";
    vd.size=maxoffsetauxspace;
    cs.localvariables.push_back(vd);
  }
  //End To be done

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
  for (AST *a1=a->down->right->down;a1!=0;a1=a1->right) {
    CodeGenSubroutine(a1,cg.l);
  }
  maxoffsetauxspace=0; 
  newLabelIf(true); 
  newLabelWhile(true);
newLabelNand(true);
  cg.mainsub.c=CodeGenInstruction(a->down->right->right)||"stop";
  if (maxoffsetauxspace>0) {
    variable_data vd;
    vd.name="aux_space";
    vd.size=maxoffsetauxspace;
    cg.mainsub.localvariables.push_back(vd);
  }
  symboltable.pop();
}

