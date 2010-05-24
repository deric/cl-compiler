#include <iostream>
#include <string>
#include <set>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

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

set<string> read_error_lines(string filename)
{
  ifstream ifs;

  ifs.open(filename.c_str()/*,ifstream::in*/);
  if (!ifs.is_open()) {
    cout<<"Error opening file "<<filename<<endl;
    exit(1);
  }
  string linea;
  set<string> s;
  while (getline(ifs,linea)) {
    if (linea.size()>4 and linea.substr(0,3)=="L. ") {
      s.insert(linea);
    }
  }
  ifs.close();
  return s;
}

vector<string> read_file(string filename)
{
  ifstream ifs;

  ifs.open(filename.c_str()/*,ifstream::in*/);
  if (!ifs.is_open()) {
    cout<<"Error opening file "<<filename<<endl;
    exit(1);
  }
  string linea;
  vector<string> v;
  while (getline(ifs,linea)) {
      v.push_back(linea);
  }
  ifs.close();
  return v;
}

bool same_set(set<string> s1,set<string> s2)
{
  set<string>::iterator it1=s1.begin();
  set<string>::iterator it2=s2.begin();
  for (;it1!=s1.end() and it2!=s2.end();it1++,it2++) {
    if ((*it1)!=(*it2)) return false;
  }
  return it1==s1.end() and it2==s2.end();
}

int main()
{
  ifstream ifs1;

  ifs1.open("infochecker.txt");
  if (!ifs1.is_open()) {
    cout<<"ERROR opening infochecker.txt"<<endl;
    exit(1);
  }
  
  int tipusjocproves;
  float nota=0;
  stringstream infopantalla;
  while (ifs1>>tipusjocproves) {
    string jpfile,sjpfile;
    float jpnota;
    ifs1>>jpfile>>sjpfile>>jpnota;
    if (tipusjocproves==0) {
      string command="./cl <"+jpfile+" >hola";
      system(command.c_str());
      set<string> jplines=read_error_lines("hola");
      set<string> sjplines=read_error_lines(sjpfile);
      if (same_set(jplines,sjplines)) {
	nota+=jpnota;
	infopantalla<<" si";
      } else {
	infopantalla<<" no";
      }
    } else {
      string command="./cl <"+jpfile+" execute >/dev/null";
      system(command.c_str());
      vector<string> v1=read_file("outputexecution");
      vector<string> v2=read_file(sjpfile);
      if (v1==v2) {
	nota+=jpnota;
	infopantalla<<" si";
      } else {
	infopantalla<<" no";
      }
    }
  }    
  infopantalla<<" "<<nota;
  cout<<infopantalla.str()<<endl;

  ofstream ofs;
  ofs.open("../nota");
  if (!ofs.is_open()) {
    cout<<"ERROR opening ../nota"<<endl;
    exit(1);
  }
  ofs<<nota;
  ofstream ofs2;
  ofs2.open("nota");
  if (!ofs2.is_open()) {
    cout<<"ERROR opening nota"<<endl;
    exit(1);
  }
  ofs2<<infopantalla;
}

