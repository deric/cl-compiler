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

#include "myASTnode.hh"

#include "semantic.hh"

static const int NONE = 0;
static const int INFO = 2;
static const int DEBUG= 3;
static const int SPEC =1; 

static const int loglevel = SPEC;

#define logable(a) (a<=loglevel)


// feedback the main program with our error status
int TypeError = 0;


/// ---------- Error reporting routines --------------

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

/// ------------------------------------------------------------
/// Table to store information about program identifiers
symtab symboltable;

static void InsertintoST(int line,string kind,string id,ptype tp)
{
  infosym p;

  if (symboltable.find(id) && symboltable.jumped_scopes(id)==0) errordeclaredident(line,id);
  else {
    symboltable.createsymbol(id);
    symboltable[id].kind=kind;
    symboltable[id].tp=tp;
  }
}

/// ------------------------------------------------------------
// utility functions
 


void debugFuncName(string funcName,int start)
{
  start == 1 ?  cout<<"::::::::::::::::[debug]start_"<<funcName<<endl : cout<<"::::::::::::::::::::[debug]end_"<<funcName<<endl;
}


/// -----------------------------------------------------------
bool isbasickind(string kind) {
  return kind=="int" || kind=="bool";
}



void check_params(AST *a,ptype tp,int line,int numparam)
{
  //check the parameters of the procedure
  AST * parameters = a;
  // check the number of parameters
  // for each node of type ptype move to the right and increment
  int counter = 0;
  for(; parameters!=0 ; parameters = parameters->right , counter++);
  
  if(logable(DEBUG))  // ==> require parameters to be counted in create_header !?!

 	cout<<"Parameter Count"<< counter <<" Actual passed " << numparam << endl; 

  if(counter != numparam ) 
	errornumparam(line);
  else
	{
		//link to the first parameter
		tp = tp->down;	 
		int parameterNumber = 1 ;
		for(parameters = a ; parameters !=0 ; parameters = parameters->right, tp=tp->right,parameterNumber++)
		{
			//check the parameter
		 	TypeCheck(parameters);	
	               // check for referenceability  ( have to do this first ! )
			if(logable(DEBUG))
				cout<<"tp->kind"<<tp->kind<<endl;
			if(tp->kind == "idref" && parameters->ref == 0)
				errorreferenceableparam(line,parameterNumber);


			// check for type equality
			if(parameters->tp->kind != "error" && tp->kind != "error" && !equivalent_types(tp->down,parameters->tp))
				errorincompatibleparam(line,parameterNumber);
					
		}	
	}
}

void insert_vars(AST *a)
{
 // cout<<"insert_vars(AST *a)";
  // from 'program' recvd ident
  
  // check if there are no more vars
  if (!a) return;
  // type check the variable
  // from 'program' recvd ident : typeCheck child(ident)
  //debuga(a);
  TypeCheck(child(a,0));
  InsertintoST(a->line,"idvarlocal",a->text,child(a,0)->tp);
  insert_vars(a->right); 
}

void construct_struct(AST *a)
{
  AST *a1=child(a,0);
  a->tp=create_type("struct",0,0);

  while (a1!=0) {
    TypeCheck(child(a1,0));
    if (a->tp->struct_field.find(a1->text)==a->tp->struct_field.end()) {
      a->tp->struct_field[a1->text]=child(a1,0)->tp;
      a->tp->ids.push_back(a1->text);
     } else {
      errorfielddefined(a1->line,a1->text);
    }
    a1=a1->right;
  }
}

