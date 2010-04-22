#include <iostream>
#include <string>
#include <map>
#include <list>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include "ptype.hh"
#include "symtab.hh"

symtab::symtab()
{
  current=0;
}

void symtab::createsymbol(string id)
{
  infosym inf;
  current->m[id];
  current->ids.push_back(id);
}

infosym &symtab::operator[](string id)
{
  scope *sc=current;
  
  while (sc!=0) {
    if (sc->m.find(id)!=sc->m.end()) {
			return sc->m[id];
    }
    sc=sc->previous;
  }

  current->ids.push_back(id);
  return current->m[id];
}

bool symtab::find(string id)
{
  scope *sc=current;
  
  while (sc!=0) {
    if (sc->m.find(id)!=sc->m.end()) {
      return true;
    }
    sc=sc->previous;
  }

  return false;
}


scope *symtab::push()
{
  scope *sc=new scope;
  
  sc->previous=current;
  current=sc;
	
  return current;
}

void symtab::push(scope *sc)
{
  current=sc;
}

void symtab::pop()
{
  current=current->previous;
}

scope *symtab::top()
{
  return current;
}

int symtab::jumped_scopes(string id)
{
  scope *sc=current;
  int d=0;
  
  while (sc!=0) {
    if (sc->m.find(id)!=sc->m.end()) {
      return d;
    }
    sc=sc->previous;
    d++;
  }
  
  return -1;
}


void symtab::write()
{
  scope *sc=current;

  cout<<"Contents of symbol table:"<<endl;
  while (sc!=0) {
    cout<<"----------------"<<endl;
    for (list<string>::iterator it=sc->ids.begin();
	 it!=sc->ids.end();it++) {
      cout<<*it<<":"<<sc->m[*it].kind<<",";
      write_type(sc->m[*it].tp);
      cout<<endl;
    }
    sc=sc->previous;
  }
  cout<<"----------------"<<endl;
}


string symtab::idtable(string id)
{
  scope *sc=current;
  
  while (sc!=0) {
    if (sc->m.find(id)!=sc->m.end()) {
      return sc->idtable;
    }
    sc=sc->previous;
  }
  
  return "";
}

void symtab::setidtable(string id)
{
  current->idtable=id;
}

