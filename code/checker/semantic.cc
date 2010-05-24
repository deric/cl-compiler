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
  return kind=="int" || kind=="bool"||kind=="string";
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

void TpPrintIndent(ptype t,string s)
{
  if (t==NULL) return;

  cout<<t->kind<< endl;

   /* if ((int)(t->ids.size())!=0) {
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
  }*/
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
/*
void parse_param(AST *param){
	int i=0;
	if(param == 0) return;
	 parse_param(param->right);

	  ///type of param
     TypeCheck(param->down->right);
	  /// val or ref
	 if(param->right == 0){
	 ///this is the only parameter
		if(param->kind=="val"){
			param->tp=create_type("parval", param->down->right->tp, 0);
		}else if(param->kind=="ref"){
			param->tp=create_type("parref", param->down->right->tp, 0);
		}
	 }else{
		if(param->kind=="val"){
			param->tp=create_type("parval", param->down->right->tp, param->right->tp);
		}else if(param->kind=="ref"){
			param->tp=create_type("parref", param->down->right->tp, param->right->tp);
		}
	 }
	///result of typecheck
  //  param->tp->down->down = param->down->right->tp;
	//cout << "tp "<<endl;
	//TpPrint(param->tp);
}*/

void parse_param(AST* a){
    if(a==0) return;
	///firstly reach the last one
    parse_param(a->right);
    TypeCheck(a->down->right);

    if(a->right==0){
		if(a->kind=="val")
			a->tp=create_type("parval", a->down->right->tp, 0);
		else if(a->kind=="ref")
			a->tp=create_type("parref", a->down->right->tp, 0);
	}else {
		if(a->kind=="val")
			a->tp=create_type("parval", a->down->right->tp, a->right->tp);
		else if(a->kind=="ref")
			a->tp=create_type("parref", a->down->right->tp, a->right->tp);
	}
}

void create_header(AST *a)
{
 ///first param
  AST * param = a->down->down->down;

  if(param != 0){
	parse_param(param);
  }
}


void insert_header(AST *a) {
	if (a->kind=="procedure"){
		//a->tp = create_type("procedure",0,0);
		create_header(a);
		///we have some parameters
		if(a->down->down->down != 0){
			a->tp = create_type("procedure",a->down->down->down->tp,0);
			a->tp->numparams = count_params(a->down->down->down);
		}else
			a->tp = create_type("procedure",0,0);
		//a->tp->down = a->down->down->down->tp;
		//TpPrint(a->down->down->down->tp);
		///insert into symbol table
		InsertintoST(a->line,"idproc",a->down->text,a->tp);
	}
	else if (a->kind=="function"){
		TypeCheck(a->down->down->right); ///return statement
		create_header(a);
		///we have some parameters
		if(a->down->down->down != 0){
			a->tp = create_type("function",a->down->down->down->tp,a->down->down->right->tp);
			a->tp->numparams = count_params(a->down->down->down);
		}else{
			a->tp = create_type("function",0,a->down->down->right->tp);
		}
		InsertintoST(a->line,"idfunc",a->down->text,a->tp);
	}
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

  //cout<<"Starting with node \""<<a->kind<<"\" "<< a->text <<endl;
  /** START of PROGRAM */
  if (a->kind=="program") {
	///initialized scope of symbol table
    a->sc=symboltable.push();
    ///variables
    insert_vars(child(child(a,0),0));
    ///scanning for names of procedures
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
    insert_params(a->down->down->down);
    insert_vars(a->down->right->down);
    //            a->down->right->right->down
    insert_headers(child(child(a,2),0));
	//cout << "context "<< a->down->text << endl;
   // symboltable.write();

    TypeCheck(child(a,2)); ///blocks
    TypeCheck(child(a,3),"instruction");///instructions

	if(a->kind == "function"){
		///return value
		//TypeCheck(a->down->down->right);
		AST* tmp = child(a,3);
		AST* prev;
		while(tmp!=0){
			prev= tmp;
			tmp= tmp->right;
		}
		///last command of function
		TypeCheck(prev);
		if(!equivalent_types(prev->tp, a->tp->right)){
			errorincompatiblereturn(prev->line);
		}
	}

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
	else if ((a->down->tp->kind!="error" && a->down->right->tp->kind!="error") &&
	       !equivalent_types(a->down->tp,a->down->right->tp)){
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
    a->ref = 0;
  }
  else if (a->kind=="+" || (a->kind=="-" && child(a,1)!=0) || a->kind=="*"
	   || a->kind=="/") {
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));
 // cout <<a->kind<<" "<<a->line<<" "<<child(a,0)->tp->kind<<" x "<<child(a,1)->tp->kind<<endl;
   // DO NOT CHANGE THIS
	if ((child(a,0)->tp->kind!="error" &&child(a,0)->tp->kind!="int") ||
			(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="int"))
	{
      errorincompatibleoperator(a->line,a->kind);
    }
	a->tp=create_type("int",0,0);
  }
  else if (a->kind=="ref"){
      a->tp=create_type("parref",0,0);
  }else if(a->kind == "val"){
      a->tp=create_type("parval",0,0);
  }else if(a->kind == "array"){
		///number of elements in array
	    TypeCheck(child(a,1));
		//ASTPrint(a);
		//cout<<a->line<<" "<<a->tp->kind<<endl;
		a->tp=create_type("array", a->down->right->tp, 0);
		a->tp->numelemsarray=atoi(a->down->text.c_str());
  }else if(a->kind == "["){
	  TypeCheck(child(a,0));
	  TypeCheck(child(a,1));
	  a->ref=1;
	    if(child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="array"){
			errorincompatibleoperator(a->down->line,"array[]");
		}else{
			if(child(a,0)->tp->kind=="error"){
				a->tp=a->down->tp;
				a->ref=0;
			}
			else{
				a->tp=child(a,0)->tp->down;
			}
		}

		if(a->down->right->tp->kind!="error" && a->down->right->tp->kind!="int")
		{
			errorincompatibleoperator(a->line, "[]");
		}
  }else if (isbasickind(a->kind)) {
    a->tp=create_type(a->kind,0,0);
  }
  else if (a->kind=="writeln") {
    TypeCheck(child(a,0));
    if (child(a,0)->tp->kind!="error" && !isbasickind(child(a,0)->tp->kind)) {
      errorreadwriterequirebasic(a->line,a->kind);
    }
  }	else if((a->kind=="read") || (a->kind=="write"))
	{
		TypeCheck(a->down);
		if(a->down->ref==0 && !isbasickind(a->down->tp->kind))
		{
			errornonreferenceableexpression(a->line,a->kind);
			//a->tp=create_type("error",0,0);
		}
		else if(!isbasickind(a->down->tp->kind) && a->down->tp->kind!="error")
		{
			errorreadwriterequirebasic(a->line, a->kind);
		}
	}
  else if (a->kind == "("){
	/**
	* at this point we don't know if this is a procedure(function) call, or an expression
	* in case of call, we have to check if that it proprerly defined  in symboltable
	*/
	   ///this is defition of function which will be applied to this call
		if(!symboltable.find(a->down->text)){
				///Identifier is not declared
				errornondeclaredident(a->line, a->down->text);
		}else{
			ptype stdef = symboltable[a->down->text].tp;
			if (info == "instruction") {

				///we need to check if  a->down->text is a callable
				if (stdef->kind != "procedure"){
						errorisnotprocedure(a->line);
				}
				if(stdef->kind == "function" || stdef->kind == "procedure"){
					///validate params of procedure
					validate_params(a, stdef, a->line, stdef->numparams);
				}
			}else{
			/// info!="instruction"
				///we are inside expression and it MUST return a value
				if (stdef->kind != "error" && stdef->kind != "function") {
				///if (stdef->kind != "function" ){
					errorisnotfunction(a->line);
				}else if(stdef->kind=="function"){
						if(info=="instruction")
							errorisnotprocedure(a->line);
						else{
							a->tp=stdef->right;
							a->kind = "idfunc";
							a->ref=0;
						}
				}
				validate_params(a, stdef, a->line, stdef->numparams);
			}
		}
	}else if(a->kind == "not" && a->down->right==0){
		TypeCheck(a->down);
		if (child(a,0)->tp->kind != "bool" &&
				child(a,0)->tp->kind != "error") {
			errorincompatibleoperator(a->line, a->kind);
		}
		a->tp = create_type("bool",0,0);
	}else if(a->kind == "or" || a->kind == "and"){
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		if ((a->down->tp->kind!="error" && a->down->tp->kind!="bool") ||
		(a->down->right->tp->kind!="error" && a->down->right->tp->kind!="bool"))
		{
			errorincompatibleoperator(a->line, a->kind);
		}
		a->tp = create_type("bool",0,0);

	}else if(a->kind == "="){
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		///we can accept here probably bool and int
	/*	if (child(a,0)->tp->kind == "error" ||
				child(a,1)->tp->kind == "error"||
				!equivalent_types(child(a,0)->tp,child(a,1)->tp)) {
			errorincompatibleoperator(a->line, a->kind);
		}*/
		if ((a->down->tp->kind != a->down->right->tp->kind
			&& a->down->tp->kind!="error" && a->down->right->tp->kind!="error")
			|| (!isbasickind(a->down->tp->kind) || !isbasickind(a->down->right->tp->kind)))
		{
			errorincompatibleoperator(a->line,a->kind);
		}

		a->tp = create_type("bool",0,0);
	}else if(a->kind == ">" || a->kind == "<"){
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		if ((child(a,0)->tp->kind != "int" &&
					child(a,0)->tp->kind != "error") ||
				 (child(a,1)->tp->kind != "int" &&
				child(a,1)->tp->kind != "error")) {
			errorincompatibleoperator(a->line, a->kind);
		}
		a->tp = create_type("bool",0,0);
	}else if(a->kind == "while"){
		TypeCheck(child(a,0));
		if(a->down->tp->kind!="error" && a->down->tp->kind!="bool")
		{
			errorbooleanrequired(a->line,a->kind);
		}
		///list of instructions
		TypeCheck(a->down->right, info);
	}else if(a->kind== "if"){
		TypeCheck(child(a,0));
		if(a->down->tp->kind!="error" && a->down->tp->kind!="bool")
		{
			errorbooleanrequired(a->line,a->kind);
		}
		///list of instructions
		TypeCheck(child(a,1), info);
		///ELSE clause
		if(child(a,2) !=0){
			TypeCheck(child(a,2), info);
		}
	}
	else if(a->kind =="+"){
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		if ((child(a,0)->tp->kind != "int" && child(a,0)->tp->kind != "error") ||
				(child(a,1)->tp->kind != "int" && child(a,1)->tp->kind == "error")) {
			errorincompatibleoperator(a->line, a->kind);
		}
		a->tp=create_type("int",0,0);
	}else if(a->kind == "-"){
		TypeCheck(child(a,0));
		///unary minus
		if (child(a,0)->tp->kind != "int" &&
				child(a,0)->tp->kind != "error") {
			errorincompatibleoperator(a->line, a->kind);
		}
		///binary operator
		if(child(a,1) != 0){
			TypeCheck(child(a,1));
			if (child(a,1)->tp->kind != "int" &&
				child(a,1)->tp->kind != "error") {
				errorincompatibleoperator(a->line, a->kind);
			}
		}
		a->tp=create_type("int",0,0);
	}else if (a->kind==".") {
		TypeCheck(child(a,0));
		a->ref=child(a,0)->ref;
		if (child(a,0)->tp->kind!="error") {
			if (child(a,0)->tp->kind!="struct") {
				errorincompatibleoperator(a->line,"struct.");
			}
			else if (child(a,0)->tp->struct_field.find(child(a,1)->text)==
					child(a,0)->tp->struct_field.end()) {
				errornonfielddefined(a->line,child(a,1)->text);
			}else {
				a->tp=child(a,0)->tp->struct_field[child(a,1)->text];
			}
		}
		//cout << a->line <<" "<<a->tp->kind<<" "<<a->text<<"["<<child(a,1)->text<<"]"<< child(a,1)->tp->kind <<endl;
	}else {
		cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
	}
  ///symboltable.write();
 /// cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}


void validate_params(AST *a,ptype tp,int line,int numparam) {
	int param=0;
	AST *a1=a->down->right->down;
	ptype tp1=tp->down;
	int n = count_params(a->down->right->down);
	///check number of params
	if(n!=numparam)
		errornumparam(line);
	else
	{ ///check is the parameters are the same
		tp1=tp->down;
		a1=a->down->right->down; ///first parameter
		while(tp1!=0 && a1!=0)
		{
			param++;
			///check tree of first param
			TypeCheck(a1);
			///check if we can assign param a value
			if(tp1->kind=="parref" && (!a1->ref))
				errorreferenceableparam(line,param);

			///compare types to definition
			if(!equivalent_types(tp1->down,a1->tp) && a1->tp->kind!="error")
			{
				errorincompatibleparam(line,param);
				a->ref=0;
			}
			tp1=tp1->right;
			a1=a1->right;
		}
	}
}

int count_params(AST *a)
{
	int i=0;
	AST *tmp = a;
	while(tmp!=0){
		i++;
		tmp = tmp->right;
	}
	return i;
}

