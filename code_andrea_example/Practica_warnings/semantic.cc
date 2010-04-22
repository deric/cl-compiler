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

#define AST_FIELDS string kind;string text;int line;scope *sc;ptype tp;int ref;int hay_valor;int valor;
#define zzcr_ast(as,attr,tttype,textt) as=new AST;as->kind=(attr)->kind;as->text=(attr)->text;as->line=(attr)->line;as->right=NULL;as->down=NULL;as->sc=0;as->tp=create_type("error",0,0);as->ref=0;hay_valor=0;valor=0;
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

void errordividezero(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Division por cero. "<<endl;
}

/************************************************************************************************************************************************/
/************************************************************************************************************************************************/


//Taula de Simbols (TS)
symtab symboltable;

//Inserir a la TS
static void InsertintoST(int line,string kind,string id,ptype tp)
{
  infosym p;

  //Ja esta declarata l'scope actual: Error Identifier already declared
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
bool check_num_params (AST * a, int line, int numparhead);
void insert_params (AST * a);
void create_header(AST *a);
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
    TypeCheck(a->down->right);		// Comprovacio (TypeCheck) del codi de funcions i procedures
    TypeCheck(a->down->right->right,"instruction");
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
      insert_params(a->down->down->down);	// Introducimos los parametros de la funcion o procedimiento
      insert_vars(a->down->right->down);	// Introducimos las variables locales
      insert_headers(a->down->right->right->down);	// Introducimos las cabeceras de los subprocesos o subfunciones
      TypeCheck(a->down->right->right);	// TypeCheck de los subprocesos
      TypeCheck(a->down->right->right->right, "instruction");	// TypeCheck de las instrucciones

      if (a->kind == "function")
	{			// En caso de ser funcion tenemos un return que ha de coincidir con el declarado
	  TypeCheck (a->down->right->right->right->right, "expression");	// TypeCheck del return

	  if (!equivalent_types(a->down->right->right->right->right->tp, a->tp->right))
	    {			// El tipo del return no coincide con el de la cabecera
		errorincompatiblereturn(a->down->right->right->right->right->line);
	    }
	}
// Restauramos el Scope original
      symboltable.pop ();

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
	      if((a->tp->kind!="function") && (a->tp->kind!="procedure"))	a->ref = 1;	//Si no es procedure ni funcio: SI referenciable
	     }

 

