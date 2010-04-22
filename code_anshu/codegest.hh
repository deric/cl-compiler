#ifndef CODEGEST_H
#define CODEGEST_H

// Data structures and functions to manage t-code.

typedef struct auxcodenode {
  string codop;
  vector<string> op;
  auxcodenode *next;
  // Used when securemode is true, and then concatenation needs linear time.
  int check;
} codenode;

class codechain {

public:
  codenode *first;
  codenode *last;
  codechain();
  codechain(string s);
  codechain(char *cp);
  codechain(string codop,string op1);
  codechain(string codop,string op1,string op2);
  codechain(string codop,string op1,string op2,string op3);
};

typedef struct {
  string name;
  int size;
} variable_data;

typedef struct {
  string name;
  list<string> parameters;
  list<variable_data> localvariables;
  codechain c;
} codesubroutine;

typedef struct {
  codesubroutine mainsub;
  list<codesubroutine> l;
} codeglobal;

// The code representation tries to concatenate code chains optimally,
// but then it does not check for mistakes of the compiler like
// node-overlapping of the chains that are being concatenated.
// With securemodeon this errors are checked, but then we have linear time.
void securemodeon();
void securemodeof();

codechain operator||(codechain c1,codechain c2);
void writecodechain(codechain &c,ostream &os=cout);
void writesubroutinecontents(codesubroutine &cs,ostream &os=cout);
void writecodeglobal(codeglobal &cg,ostream &os=cout);

// Necessary to be called before any use of previous data structures
// and functions.
void initnumops();

int executecodeglobal(codeglobal &cg,int executiontime,int doitstepbystep);

#endif
