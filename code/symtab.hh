#ifndef SYMTAB_H
#define SYMTAB_H

// Information associated to an identifier
typedef struct {
  string kind;
  ptype tp;
} infosym;


typedef struct sscope {
  // The information associated to each identifier declared in this scope.
  map<string,infosym> m;
  // For remember the order in which the ids where introduced.
  list<string> ids;
  // The next scope in the stack, nearer to the bottom.
  struct sscope *previous;
  // For the name of the complete path of the subroutine.
  string idtable;
} scope;

// The symbol table implements a stack of scopes. "current" is the top scope.
class symtab {
  scope *current;

public:
  
  symtab();
  
  // Creates a new symbol in the current scope.
  void createsymbol(string id);
  // Returns the infosym related to id, that is looked for
  // along the scopes of the symbol table. If
  // it does not exists, then creates it in the
  // last scope.
  infosym &operator[](string id);
  
  // Returns true if the id occurs in the symbol table,
  // and false otherwise.
  bool find(string id);
  
  // Push a new scope under the last one, and returns it.
  scope *push();
  
  // Push the received scope as the current scope.
  void push(scope *sc);

  // Set as current scope the previous one.
  void pop();

  // Returns the current scope.
  scope *top();
  
  // Returns the number of jumped scopes until finding the
  // declaration of id, and -1 if it is not found.
  int jumped_scopes(string id);

  // Writes the contents of the symbol table on the standard output.
  void write();

  // Returns the idtable of the scope containing id.
  string idtable(string id);

  // Sets the idtable of the current scope to id.
  void setidtable(string id);
};

#endif