void create_header(AST *a)
{
  if(logable(INFO))
	  debugFuncName("create_header",1);
  //create the procedure type
//  cout<<"a->tp->kind"<<a->tp->kind<<endl;  // --> error ! create the type
  if(logable(DEBUG))
 	cout<<"creating header for " << a->kind << " and text " << a->text << endl;
  a->tp = create_type(a->kind,0,0);
  if(logable(DEBUG))  
	cout<<"a->tp->kind"<<a->tp->kind<<endl;
  //fetch the parameters
  AST *parameters = a->down->down->down;  
 
  ptype currentParam, nextParam;
  currentParam=NULL;   
  // required for typeChecking of the number of parameters
  int counter = 0;
 
  for( ; parameters!=0 ; parameters = parameters->right , counter ++ )
  {

 /*     list
        | ref or val      = parameters
        | | ident
        | | type          = parameters->down->right
 */
    if(logable(DEBUG))	
	    cout<<"a->pt"<<a->down->kind<<"-tp->down"<<a->down<<endl;// --> error ..? create the type 
//    debuga(parameters,1);
//    cout<<parameters->kind<<"down"<<parameters->down->right->kind<<endl;
//    cout<<"parameter->text"<<parameters->text<<endl;    
    nextParam = create_type("id"+parameters->kind,0,0);
    // toggle 
    if(currentParam == NULL ) a->tp->down = nextParam;
    else currentParam->right = nextParam;
 
    TypeCheck(parameters->down->right);  // check the type of the parameter

    nextParam->down = parameters->down->right->tp;
    currentParam = nextParam;         

  }
  if(a->kind=="function")
	{
		//type check the return

                /*    
		      function           = a
	               list
  	       	       | ref or val      = a->down->down
        	       | | ident
   		       | | type          
		       | return          = a->down->down->right
		 */

                if(logable(DEBUG))
			cout<<"a->down->down->right->kind"<<a->down->down->right->kind<<" at line " << a->line<<endl;
		TypeCheck(a->down->down->right);
  		// link the type of the return to the type of the function
		a->tp->right=a->down->down->right->tp;  
	}

  //set the parameter count
  a->tp->numelemsarray = counter;


}


void insert_header(AST *a)
{
  // reaching via procedure
  //debugFuncName("insert_header",1);
  // procedure
  /*
     a) proc dec : name and parameters

     procedure
     | ident ( name of the function)   = child(a,0)->kind
     | | list ( list of parameters )
     | | | val or ref
     | | | | ident
   
  */
  
  // debug
  //debuga(a);  // ==>> jp1 : a->kind = procedure child(a,0)->kind = ident : ->text = name of procedure
  // insert this type into the symbol table 
  // first create and then insert
  create_header(a);
  if(a->kind=="procedure")
	  InsertintoST(a->line,"idProcedure",child(a,0)->text,a->tp);
  if(a->kind=="function")
	  InsertintoST(a->line,"idFunction",child(a,0)->text,a->tp);
  if(logable(INFO))
	  symboltable.write();   
  //debugFuncName("insert_header",0);
}


void insert_headers(AST *a)
{
  while (a!=0) {
    insert_header(a);
    a=a->right;
  }
}

// insert the parameters
/* procedure
   | ident
   | | list
   | | | val or ref = a
   | | | | ident
   | | | | type    =  val->down->right = type
   | | | val or ref   = a->right
*/
void insert_parameters(AST *a)
{
// received val or ref
 
 // check the type and insert the param
 if(logable(INFO))
	 debugFuncName("insert_parameters",1);
// debuga(a,0);
// cout<<a->text; ?segmentation_fault? skip not ;created bummer just leave it alone 
 if(!a) return;
// debuga(a->right,0);
// cout<<"!a=false"<<a->down->right->kind<<endl;
// typecheck the type of the parameter
 if(logable(DEBUG)) 
 	cout<<"a->line"<<a->line<<endl;
  TypeCheck(a->down->right);
  // insert the line;kind,id,type
  InsertintoST(a->line,"idParameter"+a->kind,a->down->text,a->down->right->tp);
  // the next parameter
   insert_parameters(a->right);  
 
}


