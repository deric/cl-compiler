void errornotanintconst(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator [:] requires an intconst in its "<<s<<" index."<<endl;
}

void erroroutrange(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator [:] has its "<<s<<" bound out of range."<<endl;
}

void errorinvalidrange(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator [:] with an invalid range."<<endl;
}
