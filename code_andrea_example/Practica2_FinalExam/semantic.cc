/**************************/
/* ===== semantic.c ===== */
/**************************/

#include <string>
#include <iostream>
#include <map>
#include <list>


using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include "ptype.hh"
#include "symtab.hh"

#define AST_FIELDS string kind;string text;int line;scope *sc;ptype tp;int ref;
#define zzcr_ast(as,attr,tttype,textt) as=new AST;as->kind=(attr)->kind;as->text=(attr)->text;as->line=(attr)->line;as->right=NULL;as->down=NULL;as->sc=0;as->tp=create_type("error",0,0);as->ref=0;
#define createASTlist (*_root)=new AST;((*_root))->kind="list";((*_root))->right=NULL;((*_root))->down=_sibling;
#define GENAST
#include "ast.h"

//Variable per controlar si hi ha error o no
int TypeError = 0;

/************************************************************************************************************************************************/
/************************************************************************************************************************************************/
void errornumparam(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": The number of parameters in the call do not match."<<endl;
}

void errorincompatibleparam(int l,int n) {
  TypeError = 1;
  cout<<"L. "<<l<<": Parameter "<<n<<" with incompatible types."<<endl;
}

void errorreferenceableparam(int l,int n) {
  TypeError = 1;
  cout<<"L. "<<l<<": Parameter "<<n<<" is expected to be referenceable but it is not."<<endl;
}

void errordeclaredident(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Identifier "<<s<<" already declared."<<endl;
}

void errornondeclaredident(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Identifier "<<s<<" is undeclared."<<endl;
}

void errornonreferenceableleft(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Left expression of assignment is not referenceable."<<endl;
}

void errorincompatibleassignment(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Assignment with incompatible types."<<endl;
}

void errorbooleanrequired(int l,string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Instruction "<<s<<" requires a boolean condition."<<endl;
}