void TypeCheck(AST *a,string info)
{
  if (!a) {
    return;
  }
 //symboltable.write();
 // cout<<"Starting with node \""<<a->kind<<"\""<<"and at line"<<a->line<<endl;
  if (a->kind=="program") {
    // push a new scope into the symbol table
    // and get the pointer to that scope
    a->sc=symboltable.push();
    // insert the variables
    /* program
     | list
     | | ident
     | | | int
     | | ident
     | list
     | | procedure  
     | | | ident
     | | | | list ( of parameters)  
     | | | | | val or ref        = procedure->down->down->down
     | | | list (of variables)
     | | | | vars                = procedure->down->right->down
     | | | list  (of procedures) 
     | | | | procedure           = procedure->down->right->right->down
     | | | list (of instructions) 
     | | | | instruction         = procedure->down->right->right->right->down
  
    */
   if(logable(DEBUG))
   	 cout<<"**********************[debug]inserting vars for program"<<endl;
    insert_vars(child(child(a,0),0));
       // procedureCheck 
   if(logable(DEBUG))
   	 cout<<"**********************[debug]inserting headers (procedures) for program"<<endl;
    insert_headers(child(child(a,1),0));
   if(logable(DEBUG))
   	 cout<<"**********************[debug]typechecking the header (procedures)"<<endl;
    TypeCheck(child(a,1));
    // end
   // cout<<"**********************[debug]typechecking the instructions of the programs"<<endl;
    TypeCheck(child(a,2),"instruction");

    symboltable.pop();
  } 
  else if(a->kind=="procedure")
  {
   // create a new scope for procedure
   //cout<<"**********************[debug]procedure at line"<<a->line<<endl;
   a->sc = symboltable.push();
   // child => a->down 
  // symboltable.write();

  // cout<<"child(child)"<< a->down->down->kind<<endl;//->down;
   // insert the parameters 
   insert_parameters( a->down->down->down); // procedure->ident->list->val or ref 
   // insert the variables of the procedure 
   insert_vars(a->down->right->down); 	
   // insert the headers (procedures ?!)
   insert_headers(a->down->right->right->down);
   // type check the procedures and then the instructions just like above
   TypeCheck(a->down->right->right);
   TypeCheck(a->down->right->right->right,"instruction");
   symboltable.pop();


   
  }
  else if (a->kind=="function")
	{

		/*
        function  		= a
        | ident
        | | list ( of parameters)  
        | | | val or ref        = a->down->down->down
        | list (of variables)
        | | vars                = a->down->right->down
        | list  (of block) 
        | | procedure           = a->down->right->right->down
        | list (of instructions) 
        | | instruction         = a->down->right->right->right->down
        | ident
        | | type                = a->down->right->right->right->right = child(a,4)
		*/
		a->sc=symboltable.push();
		//insert the parameters
		insert_parameters(a->down->down->down);
		//insert the variables of the function
		insert_vars(a->down->right->down);
		//insert the headers
		insert_headers(a->down->right->right->down); 
                // type check the procedures and then the instructions just like above
		TypeCheck(child(a,2));
		TypeCheck(child(a,3)); 
		// check the return of the function
		TypeCheck(child(a,4));
                // check the return type is the same as what was declared
                if(logable(DEBUG))
			cout<<"a->tp->right->kind"<<a->tp->right->kind<<endl;
		if(!equivalent_types(child(a,4)->tp,a->tp->right))		
			errorincompatiblereturn(child(a,4)->line);
		symboltable.pop();

                
	}
  else if (a->kind=="list") {
    // At this point only instruction, procedures or parameters lists are possible.
    for (AST *a1=a->down;a1!=0;a1=a1->right) {
      TypeCheck(a1,info);
    }
  } 
  else if (a->kind=="ident") {
    if (!symboltable.find(a->text)) {
      errornondeclaredident(a->line, a->text);
//      a->ref=0;
    } 
    else {
      a->tp=symboltable[a->text].tp;
      // procedure and functions cannot be left expressions
      if(a->tp->kind == "procedure" || a->tp->kind == "function") 
	a->ref=0;
      else
	a->ref=1;
    }
  } 
  else if (a->kind=="struct") {
    construct_struct(a);
  }
  
// arrays 
  else if (a->kind == "array")
	{
	 /*
	    | array
	    | | intconst
	    | | type
	*/
    	TypeCheck(child(a,0));
	TypeCheck(child(a,1));
	
	a->tp = create_type("array",child(a,1)->tp,0);
        if(logable(DEBUG))
		cout<<"a->down->text.c_str()"<<a->down->text<<endl;
        a->tp->numelemsarray = atoi(a->down->text.c_str());
	
	}

// accessing an array
  else if ( a->kind == "[")
	{
	  // Check the Id
	  TypeCheck(child(a,0));
	  // Check the index
	  TypeCheck(child(a,1));
	  
          if(child(a,0)->tp->kind != "error" && child(a,0)->tp->kind != "array")
		errorincompatibleoperator(a->line,"array[]");
	  // if its an array get the assignment of the type of the array
	  if(child(a,0)->tp->kind == "array")
		a->tp = child(a,0)->tp->down;   
	  // the index is not int
	  if(child(a,1)->tp->kind != "error" && child(a,1)->tp->kind != "int")	
		errorincompatibleoperator(a->line,"[]");
	  // maintain the reference to the Id
          a->ref = child(a,0)->ref;

	}
  
  else if (a->kind==":=") {
//    symboltable.write();
    if(logable(DEBUG))
	cout<<"at line"<<a->line<<" child(a,0)"<<a->down->ref<<"child(a,1)"<<a->down->right->kind<<endl;
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));
     if(logable(DEBUG))
	cout<<"!child(a,o)->ref) "<<!child(a,0)->ref<<endl;

    if (!child(a,0)->ref) {
     if(logable(DEBUG))
	cout<<"!child(a,o)->ref) "<<!child(a,0)->ref<<endl;
      errornonreferenceableleft(a->line,child(a,0)->text);
    }
    else if (child(a,0)->tp->kind!="error" && child(a,1)->tp->kind!="error" &&
	     !equivalent_types(child(a,0)->tp,child(a,1)->tp)) {
      errorincompatibleassignment(a->line);
    } 
    else {
      a->tp=child(a,0)->tp;
    }
  } 
  else if (a->kind=="intconst") {
    a->tp=create_type("int",0,0);
    // non-referenceable (not a left expression)
 //   a->ref=0;
  }
  //added for true / false
  else if ( a->kind=="true" || a->kind =="false")
 {
   a->tp = create_type("bool",0,0);
   a->ref = 0;
 } 
  else if (a->kind=="+" || (a->kind=="-" && child(a,1)!=0) || a->kind=="*"
	   || a->kind=="/") {
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));
    if ((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="int") ||
	(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="int")) {
      errorincompatibleoperator(a->line,a->kind);
    }
    a->tp=create_type("int",0,0);
  }
  // comparison operators
  else if (a->kind == ">" || a->kind == "<")
	{
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		if((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="int") ||
			(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="int"))
		errorincompatibleoperator(a->line,a->kind);
		a->tp=create_type("bool",0,0);
	}
  
  
  //boolean operations
  else if ( a->kind== "and" || a->kind == "or")
	{
		// and expression1 expression2
		TypeCheck(a->down);
		TypeCheck(a->down->right);
		if(( a->down->tp->kind!="error" && a->down->tp->kind!="bool") ||
			(a->down->right->tp->kind!="error" && a->down->right->tp->kind!="bool"))
			errorincompatibleoperator(a->line,a->kind);

		a->tp = create_type("bool",0,0);
	}

  // equality
  else if ( a->kind == "=") 
	{
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		
		if(child(a,0)->tp->kind!="error" && child(a,1)->tp->kind!="error" && !equivalent_types(child(a,0)->tp,child(a,1)->tp))
			errorincompatibleoperator(a->line,a->kind);
		
		// equality is only allowed for bool and int
		if( (child(a,0)->tp->kind == "array" && child(a,1)->tp->kind == "array") ||
			(child(a,0)->tp->kind == "struct" && child(a,1)->tp->kind == "struct"))
			errorincompatibleoperator(a->line,a->kind);

		a->tp = create_type("bool",0,0);
	}

  // not operation
  else if ( a->kind == "not")
	{
		// not expression
		TypeCheck(a->down);
		if(a->down->tp->kind != "error" && a->down->tp->kind!= "bool")
			errorincompatibleoperator(a->line,a->kind);
		
		a->tp = create_type("bool",0,0);
	}
  // unary operator
  else if ( a->kind == "-" && a->down->right==0)
	{
		TypeCheck(a->down);
		// applicable to int
		if(a->down->tp->kind != "error" && a->down->tp->kind != "int")	
			errorincompatibleoperator(a->line,a->kind);
		a->tp = create_type("int",0,0);
	}

  else if (isbasickind(a->kind)) {
    a->tp=create_type(a->kind,0,0);
  }
 // added read,write
  else if (a->kind=="writeln" ) {
    TypeCheck(child(a,0));
    if (child(a,0)->tp->kind!="error" && !isbasickind(child(a,0)->tp->kind)) 
      errorreadwriterequirebasic(a->line,a->kind);
        
  }
  else if (a->kind == "write")
	{
	 TypeCheck(child(a,0));
         if (child(a,0)->tp->kind!="error" && !isbasickind(child(a,0)->tp->kind)) 
      		errorreadwriterequirebasic(a->line,a->kind);
	}
