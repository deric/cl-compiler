/// utility functions for cl.g
#include <string>

using namespace std;

// convert an integer to string
string itostring(int x) {
  string s;
  if (x==0) return "0";

  while(x>0) {
    s=char((x%10)+'0')+s;
    x=x/10;
  }
  return s;
}

/// convert a string to an integer
int stringtoint(string s)
{
  int v=0;
  for (int i=0;i<(int)s.size();i++) {
    v=v*10+s[i]-'0';
  }
  return v;
}

/// convert a string to lowercase
string lowercase(string s)
{
  for (int i=0;i<(int)s.size();i++) {
    if (s[i]>='A' && s[i]<='Z') {
      s[i]=s[i]-'A'+'a';
    }
  }
  return s;
}


