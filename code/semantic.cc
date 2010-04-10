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

bool isbasickind(string kind) {
  return kind=="int" || kind=="bool";
}

void insert_vars(AST *a)
{
  if (!a) return;
  TypeCheck(child(a,0));
  InsertintoST(a->line,"idvarlocal",a->text,child(a,0)->tp);
  insert_vars(a->right);
}

void insert_params(AST *a) {
  if (!a) return;

  if (a->kind=="ref")
    InsertintoST(a->line,"idparref",a->down->text,a->down->right->tp);

  else if (a->kind=="val")
    InsertintoST(a->line,"idparval",a->down->text,a->down->right->tp);

  insert_params(a->right);
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
  if (a->kind == "procedure")
    a->tp = create_type("procedure",0,0);

  else if (a->kind == "function")
    a->tp = create_type("function",0,0);

  AST * param = a->down->down->down;
        /// proc->ident->list->val_or_ref
  int i=0;
  //ASTPrint(param);
  ///first param
  ptype plist = a->tp;
  while(param != 0){
      TypeCheck(param);
      plist->right = param->tp;

      ///type of param -- val V ident > int
      TypeCheck(param->down->right);
      plist->right->down = param->down->right->tp;
     // cout << "typechecked: "<< param->down->tp->kind << endl;
//      check_params(param->down->right,param->down->right->tp,param->down->right->line,i++);
      ///next param of procedure
      param = param->right;
      plist = plist->right;
	  i = i++;
  }
  a->tp->numparams = i;
  a->tp->down = plist;
  //TpPrint(a->tp);

  if (a->kind == "procedure") {
    a->tp->right = 0;
  }else if (a->kind == "function"){
	///function has a return value
  }
}


void TpPrintIndent(ptype t,string s)
{
  if (t==NULL) return;

  cout<<t->kind<< " ("<< t->numparams << ")" <<endl;

    if ((int)(t->ids.size())!=0) {
    cout<<"(";
    list<string>::iterator it=t->ids.begin();
    for (;it!=t->ids.end();) {
      cout<<"<"<<*it<<",";
      TpPrintIndent(t->struct_field[*it], s);
      cout<<">";
      it++;
      if (it!=t->ids.end()) cout<<",";
    }
    cout<<")";
  }
   if (t->down!=0) {
    cout<<s+"  \\__";
    TpPrintIndent(t->down,s+"  |"+string(t->kind.size(),' '));
  }
  if (t->right!=0) {
    cout<<s+"  \\__";
    TpPrintIndent(t->right, s+"   "+string(t->kind.size(),' '));
  }
}

void TpPrint(ptype t)
{
  cout<<endl;
  TpPrintIndent(t,"  ");
}


void insert_header(AST *a) {
  create_header(a);

  if (a->kind=="procedure")
    InsertintoST(a->line,"idproc",a->down->text,a->tp);

  else if (a->kind=="function")
    InsertintoST(a->line,"idfunc",a->down->text,a->tp);

}


void insert_headers(AST *a)
{
  while (a!=0) {
    insert_header(a);
    a=a->right;
  }
}