else if (a->kind == "read")
	{
	 TypeCheck(child(a,0));
	// needs to be referencable
        if(a->kind == "read" && child(a,0)->tp->kind != "error" && child(a,0)->ref == 0)
		errornonreferenceableexpression(a->line,a->kind);
	
        else if (child(a,0)->tp->kind!="error" && !isbasickind(child(a,0)->tp->kind)) 
     	    errorreadwriterequirebasic(a->line,a->kind);

	}

  else if (a->kind==".") {
    TypeCheck(child(a,0));
    a->ref=child(a,0)->ref;
    if (child(a,0)->tp->kind!="error") {
      if (child(a,0)->tp->kind!="struct") {
	errorincompatibleoperator(a->line,"struct.");
      }
      else if (child(a,0)->tp->struct_field.find(child(a,1)->text)==
	       child(a,0)->tp->struct_field.end()) {
	errornonfielddefined(a->line,child(a,1)->text);
      } 
      else {
	a->tp=child(a,0)->tp->struct_field[child(a,1)->text];
      }
    }
  } 
  // for the case of parentheses
  else if (a->kind == "(")
  {
    //check for declaration
    //check for procedure calls of the type p(3) p(true) etc.
    if (logable(DEBUG)) 
	cout<<"*************************[debug] a->kind '('"<<a->down->kind<<" a->text "<<a->down->text<<" at line"<<a->line<<endl;
    //if the identifier is not on the symbol table then it is an error
    if(!symboltable.find(a->down->text))
   	errornondeclaredident(a->line,a->down->text);
    
    else if (info=="instruction")
    {
       // arrived via a procedure : instruction in a procedure
       // fetch the identifier and its type from the symbol table and check it is a procedure
       // the name of the procedure is the ident ( a->down->text) 
       a->tp = symboltable[a->down->text].tp;
       //check if its kind is a procedure
       if(a->tp->kind!="procedure")
       {
        // this isnt a procedure
	errorisnotprocedure(a->line);
       // cout<<"a->down->right->down"<<a->down->right->down->kind<<endl;
        if(a->tp->kind == "function")
           check_params(a->down->right->down,a->tp, a->line, a->tp->numelemsarray);
       }
       else
       {
	// if it is a procedure then check the parameters that are being passed to it 
        if(logable(DEBUG))
	{
              /*
   		    | (                     = a
		    | | ident(p)            = a->down(->text = p used above) 
		    | | list
                    | | | intconst(3)       = a->down->right->down
           */
		
  		cout<<"a->down->right->down->kind"<<a->down->right->down->kind<<endl;
		cout<<"a->tp"<<a->tp<<endl;
                cout<<"a->line"<<a->line<<endl;
		cout<<"a->tp->numelemsarray"<<a->tp->numelemsarray<<endl;
        }
         // check the parameters of the procedure         
         check_params(a->down->right->down,a->tp, a->line,a->tp->numelemsarray); 
       }
   }
   else
    {
	//not an instruction but a function
	a->tp = symboltable[child(a,0)->text].tp;
	if(a->tp->kind != "function")
		{
			errorisnotfunction(a->line);
			if(a->tp->kind == "procedure")
				{
					check_params(a->down->right->down,a->tp, a->line,a->tp->numelemsarray);
					a->tp = create_type("error",0,0);
				}
			
		}
	else
		{
			// assign the return type to the function
			check_params(a->down->right->down,a->tp,a->line,a->tp->numelemsarray);
			a->tp = a->tp->right;
		}
          
    }
    
  }
  else if (a->kind=="if")
	{	
		/*
 		   | if 			     = a
                   | | condition
                   | | list of instructions (then)   = a->down->right
		   | | list of instructions (else)   = a->down->right->right
		*/
		// typeCheck the condition
		TypeCheck(a->down);
		//if should be followed by a boolean condition
		if(a->down->tp->kind != "error" && a->down->tp->kind != "bool")
			errorbooleanrequired(a->line,a->kind);
		
		//check the instructions for the then
		TypeCheck(a->down->right,"instruction");
 		
		if(a->down->right->right != 0 ) // there exist else instructions
			TypeCheck(a->down->right->right,"instruction");
	}
  else if (a->kind == "while")
	{
		/*
		  | while
		  | | condition
		  | | list of instructions    = a->down->right
		*/
		// typeCheck the condition
		TypeCheck(a->down);
		if(a->down->tp->kind != "error" && a->down->tp->kind != "bool")
			errorbooleanrequired(a->line,a->kind);
		//Type check the while instructions
		TypeCheck(a->down->right,"instruction");
	} 
  
  else {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }

  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}