/** STRUCT **/
  } else if (a->kind=="struct") {
    construct_struct(a);	//Declaracio de l'struct i construccio dels seus camps (estructura interna)

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

/** CONSTANT ENTERA (INT)**/
  } else if (a->kind=="intconst") {
    a->tp=create_type("int",0,0);
    a->hay_valor=1;
    a->valor=atoi(a->text);

/** CONSTANT BOOLEANA **/
  } else if (a->kind == "true" || a->kind == "false") {
    a->tp=create_type("bool",0,0);

/** OPERACIONS ARITMETIQUES **/
  } else if (a->kind=="+" || (a->kind=="-" && a->down->right!=0) || a->kind=="*" || a->kind=="/") {

    TypeCheck(a->down);		//Comprovacio de la part esquerra de l'operacio
    TypeCheck(a->down->right);	//Comprovacio de la part dreta de l'operacio

    if((a->kind=="-") && (a->down->hay_valor==1) && (a->down->right->hay_valor==1) && (a->down->tp->kind=="int") && (a->down->right->tp->kind=="int")){
		a->hay_valor = 1;
		a->valor = atoi(a->down->text)-atoi(a->down->right->text);
    }

    if((a->kind=="/") && (a->down->hay_valor==1) && (a->down->right->hay_valor==1) && (a->down->tp->kind=="int") && (a->down->right->tp->kind=="int")){
		a->hay_valor = 1;
		a->valor = atoi(a->down->text)/atoi(a->down->right->text);
		if (a->down->right->text=="0")	errordividezero(a->line);
    }

    if ((a->down->tp->kind!="error" && a->down->tp->kind!="int") || (a->down->right->tp->kind!="error" && a->down->right->tp->kind!="int")) {
      		errorincompatibleoperator(a->line,a->kind);		//Error: Incompatible Operator
    }

    a->tp=create_type("int",0,0);	//Tipus de l'operacio = int (tipus de qualsevol operacio aritmetica)

/** OPERACIONS BOOLEANES **/
  } else if (a->kind == "and" || a->kind == "or")
    {

    TypeCheck(a->down);		//Comprovacio de la part esquerra de l'operacio
    TypeCheck(a->down->right);	//Comprovacio de la part dreta de l'operacio

      if ((a->down->tp->kind != "error" && a->down->tp->kind != "bool") || (a->down->right->tp->kind != "error" && a->down->right->tp->kind != "bool"))
	{
	  errorincompatibleoperator (a->line, a->kind);		//Error: Incompatible Operator
	}
      a->tp = create_type ("bool", 0, 0);	//Tipus de l'operacio = bool (tipus de qualsevol operacio booleana)

/** OPERACIONS COMPARACIO **/
  }else if (a->kind == ">=" || a->kind == "<=" || a->kind == "=" || a->kind == ">" || a->kind == "<")
    {

    TypeCheck(a->down);		//Comprovacio de la part esquerra de l'operacio
    TypeCheck(a->down->right);	//Comprovacio de la part dreta de l'operacio

      if (a->kind == "=")
	{			// Igualtat, es permet bool = bool o int = int.
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

/** NOT (Operacio logica binaria) **/
  }else if (a->kind == "not")
    {
      TypeCheck (a->down);		//Comprovacio de la part dreta
	
      if (a->down->tp->kind != "error" && a->down->tp->kind != "bool")		//Ha de ser de tipus "bool"
	{
	  errorincompatibleoperator (a->line, a->kind);			//Error: Incompatible Operator
	}

      a->tp = create_type ("bool", 0, 0);	//Tipus de l'operacio = bool

/** CANVI DE SIGNE (Operacio algebraica numerica) **/
  }else if (a->kind == "-" && a->down->right == 0)
    {
      TypeCheck (a->down);		//Comprovacio de la part dreta

      if (a->down->tp->kind != "error" && a->down->tp->kind != "int")		//Ha de ser de tipus "int"
	{
	  errorincompatibleoperator (a->line, a->kind);			//Error: Incompatible Operator
	}

      a->tp = create_type ("int", 0, 0);	//Tipus de l'operacio = bool

/** TIPUS BASIC (int/bool) **/
  } else if (isbasickind(a->kind)) {
    a->tp=create_type(a->kind,0,0);

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

/** STRING **/
  }else if (a->kind == "string")
    {
	a->tp = create_type ("string", 0, 0);

/** ACCES A UNA ESTRUCTURA (.) **/
  }else if (a->kind == ".")
    {
      TypeCheck (a->down);		// Comprovacio del parametre 
      a->ref = a->down->ref;		// Mantenim si es referenciable l'estructura que conte el camp

    if (a->down->tp->kind!="error") {
       if (a->down->tp->kind!="struct") {
            errorincompatibleoperator(a->line,"struct");
       } else if (a->down->tp->struct_field.find(a->down->right->text)==a->down->tp->struct_field.end()) {
            errornonfielddefined(a->line,a->down->right->text);
       } else {
         a->tp = a->down->tp->struct_field[a->down->right->text];
       }

    }

  /*    if (a->down->tp->kind != "error" && a->down->tp->kind != "struct")	//L'identificador de l'estructura no es struct
	{
	  errorincompatibleoperator (a->line, "struct.");		//Error: Incompatible Operator
	  a->tp = create_type ("error", 0, 0);				//Assignem tipus error
	}

      else if (a->down->tp->kind == "error")			//L'identificador de l'estructura va provocar un error (p.ex. no existeix)
	{
	  a->tp = create_type ("error", 0, 0);			//Assignem tipus error
	}

      else if (a->down->tp->struct_field.find (a->down->right->text) == a->down->tp->struct_field.end ())	//El camp no esta definit a l'estructura
	{
	  errornonfielddefined (a->line, a->down->right->text);		//Error: Non Defined Field
	  a->tp = create_type ("error", 0, 0);				//Assignem tipus error
	}

      else		//Si es tot correcte: El tipus de l'acces ha de ser el mateix que el del camp accedit
	{
	  a->tp = a->down->tp->struct_field[a->down->right->text];
	}*/

/** PARÈNTESI ( **/
  }else if (a->kind == "(")
    {
	//if (info != "instruction"){

	TypeCheck(a->down);			// Comprovacio de l'identificador del procedure/funcio cridat
	a->down->ref = 0;			//Si es funcio/procedure, NO es referenciable

	if (info == "instruction")	//Es una crida a FUNCIO o PROCEDIMENT
	{
	      if (a->down->tp->kind != "error" && a->down->tp->kind != "procedure")		//El tipus es correcte pero NO es un PROCEDURE
		{
		  //Sera FUNCTION (ja que al cas de "(" nomes entrara si es procedure i function, i ja sabem que no es procedure)
		  errorisnotprocedure (a->line);	// No es un procedure pero ha estat cridat en una expressio
		  a->tp = create_type ("error", 0, 0);		//El tipus del node es ERROR
		  if (a->down->tp->kind == "function")		check_params (a->down);	// Comprovem els parametres en cas que sigui FUNCTION (tot i ser incorrecte com a crida)
		}
	     else if (a->down->tp->kind == "procedure")	//Es un PROCEDURE
		{
		  check_params (a->down);	//Comprovacio dels parametres del PROCEDURE
		}
	      else
		{//Crec que es un error del qual hem de passar
		//cout << "Hauria de ser un PROCEDURE pero es ERROR "+a->down->kind +"  "+ a->down->text <<endl;
		}
	}
      else	//Si no es "instruction" --> ara es "expression"
	{
   
	      if (a->down->tp->kind != "error" && a->down->tp->kind != "function")		//El tipus es correcte pero NO es un FUNCTION
		{
		  errorisnotfunction(a->line);		//Error: Is Procedure
		  a->tp = create_type ("error", 0, 0);		//El tipus del node es ERROR
		  if (a->down->tp->kind == "procedure")	check_params(a->down);	// Comprovem els parametres en cas que sigui PROCEDURE (tot i ser incorrecte com a crida)
		}
	      else if (a->down->tp->kind == "function")	//Es un FUNCTION
		{
		  check_params(a->down);			// Comprovem els parametres
		  a->tp = symboltable[a->down->text].tp->right;	// Li assignem el tipus de retorn del metode (left part del parenthesis)
		}
	    }

  } else {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }

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
 string kindparam;

  if (!a)
    return;

  TypeCheck (a->down->right);	// Comprovació del tipus del parámetre

   // "parval" o "parref" es el valor que s'utilitzara al tipus "kind", quan l'inserim a la TS
  if (a->kind == "val")		kindparam = "parval";
  else if (a->kind == "ref")	kindparam = "parref";

  // Inserir parametre a la taula de simbols. Parametres: linia del codi, el tipus del param, l'identificador, i el tipus
  InsertintoST (a->line, kindparam, a->down->text, a->down->right->tp);
  
  insert_params (a->right);		// Crida (recursiva) a insert params amb el seguent parametre
}