void TypeCheck(AST *a,string info)
{
  if (!a) {
    return;
  }

  ///cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  /** START of PROGRAM */
  if (a->kind=="program") {
    a->sc=symboltable.push();
    ///down->down
    insert_vars(child(child(a,0),0));
    ///down->right->down
    insert_headers(child(child(a,1),0));
   // symboltable.write();
    TypeCheck(child(a,1));
    TypeCheck(child(a,2),"instruction");

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
	  }
	  else {
		if ((symboltable[a->text].tp->kind == "procedure") ||
					(symboltable[a->text].tp->kind == "function")) {
			/// we can not assign a value to funcion or procedure -> is not referenceble
			a->ref = 0;
		}else{
			///other identifiers; variables etc.
			a->ref=1;
		}
		a->tp=symboltable[a->text].tp;

	  }
  }
  else if (a->kind == "procedure" || a->kind == "function"){
    ///push the context
	a->sc=symboltable.push();
   // insert_vars(child(child(a,0),0));
    insert_params(a->down->down->down);
    insert_vars(a->down->right->down);
    //            a->down->right->right->down
    insert_headers(child(child(a,2),0));
	//cout << "context "<< a->down->text << endl;
    //symboltable.write();
    TypeCheck(child(a,2)); ///blocks
    TypeCheck(child(a,3),"instruction");///instructions
	///going out of context
    symboltable.pop();
  }
  else if (a->kind=="struct") {
    construct_struct(a);
  }
  else if (a->kind==":=") {
	TypeCheck(child(a,0));
	TypeCheck(child(a,1));
	if (!child(a,0)->ref) {
		errornonreferenceableleft(a->line,child(a,0)->text);
	}
	else if (child(a,0)->tp->kind!="error" && child(a,1)->tp->kind!="error" &&
		!equivalent_types(child(a,0)->tp,child(a,1)->tp)) {
			errorincompatibleassignment(a->line);
		}
	else {
		a->tp=child(a,0)->tp;
	}
	//cout << "# " << a->line<<"- "<< child(a,0)->tp->kind << " = "<< child(a,1)->tp->kind<< endl;
  }
  else if (a->kind=="intconst") {
    a->tp=create_type("int",0,0);
  }
  else if(a->kind == "true" || a->kind == "false"){
    a->tp=create_type("bool",0,0);
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
  else if (a->kind=="ref"){
      a->tp=create_type("parref",0,0);
  }else if(a->kind == "val"){
      a->tp=create_type("parval",0,0);
  }
  else if (isbasickind(a->kind)) {
    a->tp=create_type(a->kind,0,0);
  }
  else if (a->kind=="writeln") {
    TypeCheck(child(a,0));
    if (child(a,0)->tp->kind!="error" && !isbasickind(child(a,0)->tp->kind)) {
      errorreadwriterequirebasic(a->line,a->kind);
    }
  }else if (a->kind == "("){
	/**
	* at this point we don't know if this is a procedure(function) call, or an expression
	* in case of call, we have to check if that it proprerly defined  in symboltable
	*/

	 if (info == "instruction") {
	   ///we need to check if  a->down->text is a callable

	   ///this is defition of function which will be applied to this call
	   a->tp = symboltable[a->down->text].tp;
	   if (a->tp->kind != "procedure" && a->tp->kind != "function" ){
				errorisnotprocedure(a->line);
	   }else{
		   ///procedure or function
		   /** a =
		   (
			\__ident(p)
			\__list
					\__intconst(3)	   */
		   AST * param = a->down->right->down;
		   ptype tp = a->tp;
		   /** tp is sth like this
		   procedure (1)
				\__parval (0)
						\__int (0)
		   */
		    //ASTPrint(param);
			//TpPrint(tp);
		   int i= 0;
           int n = a->tp->numparams;
		   ///we have to check number of params and their types
		   while(i < n){
			   TypeCheck(param);
			   check_params(param,tp,a->line,++i);
			   param = param->right;
			   tp = tp->right;
		   }
			//cout << a->down->text << ": "<< info << endl;
	   }
	///bool ret = symboltable.find(a->down->text);
	//if(ret){
	//	cout << "< "<<ret<< endl;
	//}
	 }else{


	 }

  } else if (a->kind==".") {
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
  else {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }
  ///symboltable.write();
 /// cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}

/**
* parametres of procedure or function
* @param *a is node of fcn call with it's params
* @param tp is definition of fcn, wich is applied in this scope,
*           we need to check is the param is same as expected
*/
void check_params(AST *a,ptype tp,int line,int numparam)
{
 //  cout << "[check_param] line: " << line << " num: " <<numparam<< " ";
 //  cout << a->tp->kind << " x "<< tp->down->down->kind << endl;
   /// according to fcn defition we should be able to assign a value
   /// to this param
   if (tp->down->kind == "parref" && !a->ref)
		errorreferenceableparam(line,numparam);

	if (!equivalent_types( a->tp,tp->down->down) && a->tp->kind !=  "error")
		errorincompatibleparam(line,numparam);
}


