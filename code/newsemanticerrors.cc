void errornumberitemsunpack(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": The number of items in the unpacking assignment do not match."<<endl;
}

void errornonreferenceableleftunpack(int l, int n) {
  TypeError = 1;
  cout<<"L. "<<l<<": Left expression of item " << n << " in unpacking assignment is not referenceable."<<endl;
}

void errorincompatibleassignmentunpack(int l, int n) {
  TypeError = 1;
  cout<<"L. "<<l<<": Unpacking assignment with incompatible types for item " << n << "." << endl;
}

void errorunpackrequiresstruct(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Unpacking assignment requires a struct." << endl;
}