/** INSERCIO DE TOTES LES CAPSALERES **/
void insert_headers(AST *a)
{
  while (a!=0) {	//Bucle per recorrer totes les funcions/procedures declarats
    a->tp = create_type (a->kind, 0, 0);	//Actualitzacio del tipus del node
    insert_header(a);
    a=a->right;
  }
}


/** INSERCIO D'UNA CAPSALERA **/
void insert_header(AST *a)
{
    create_header (a);		//Crear capsalera de la funcio/procedure
    InsertintoST (a->down->line, a->kind, a->down->text, a->tp);	//Inserir l'arbre creat a la taula de simbols (TS)
}


/** CREACIO D'UNA CAPSALERA **/
void create_header(AST *a)
{
  
	AST *a1 = a->down;		// a1 = identificador del procedure
  	AST *param = a1->down;		// param = punter al primer parametre de la llista
  	AST *access = param->down;	//access = tipus del parametre (parref/parval)

	if (access != 0)	// tipus de parametre no es nul
	    {

		access->tp = create_type ("", 0, 0);	// Creamos la estructura de un parametro.
		a->tp->down = access->tp;	// La estructura principal apunta al siguiente parametro. 
		while (access != 0)
			{			// Mientras queden parametros por revisar.
		  	AST *paramid = access->down;	// Apunta al identificador del parametro.
		  	TypeCheck (paramid->right);	/* checkeamos el tipo de los parametros */
		  /* Usamos un map para almacenar los diferentes parametros y el map requiere que los strings sean unicos o la informacion se sobreescribe en el mismo nodo, por ese motivo almacenamos el identificador del parametro ya que deberia ser unico */
		  	a->tp->struct_field[paramid->text] = paramid->right->tp;	// Guardamos en la estructura de la cabecera los nombres de los parametros para poder indexar
		 	 a->tp->ids.push_back (paramid->text);
		  	access->tp->struct_field[access->kind] = paramid->right->tp;	// Insertamos a la map el valor del tipo de la variable 
		 	 access->tp->ids.push_back (access->kind);	// insertamos val/ref a la lista 
		if (access->right != 0)
			{
			access->right->tp = create_type ("", 0, 0);	// Creamos la estructura del proximo parametro.
			access->tp->right = access->right->tp;	// Apuntamos a la estructura del siguiente parametro.
		   	 }
		access = access->right;	//Cambiamos de parametro y reiniciamos el bucle.
		}
	    }
	  if (a->kind == "function")
	    {				// La cabecera corresponde a una función por lo que hemos de guardar el tipo.
	      ptype tp;
	      TypeCheck (param->right);	// Comprobamos el tipo del return.
	      tp = param->right->tp;
	      a->tp->right = tp;	// Asignamos el tipo del return.
	    }

}


