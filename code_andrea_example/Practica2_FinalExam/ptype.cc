#include <iostream>
#include <string>
#include <map>
#include <list>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include "ptype.hh"

ptype create_type(string kind,ptype down,ptype right)
{
  ptype t=new(stype);

  t->kind=kind;
  t->down=down;
  t->right=right;
  t->numelemsarray=0;
  // By default we assume a basic kind.
  t->size=4;
  return t;
}

void write_type(ptype t)
{
  if (t==0) return;

  cout<<t->kind;
  if (t->numelemsarray!=0) {
    cout<<"(numelemsarray:"<<t->numelemsarray<<")";
  }
  if ((int)(t->ids.size())!=0) {
    cout<<"(";
    list<string>::iterator it=t->ids.begin();
    for (;it!=t->ids.end();) {
      cout<<"<"<<*it<<",";
      write_type(t->struct_field[*it]);
      cout<<">";
      it++;
      if (it!=t->ids.end()) cout<<",";
    }
    cout<<")";
  }
  if (t->down!=0) {
    cout<<"(";
    write_type(t->down);
    cout<<")";
  }
  if (t->right!=0) {
    cout<<",";
    write_type(t->right);
  }
}

ptype typecopy(ptype t)
{
  if (t==0) return 0;

  // Instead of the following line
  // return type_create(t->kind,typecopy(t->down),typecopy(t->right),t->passmode);
  // we do a more generic implementation able to additions of fields in ptype.
  ptype newt=new stype;
  *newt=*t;
  newt->down=typecopy(t->down);
  newt->right=typecopy(t->right);
  return newt;
}

bool equivalent_types(ptype t1,ptype t2)
{
  if (t1==0 && t2==0) return true;
  if (t1==0) return false;
  if (t2==0) return false;

  bool equiv=t1->kind==t2->kind && 
    equivalent_types(t1->down,t2->down) &&
    equivalent_types(t1->right,t2->right) &&
    t1->numelemsarray==t2->numelemsarray;
  if (equiv) {
    list<string>::iterator it1=t1->ids.begin();
    list<string>::iterator it2=t2->ids.begin();
    for (;it1!=t1->ids.end() && it2!=t2->ids.end();it1++,it2++) {
      if ((*it1)!=(*it2) || !equivalent_types(t1->struct_field[*it1],t2->struct_field[*it2])) {
	return false;
      }
    }
    if (it1!=t1->ids.end() || it2!=t2->ids.end()) return false;
  }
  return equiv;
}