void errorisnotprocedure(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator ( must be applied to a procedure in an instruction."<<endl;
}

void errorisnotfunction(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator ( must be applied to a function in an expression."<<endl;
}

void errorincompatibleoperator(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator "<<s<<" with incompatible types."<<endl;
}

void errorincompatiblereturn(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Return with incompatible type."<<endl;
}

void errorreadwriterequirebasic(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Basic type required in "<<s<<"."<<endl;
}

void errornonreferenceableexpression(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Referenceable expression required in "<<s<<"."<<endl;
}

void errornonfielddefined(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Field "<<s<<" is not defined in the struct."<<endl;
}

void errorfielddefined(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Field "<<s<<" already defined in the struct."<<endl;
}

void errornotalabel(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Identifier "<<s<<" is not a label."<<endl;
}
/********************************************************************************************************/
/********************************************************************************************************/

//Taula de Simbols (TS)
symtab symboltable;

//Inserir a la TS
static void InsertintoST(int line,string kind,string id,ptype tp)
{
  infosym p;

  //Ja esta declarat a l'scope actual: Error Identifier already declared
  if (symboltable.find(id) && symboltable.jumped_scopes(id)==0) errordeclaredident(line,id);	
  //No esta declarat: inserir-lo a la TS
  else {
    symboltable.createsymbol(id);
    symboltable[id].kind = kind;
    symboltable[id].tp = tp;
  }
}

//Comprovar si es tipus basic
bool isbasickind(string kind)
{
  return kind=="int" || kind=="bool" || kind=="string";
}


/*****************************************************/
/** DECLARACIO DE LES CAPSELERES DE LES FUNCIONS *****/
void check_params(AST *a);
void check_param(AST * a, ptype tp, int line, int numparam);
void insert_vars(AST *a);
void construct_struct(AST *a);
void insert_headers(AST *a);
void insert_header(AST *a);
bool check_num_params (AST * a, ptype tp, int line);
void insert_params (AST * a);
void create_header(AST *a);
void lookforlabels(AST *a);
/*****************************************************/
/*****************************************************/


/** TYPE CHECK **/
void TypeCheck(AST *a,string info="")
{
  if (!a) {
    return;
  }

/**  PROGRAM  **/
  if (a->kind=="program") {		// Inici de codi
    a->sc=symboltable.push();		// Crear scope (espai per a l'ambit)
    insert_vars(a->down->down);		// Inserim variables locals
    insert_headers(a->down->right->down);	// Comprova i insereix a la Taula de Simbols les capsaleres funcions i procedures
    TypeCheck(a->down->right);		// Comprovacio del codi de funcions i procedures
    lookforlabels(a->down->right->right);
    TypeCheck(a->down->right->right,"instruction"); // Comprovacio les instruccions
    symboltable.pop();			// Fi del programa, fem el pop de l'scope

/**  LIST  **/
  } else if (a->kind=="list") {		// At this point only instruction, procedures or parameters list is possible.
    for (AST *a1=a->down;a1!=0;a1=a1->right) {		//Fem un TypeCheck de cada element de la llista
      TypeCheck(a1,info);
    }

/**  PROCEDURE O FUNCTION  **/
  }else if (a->kind == "procedure" || a->kind == "function")
    {
      a->sc = symboltable.push ();		// Afegim l'scope a la pila i fem el canvi d'scope (current = a->scope)
      insert_params(a->down->down->down);	// Inserim els seus parametres
      insert_vars(a->down->right->down);	// Introduim les seves variables locals
      lookforlabels(a->down->right->right->right);
      insert_headers(a->down->right->right->down);		// Introduim les capsaleres dels subprocesos o subfuncions
      TypeCheck(a->down->right->right);				// TypeCheck dels subprocesos o subfuncions
      TypeCheck(a->down->right->right->right, "instruction");	// TypeCheck de la llista d'instruccions

      if (a->kind == "function")	//Si es funcio, cal comprovar el valor de retorn (coincideix amb declaracio)
	{
	  TypeCheck(a->down->right->right->right->right, "expression");	// TypeCheck del return

	  if (!equivalent_types(a->down->right->right->right->right->tp, a->tp->right))	//Si no coincideix el return i la capsalera: error
	    {
		errorincompatiblereturn(a->down->right->right->right->right->line);	//Error: Return with incompatible type
	    }
	}
	
      symboltable.pop ();	//Restaurem l'scope original

/** IF (Condicional) **/
  } else if (a->kind == "if")
    {

      TypeCheck (a->down, "expression");	// Comprovem la condicio
      if (a->down->tp->kind != "bool")		//La condicio ha de ser booleana
	{
	  errorbooleanrequired (a->line, a->kind);
	}

      TypeCheck (a->down->right, info);		// Comprovem la llista d'instruccions

      if (a->down->right->right != 0)	TypeCheck (a->down->right->right, info);	// Comprovem la llista d'instruccions de l'else (si hi ha "else")

/** WHILE (Bucle) **/
  }else if (a->kind == "while")
    {

      TypeCheck (a->down, "expression");	// Comprovem la condicio
      if (a->down->tp->kind != "bool")		//La condicio ha de ser booleana
	{
	  errorbooleanrequired(a->line, a->kind);
	}

      TypeCheck (a->down->right, info);		// Comprovem la llista d'instruccions

/**  IDENTIFICADOR  **/
  } else if (a->kind=="ident")
    {

     	if (!symboltable.find (a->text))	//Si no esta a la Taula de Simbols (no declarat)
	    {
	      errornondeclaredident (a->line, a->text);		//Error: Identifier undeclared
	    }
	  else	//Identificador SI es a la TS
	    {
	      a->tp = symboltable[a->text].tp;	// Tipus = tipus de la declaracio obtingut de la TS

	    //Si l'identificador no es procedure ni funcio: SI es referenciable
	  if (a->tp->kind == "function" || a->tp->kind == "procedure") 	a->ref = 0;
	  else  a->ref = 1;	
	     }

 /** ASSIGNACIO **/
  } else if (a->kind==":=") {

    TypeCheck(a->down, "expression");		//Comprovacio de la part esquerra de l'assignacio
    TypeCheck(a->down->right, "expression");	//Comprovacio de la part dreta de l'assignacio

    if (!a->down->ref) {	//Part esquerra NO referenciable
      errornonreferenceableleft(a->line,a->down->text);		//Error: Non Referenceable Left Part

    } else if (a->down->tp->kind!="error" && a->down->right->tp->kind!="error" && !equivalent_types(a->down->tp,a->down->right->tp)) {
      errorincompatibleassignment(a->line);			//Error: Incompatible Assignment

    } else {
      a->tp=a->down->tp;	//Assig CORRECTA: Tipus de l'assignacio = tipus de la part esquerra (que es igual al de la dreta)
    }


/** OPERACIONS ARITMETIQUES **/
  } else if (a->kind=="+" || (a->kind=="-" && a->down->right!=0) || a->kind=="*" || a->kind=="/") {

    TypeCheck(a->down);		//Comprovacio de la part esquerra de l'operacio
    TypeCheck(a->down->right);	//Comprovacio de la part dreta de l'operacio

    if ((a->down->tp->kind!="error" && a->down->tp->kind!="int") || (a->down->right->tp->kind!="error" && a->down->right->tp->kind!="int")) {

        errorincompatibleoperator(a->line,a->kind);		//Error: Incompatible Operator
    }

    a->tp=create_type("int",0,0);	//Tipus de l'operacio = int (tipus de qualsevol operacio aritmetica)
    a->ref = 0;		// Operacio aritmetica no referencianble


/** OPERACIONS BOOLEANES **/
  } else if (a->kind == "and" || a->kind == "or" || a->kind == "nand")
    {

    TypeCheck(a->down);		//Comprovacio de la part esquerra de l'operacio
    TypeCheck(a->down->right);	//Comprovacio de la part dreta de l'operacio

      if ((a->down->tp->kind != "error" && a->down->tp->kind != "bool") || (a->down->right->tp->kind != "error" && a->down->right->tp->kind != "bool"))
        {
            errorincompatibleoperator (a->line, a->kind);		//Error: Incompatible Operator
        }
      a->tp = create_type ("bool", 0, 0);	//Tipus de l'operacio = bool (tipus de qualsevol operacio booleana)
      a->ref = 0;		// Operacio booleana no referencianble


/** OPERACIONS COMPARACIO **/
  }else if (a->kind == ">=" || a->kind == "<=" || a->kind == "=" || a->kind == ">" || a->kind == "<")
    {

    TypeCheck(a->down);		//Comprovacio de la part esquerra de l'operacio
    TypeCheck(a->down->right);	//Comprovacio de la part dreta de l'operacio

      if (a->kind == "=")
	{// Igualtat, es permet bool = bool o int = int.
	    if ((a->down->tp->kind != "error" || (a->down->right->tp->kind != "bool" && a->down->right->tp->kind != "int"))
		&& (a->down->right->tp->kind != "error" || (a->down->tp->kind != "bool" && a->down->tp->kind != "int")) 
		&& (a->down->tp->kind != "int" || a->down->right->tp->kind != "int") 
		&& (a->down->tp->kind != "bool" || a->down->right->tp->kind != "bool"))

	    {
	      errorincompatibleoperator (a->line, a->kind);		//Error: Incompatible Operator
	    }
	}
	//Qualsevol comparacio excepte IGUALTAT: nomes permet int op int
      else if ((a->down->tp->kind != "error" && a->down->tp->kind != "int")
		|| (a->down->right->tp->kind != "error" && a->down->right->tp->kind != "int"))
	{
	  errorincompatibleoperator (a->line, a->kind);			//Error: Incompatible Operator
	}
      a->tp = create_type ("bool", 0, 0);	//Tipus de l'operacio = bool (tipus de qualsevol operacio de comparacio)
      a->ref = 0;		// Operacio de comparacio no referencianble


/** NOT (Operacio logica binaria) **/
  }else if (a->kind == "not")
    {
      TypeCheck (a->down);		//Comprovacio de la part dreta
	
      if (a->down->tp->kind != "error" && a->down->tp->kind != "bool")		//Ha de ser de tipus "bool"
        {
            errorincompatibleoperator (a->line, a->kind);			//Error: Incompatible Operator
        }

      a->tp = create_type ("bool", 0, 0);	//Tipus de l'operacio = bool
      a->ref = 0;		// Operacio logica no referencianble


/** CANVI DE SIGNE (Operacio algebraica unaria) **/
  }else if (a->kind == "-" && a->down->right == 0)
    {
      TypeCheck (a->down);		//Comprovacio de la part dreta

      if (a->down->tp->kind != "error" && a->down->tp->kind != "int")		//Ha de ser de tipus "int"
        {
            errorincompatibleoperator (a->line, a->kind);			//Error: Incompatible Operator
        }

      a->tp = create_type ("int", 0, 0);	//Tipus de l'operacio = bool
      a->ref = 0;		// Operacio algebraica unaria no referencianble


/** WRITELN (Escritura amb salt de linia) **/
  } else if (a->kind=="writeln") {

    TypeCheck(a->down);			// Comprovacio del parametre 

    if (a->down->tp->kind!="error" && !isbasickind(a->down->tp->kind)) {	//No es ni error ni basic...
      errorreadwriterequirebasic(a->line,a->kind);		//Error: Require Basic
    }

/** WRITE (Escritura ordinaria) **/
  }else if (a->kind == "write")
    {
      TypeCheck (a->down);		// Comprovacio del parametre 

      if (a->down->tp->kind != "error" && !isbasickind (a->down->tp->kind))	//No es ni error ni basic
	{
	  errorreadwriterequirebasic (a->line, a->kind);		//Error: Require Basic
	}

/** READ (Lectura) **/
  }else if (a->kind == "read")
    {
     TypeCheck (a->down);		// Comprovacio del parametre 

      if (a->down->ref != 1)		//Si el parametre no es referenciable
	{
	  errornonreferenceableexpression (a->line, a->kind);		//Error: Non Referenceable Expression
	}
      else if (a->down->tp->kind != "error" && !isbasickind (a->down->tp->kind))	//No es ni error ni basic
	{
	  errorreadwriterequirebasic (a->line, a->kind);		//Error: Require Basic
	}

/** ARRAY **/
  } else if (a->kind == "array")
    {
      TypeCheck (a->down);		//Comprovacio del rang
      TypeCheck (a->down->right);	//Comprovacio del tipus

      a->tp = create_type("array", a->down->right->tp, 0);	// Creacio del tipus Array amb el tipus del node d'entrada
      a->tp->numelemsarray = atoi(a->down->text.c_str());	// Assignem el rang

/** ACCES A ARRAY ([) **/
  }else if (a->kind == "[")
    {
      TypeCheck (a->down);		// Comprovacio de l'identificador 
      TypeCheck (a->down->right);	// Comprovacio de l'index

      if (a->down->tp->kind != "error" && a->down->tp->kind != "array")		//NO es ni ARRAY ni ERROR
	{
	  errorincompatibleoperator (a->line, "array[]");	//Error: Incompatible Operator
	}

      else {
	  if (a->down->tp->kind != "error")	a->tp = a->down->tp->down;	// Es ARRAY: Asignamos el tipo del array.
	  else	a->tp = create_type ("error", 0, 0);		// Es ERROR: Crear type d'error
	      
	}

      if (a->down->right->tp->kind != "error" && a->down->right->tp->kind != "int")	//L'index no es "int"
	{
	  errorincompatibleoperator (a->line, "[]");		//Error: Incompatible Operator
	}

      a->ref = a->down->ref;		//Mantenim la referencia de l'identificador exemple: id[3]

/** STRUCT **/
  } else if (a->kind=="struct") {
    construct_struct(a);	//Declaracio de l'struct i construccio dels seus camps (estructura interna)

/** ACCES A UNA ESTRUCTURA (.) **/
  }else if (a->kind == ".")
    {
      TypeCheck (a->down);		// Comprovacio del parametre 
      a->ref = a->down->ref;		// Mantenim si es referenciable l'estructura que conte el camp

    if (a->down->tp->kind!="error") {				//L'identificador de l'estructura no es error

       if (a->down->tp->kind!="struct") {			//L'identificador de l'estructura no es struct

            errorincompatibleoperator(a->line,"struct.");	//Error: Incompatible Operator

       } else if (a->down->tp->struct_field.find(a->down->right->text)==a->down->tp->struct_field.end()) {     //El camp no esta definit a l'estructura	

            errornonfielddefined(a->line,a->down->right->text);		//Error: Non Defined Field

       } else {			//Si es tot correcte: El tipus de l'acces ha de ser el mateix que el del camp accedit

         a->tp = a->down->tp->struct_field[a->down->right->text];

       }

    }

/** PARÈNTESI ( **/
  }else if (a->kind == "(")
    {

    TypeCheck(a->down);			// Comprovacio de l'identificador del procedure/funcio cridat
    a->down->ref = 0;			//Si es funcio/procedure, NO es referenciable

    if (info == "instruction")	//Es una crida a FUNCIO o PROCEDIMENT
    {
        if (a->down->tp->kind != "error" && a->down->tp->kind != "procedure")		//El tipus es correcte pero NO es un PROCEDURE
        {
        //Sera FUNCTION (ja que al cas de "(" nomes entrara si es procedure i function, i ja sabem que no es procedure)
            errorisnotprocedure (a->line);	// No es un procedure pero ha estat cridat en una expressio

        // Comprovem els parametres en cas que sigui FUNCTION (tot i ser incorrecte com a crida)
        if (a->down->tp->kind == "function")		check_params (a->down);	
        }
        else if (a->down->tp->kind == "procedure")	//Es un PROCEDURE
        {
            check_params (a->down);	// Comprovacio dels parametres del PROCEDURE
            a->kind = "idproc";		// Re-Assignem el tipus (Generacio de codi)
        }
        else
        {//Es un error que no hem de contemplar
        //cout << "Hauria de ser un PROCEDURE pero es ERROR "+a->down->kind +"  "+ a->down->text <<endl;
        }
    }
    else	//Si no es "instruction" --> ara es "expression"
    {
        if (a->down->tp->kind != "error" && a->down->tp->kind != "function")		//El tipus es correcte pero NO es un FUNCTION
        {
            errorisnotfunction(a->line);		//Error: Is Procedure

        // Comprovem els parametres en cas que sigui PROCEDURE (tot i ser incorrecte com a crida)
            if (a->down->tp->kind == "procedure"){	check_params(a->down);}
        }
        else if (a->down->tp->kind == "function")		//Es un FUNCTION
        {
            check_params(a->down);			// Comprovem els parametres
            a->tp = symboltable[a->down->text].tp->right;	// Li assignem el tipus de retorn del metode (left part del parenthesis)
            a->kind = "idfunc";		// Re-Assignem el tipus (Generacio de codi)
            a->ref = 0;	//Function NO es referenciable
        }
    }


/** PARREF **/
  } else if (a->kind == "ref") {
    a->tp=create_type("parref",0,0);

/** PARVAL **/
  } else if (a->kind == "val") {
    a->tp=create_type("parval",0,0);

/** TIPUS BASIC (int/bool) **/
  } else if (isbasickind(a->kind)) {
    a->tp=create_type(a->kind,0,0);
    a->ref = 0;		// Operacio de tipus basic no referencianble

/** STRING **/
  }else if (a->kind == "string") {
    a->tp = create_type ("string", 0, 0);
    a->ref = 0;		// String no referenciable

/** CONSTANT ENTERA (INT)**/
  } else if (a->kind=="intconst") {
    a->tp=create_type("int",0,0);
    a->ref = 0;		// Constant entera no referencianble

/** CONSTANT BOOLEANA **/
  } else if (a->kind == "true" || a->kind == "false") {
    a->tp=create_type("bool",0,0);
    a->ref = 0;		// Constant booleana no referencianble

/** LABEL **/
  } else if (a->kind == "label") {
    a->tp=create_type("label",0,0);
//     a->down->tp = create_type("label",0,0);
//     InsertintoST(a->line,"label",a->down->text,a->down->tp);		//Insercio a la Taula de Simbols

/** GOTO **/
  } else if (a->kind == "goto") {
    TypeCheck(a->down);						//Comprovacio de la variable

	if (a->down->tp->kind != "error" && a->down->tp->kind != "label")		//NO es ni ARRAY ni ERROR
	{
	  errornotalabel(a->line, a->down->text);	//Error: Incompatible Operator
	}


  } else {
    cout<<"BIG PROBLEM! No case defined in TypeCheck for kind "<<a->kind<<endl;
  }

}

void lookforlabels(AST *a)
{
  if (!a) return;
  if(a->kind == "label"){	
    a->down->tp = create_type("label",0,0);
    InsertintoST(a->line,"idlabel",a->down->text,a->down->tp);		//Insercio a la Taula de Simbols
	
}
	lookforlabels(a->down);				
	lookforlabels(a->right);	
}


/**  INSERCIO DE VARIABLES LOCALS  **/
void insert_vars(AST *a)
{
  if (!a) return;
  TypeCheck(a->down);						//Comprovacio de la variable
  InsertintoST(a->line,"idvarlocal",a->text,a->down->tp);	//Insercio a la Taula de Simbols
  insert_vars(a->right); 					//Crida recursiva per a comprovar la seguent variable
}


/**  INSERCIO DE PARAMETRES  **/
void insert_params (AST * a)
{
 string kindparam = "";

  if (!a)	return;

  TypeCheck (a->down->right);	// Comprovació del tipus del parámetre

   // "idparval" o "idparref" es el valor que s'utilitzara al tipus "kind", quan l'inserim a la TS
   // L'utlitzem tambe a la Generacio de codi
  if (a->kind == "val")		kindparam = "idparval";
  else if (a->kind == "ref")	kindparam = "idparref";

  // Inserir parametre a la taula de simbols. Parametres: linia del codi, el tipus del param, l'identificador, i el tipus
  InsertintoST (a->line, kindparam, a->down->text, a->down->right->tp);

  insert_params (a->right);		// Crida (recursiva) a insert params amb el seguent parametre
}


/** INSERCIO DE TOTES LES CAPSALERES **/
void insert_headers(AST *a)
{
  while (a!=0) {	//Bucle per recorrer totes les funcions/procedures declarats
    a->tp = create_type (a->kind, 0, 0);	//Actualitzacio del tipus del node
    insert_header(a);				//Insercio de la capsalera
    a=a->right;					//Passar a seguent capsalera
  }
}


/** INSERCIO D'UNA CAPSALERA **/
void insert_header(AST *a)
{
    string ts_id;
    create_header (a);		//Crear capsalera de la funcio/procedure

    if(a->kind=="procedure")	ts_id = "idproc";
    else if(a->kind=="function") 	ts_id = "idfunc";	
    InsertintoST (a->down->line, ts_id, a->down->text, a->tp);	//Inserir l'arbre creat a la taula de simbols (TS)
}


/** CREACIO D'UNA CAPSALERA **/
void create_header(AST *a)
{
  int count = 0;
  AST * retorn;

  if (a->kind == "procedure")	a->tp = create_type("procedure",0,0);

  else if (a->kind == "function") 	a->tp = create_type("function",0,0);

  ptype type = a->tp;
  AST *param = a->down->down->down;		// param = punter al primer parametre de la llista


  while (param != 0)		//Mentre hi hagi parametres per revisar
  {
      TypeCheck (param);		// TypeCheck del parametre
      type->right = param->tp;

      TypeCheck (param->down->right);		// TypeCheck del tipus del parametre
      type->right->down = param->down->right->tp;

      //Passem al seguent parametre
      param = param->right;
      type = type->right;

      count++;

  }

  a->tp->down = a->tp->right;
  a->tp->numparam = count;

  // La capsalera es un procedure i el retorn es nul
  if (a->kind == "procedure")	a->tp->right = 0;
  // La capsalera es una funcio i hem de guardar el tipus
  else if (a->kind == "function"){
      retorn = a->down->down->right;
      TypeCheck(retorn);		// TypeCheck del tipus de retorn
      a->tp->right = retorn->tp;	// Li assignem el tipus de retorn
  }

}


/** COMPROVACIO DELS PARAMETRES **/
void check_params(AST *a)
{

  int numparams = 1;		// Nombre de parametres de la capsalera
  bool ok;			// El nombre de parametres es correcte

  AST *nodeaux = a->right->down;	// Copia de la llista de parametres.
  ok = check_num_params(nodeaux, symboltable[a->text].tp->down, a->line);	//Coincideix el numero de parametres?

  if (ok)	//Coincideix. Fem Comprovacio de tipus
    {
      AST *aux = a->right->down;			// Copia de la llista de parametres.
      ptype t = symboltable[a->text].tp->down;	// Tipus del parametre a comprovar: consulta a la TS

      check_param(aux, t, a->line, numparams);	// Comprobacio dels parametres de forma recursiva

    } else {
      errornumparam (a->line);
    }
}


/**  COMPROVACIO D'UN PARAMETRE (recursiu)  **/
void check_param (AST * a, ptype tp, int line, int numparam)
{

  if (a==0 && tp==0)	return;

  else if (a==0 || tp ==0) {
  	errornumparam(line);
  	return;
  }
  else{
  	TypeCheck(a);
  	if (tp->kind == "parref" && a->ref == 0)	//El parametre no es referenciable pero la capsalera te param referenciable
  	{
  		errorreferenceableparam(line, numparam);		//Error: Parameter is expected to be referenceable but it is not
  	}
  //El tipus del parametre i de la capsalera no coincideix
  	if (a->tp->kind != "error" && !equivalent_types (a->tp, tp->down))
  	{
  		errorincompatibleparam (line, numparam);		//Error: Parameter with incompatible types
  	}
  numparam++;
  check_param(a->right, tp->right, line, numparam);
  }
}


/** COMPROVACIO DEL NUMERO DE PARAMETRES **/
bool check_num_params (AST * a, ptype tp, int line)
{
  //El node d'entrada a es la llista de parametres de la crida
  //tp conte la llista dels tipus dels parametres que hi ha a la TS (en la funcio/proc en questio)
  AST *params = a;	// Obtenim el punter al primer element de la llista

  if (params == 0 && tp == 0)	return true;
  else if (params == 0 || tp == 0)	return false;
  else return check_num_params (params->right, tp->right, line);	//Crida recursiva
}


/**  DECLARACIO I CONSTRUCCIO D'UN STRUCT  **/ 
void construct_struct(AST *a)
{
  AST *a1=a->down;			//Obtenim la LLISTA de camps de l'struct
  a->tp = create_type("struct",0,0);	//Tipus del node que m'han passat ha de ser STRUCT

  //BUCLE: Recorrer els CAMPS de la llista
  while (a1!=0) {
    TypeCheck(a1->down);	//Comprovem el tipus de cada CAMP

    // Funcio map.find() --> Retorna end( ), si no es troba match per a a1->text
    if (a->tp->struct_field.find(a1->text)==a->tp->struct_field.end()) {	//Si el nom del camp no esta al map, no existeix el camp
      a->tp->struct_field[a1->text]=a1->down->tp;	//S'afegeix tant al MAP de camps com a la LLISTA ids
      a->tp->ids.push_back(a1->text);
     } else {
      errorfielddefined(a1->line,a1->text);	//Error: Field already defined in the struct
    }

    a1=a1->right;		//Obtenim el seguent CAMP de la llista
  }
}