/** COMPROVACIO DELS PARAMETRES **/
void check_params(AST *a)
{

  int numparams;		// Nombre de parametres de la capsalera
  bool ok;			// El nombre de parametres es correcte

  numparams = symboltable[a->text].tp->ids.size();	// Obtenim el numero de parametres de la capsalera

  ok = check_num_params(a, a->line, numparams);	//Coincideix el numero de parametres?

  if (ok)	//Coincideix. Fem Comprovacio de tipus
    {
      int numparam = 1;
      AST *aux = a->right->down;			// Copia de la llista de parametres.
      ptype t = symboltable[a->text].tp->down;	// Tipus del parametre a comprovar: consulta a la TS

      while (aux != 0 && t != 0)	//Bucle per recorrre els parametres
	{
	  check_param(aux, t, a->line, numparam);	// Comprobacio del parametre
	  aux = aux->right;	// Avancem al proxim parametre
	  t = t->right;		// Avancem en la declaracio al proxim parametre (TS)
	  numparam++;
	}
    }
}


/**  COMPROVACIO D'UN PARAMETRE  **/
void check_param (AST * a, ptype tp, int line, int numparam)
{

  AST *a1 = a;			//Copia que conté els parametres de la crida
  ptype tp1 = tp;		//Copia que conté el tipus del parametre en la capsalera

  if (tp1->ids.front() == "ref" && a1->ref == 0)	//El parametre no es referenciable pero la capsalera te param referenciable
    {
      errorreferenceableparam(line, numparam);
    }
  //El tipus del parametre i de la capsalera no coincideix
  if (a1->tp->kind != "error" && !equivalent_types (a1->tp, tp1->struct_field[tp1->ids.front ()]))
    {
      errorincompatibleparam (line, numparam);
    }
}


/** COMPROVACIO DEL NUMERO DE PARAMETRES **/
bool check_num_params (AST * a, int line, int numparhead)
{
  //El node d'entrada es el del procedure o funcio
  AST *params = a->right->down;	// Obtenim la llista de parametres de la crida (punter al primer element)
  int num;			// Variable per comptar el numero d'elements de la llista de params
  bool ok = true;

  num = 0;
  while (params != 0)		//Recorregut de tots els params de la llista (fins no buit)
    {
      TypeCheck (params);	//Comprovem el parametre
      num++;
      params = params->right;			//Avancem al seguent parametre
    }

  if (num != numparhead)	//Si el numero de parametres es diferent de numparhead (num de params de la capsalera)
    {
      errornumparam(line);	//Error: Number of params do not match
      ok = false;
    }

  return ok;
}



/**  DECLARACIO I CONSTRUCCIO D'UN STRUCT  **/ 
void construct_struct(AST *a)
{
  AST *a1=a->down;			//Obtenim la LLISTA de camps de l'struct
  a->tp = create_type("struct",0,0);	//Tipus del node que m'han passat es STRUCT

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


