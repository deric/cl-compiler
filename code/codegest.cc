#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <cstdlib>

using namespace std;

#include "util.hh"

#include "codegest.hh"

int securemode=0;
map<string,string>numops;

// Local functions:
bool isint(string s);
bool isvar(string s);
int checkcodopcorrect(string s);
void initnumops();

void securemodeon()
{
  securemode=1;
}

void securemodeof()
{
  securemode=0;
}

codechain::codechain()
{
  first=last=0;
}

codechain operator||(codechain c1,codechain c2)
{
  if (c1.first==0) return c2;
  if (c2.first==0) return c1;
  
  if (securemode) {
    codenode *cn=c1.first;
    while (cn!=0) {
      cn->check=0;
      cn=cn->next;
    }
    cn=c2.first;
    while (cn!=0) {
      cn->check=0;
      cn=cn->next;
    }
    cn=c1.first;
    while (cn!=0) {
      if (cn->check!=0) {
	cout<<"Extremelly fatal error: concat of overlapping chains."<<endl;
	exit(1);
      } 
      cn->check=1;
      cn=cn->next;
    }
    cn=c2.first;
    while (cn!=0) {
      if (cn->check!=0) {
	cout<<"Extremelly fatal error: concat of overlapping chains."<<endl;
	exit(1);
      } 
      cn->check=1;
      cn=cn->next;
    }
  }

  c1.last->next=c2.first;
  c1.last=c2.last;

  return c1;
}

codechain::codechain(string codop,string op1)
{
  codenode *cn=new codenode;
  cn->codop=codop;
  cn->op.push_back(op1);
  cn->next=0;
  first=last=cn;
}

codechain::codechain(string codop,string op1,string op2)
{
  codenode *cn=new codenode;
  cn->codop=codop;
  cn->op.push_back(op1);
  cn->op.push_back(op2);
  cn->next=0;
  first=last=cn;
}

codechain::codechain(string codop,string op1,string op2,string op3)
{
  codenode *cn=new codenode;
  cn->codop=codop;
  cn->op.push_back(op1);
  cn->op.push_back(op2);
  cn->op.push_back(op3);
  cn->next=0;
  first=last=cn;
}

bool isint(string s)
{
  for (int i=0;i<(int)s.size();i++) {
    if (s[i]<'0'||s[i]>'9') return false;
  }
  return true;
}

bool isextendedint(string s)
{
  if (!isint(s)) {
    return s.substr(0,7)=="offset(";
  }
  return true;
}

bool iskeyword(string s)
{
  return (s=="program" || s=="parameters" ||
	  s=="endparameters" || s=="variables" || s=="endvariables" ||
	  s=="endprogram" || s=="subroutine" || s=="endsubroutine");
}

bool isinstruction(string s)
{
  return numops.find(s)!=numops.end();
}

bool isstring(string s)
{
  if (s.size()<2) return false;
  if (s[0]!='\"' || s[(int)s.size()-1]!='\"') return false;

  for (int i=1;i<(int)s.size()-1;i++) {
    if (s[i]=='\"') return false;
  }
  return true;
}

bool istemporal(string s)
{
  if (s.size()==0 || s[0]!='t') return false;

  for (int i=1;i<(int)s.size();i++) {
    if (s[i]<'0'||s[i]>'9') return false;
  }
  return true;
}

bool isvariable(string s)
{
  if (s.size()==0) return false;
  if (iskeyword(s)) return false;
  if (!((s[0]>='a' && s[0]<='z') ||
	(s[0]>='A' && s[0]<='Z') || s[0]=='_')) return false;
  for (int i=1;i<(int)s.size();i++) {
    if (!((s[i]>='a' && s[i]<='z') ||
	  (s[i]>='A' && s[i]<='Z') ||
	  (s[i]>='0' && s[i]<='9') || s[i]=='_')) {
      return false;
    }
  }
  return true;
}

bool isaddress(string s)
{
  return istemporal(s) || isvariable(s);
}

bool isextendedintoraddress(string s)
{
  return isextendedint(s) || isaddress(s);
}

codechain::codechain(char *cp)
{
  string s=cp;
  codechain c(s);
  first=c.first;
  last=c.last;
}

void jumpwhitespaces(string &s,int &i)
{
  while (i<(int)s.size() && s[i]==' ') i++;
}

void getbasicunits(string &s,vector<string> &basicunits)
{
  int i=0;
  string word;

  jumpwhitespaces(s,i);
  while (i<(int)s.size()) {
    word="";
    if (s[i]=='\"') {
      int iini=i;
      i++;
      while (i<(int)s.size() && s[i]!='\"') {
	i++;
      }
      if (i==(int)s.size()) {
	cout<<"CODE GENERATION FATAL ERROR, non-terminated string in: "<<s<<endl;
      } else {
	basicunits.push_back(s.substr(iini,i-iini+1));
	i++;
      }
    } else {
      while (i<(int)s.size() && s[i]!=' ') {
	word=word+string(1,s[i]);
	i++;
      }
      basicunits.push_back(word);
    }
    jumpwhitespaces(s,i);
  }	 
}

codechain::codechain(string s)
{
  vector<string> basicunits;
  getbasicunits(s,basicunits);
  codechain c;
  int i=0;

  while (i<(int)basicunits.size()) {
    if (numops.find(basicunits[i])==numops.end()) {
      cout<<"CODE GENERATION FATAL ERROR, a non-operation tried as operation: "<<basicunits[i]<<endl;
    } else {
      if (numops[basicunits[i]]=="") {
	codenode *cn=new codenode;
	cn->codop=basicunits[i];
	cn->next=0;
	codechain caux;
	caux.first=caux.last=cn;
	c=c||caux;
      } else if ((int)numops[basicunits[i]].size()>(int)basicunits.size()-i-1) {
	cout<<"CODE GENERATION FATAL ERROR, not enough operators for operation: "<<basicunits[i]<<endl;
      } else {
	bool error=false;
	for (int j=0;j<(int)numops[basicunits[i]].size();j++) {
	  if (numops[basicunits[i]][j]=='s' && !isstring(basicunits[i+1+j])) {
	    cout<<"CODE GENERATION FATAL ERROR, string expected instead of: "<<basicunits[i+1+j]<<endl;
	    error=true;
	  } else if (numops[basicunits[i]][j]=='a' && !isaddress(basicunits[i+1+j])) {
	    cout<<"CODE GENERATION FATAL ERROR, temporal or variable expected instead of: "<<basicunits[i+1+j]<<endl;
	    error=true;
	  } else if (numops[basicunits[i]][j]=='c' && !isextendedint(basicunits[i+1+j])) {
	    cout<<"CODE GENERATION FATAL ERROR, integer constant expected instead of: "<<basicunits[i+1+j]<<endl;
	    error=true;
	  } else if (numops[basicunits[i]][j]=='i' && !isextendedintoraddress(basicunits[i+1+j])) {
	    cout<<"CODE GENERATION FATAL ERROR, value expected instead of: "<<basicunits[i+1+j]<<endl;
	    error=true;
	  } else if (numops[basicunits[i]][j]=='t' && !istemporal(basicunits[i+1+j])) {
	    cout<<"CODE GENERATION FATAL ERROR, temporal expected instead of: "<<basicunits[i+1+j]<<endl;
	    error=true;
	  } else if (numops[basicunits[i]][j]=='v' && !isvariable(basicunits[i+1+j])) {
	    cout<<"CODE GENERATION FATAL ERROR, variable expected instead of: "<<basicunits[i+1+j]<<endl;
	    error=true;
	  }
	}
	if (!error) {
	  switch ((int)numops[basicunits[i]].size()) {
	  case 1: c=c||codechain(basicunits[i],basicunits[i+1]);break;
	  case 2: c=c||codechain(basicunits[i],basicunits[i+1],basicunits[i+2]);break;
	  case 3: c=c||codechain(basicunits[i],basicunits[i+1],basicunits[i+2],basicunits[i+3]);break;
	  }
	}
	i+=(int)numops[basicunits[i]].size();
      }
    }
    i++;
  }

  first=c.first;
  last=c.last;
}

void writecodechain(codechain &c,ostream &os)
{
  codenode *cn=c.first;

  while (cn!=0) {
    if (cn->codop!="etiq" && cn->codop!="ETIQ") {
      os<<"    "<<cn->codop;      
    } else {
      os<<"  "<<cn->codop;
    }
    for (int i=0;i<(int)cn->op.size();i++) {
      os<<" "<<cn->op[i];
    }
    os<<endl;
    cn=cn->next;
  }
}

void writesubroutinecontents(codesubroutine &cs,ostream &os)
{
  os<<"  parameters"<<endl;
  for (list<string>::iterator it=cs.parameters.begin();it!=cs.parameters.end();it++) {
    os<<"    "<<*it<<endl;
  }
  os<<"  endparameters"<<endl<<endl;
    
  os<<"  variables"<<endl;
  for (list<variable_data>::iterator it=cs.localvariables.begin();it!=cs.localvariables.end();it++) {
    os<<"    "<<(*it).name<<" "<<(*it).size<<endl;
  }
  os<<"  endvariables"<<endl<<endl;
  writecodechain(cs.c,os);
}

void writecodeglobal(codeglobal &cg,ostream &os)
{
  os<<"program"<<endl;
  writesubroutinecontents(cg.mainsub,os);
  os<<"endprogram"<<endl;
  for (list<codesubroutine>::iterator it=cg.l.begin();it!=cg.l.end();it++) {
    os<<endl<<"subroutine "<<(*it).name<<endl;
    writesubroutinecontents(*it,os);
    os<<"endsubroutine"<<endl;
  }
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// An interpreter of T-code, for direct execution:
//////////////////////////////////////////////////
//////////////////////////////////////////////////

#define MAXSTACK 1000000
class memorygest {
public:
  map<int,int> m;
  int stacksize;
  map<int,int> allocatedsize;
  int freeallocatedsize;
  memorygest() {
    stacksize=0;
    freeallocatedsize=MAXSTACK+1;
  }
  int &operator[](int i) {
    if (m.find(i)==m.end()) {
      cout<<"Execution error: out of memory access."<<endl;
      exit(1);
    }
    return m[i];
  }
  void executepush(int value) {
    m[stacksize]=value;
    stacksize++;
    if (stacksize>=MAXSTACK) {
      cout<<"Execution error: stack size used is bigger than the allowed size."<<endl;
      exit(1);
    }
  }
  void executepushsize(int size) {
    for (int i=0;i<size/4;i++) {
      executepush(0);
    }
  }
  int executepop() {
    int v=m[stacksize-1];
    m.erase(stacksize-1);
    stacksize--;
    return v;
  }
  void executepopsize(int size) {
    for (int i=0;i<size/4;i++) {
      executepop();
    }
  }
  void write() {
    int first=true;
    for (map<int,int>::iterator it=m.begin();it!=m.end();it++) {
      if (!first) cout<<",";
      first=false;
      cout<<"M["<<4*it->first<<"]="<<it->second;
    }
    cout<<endl;
  }
  int malloc(int size) {
    for (int i=0;i<size/4;i++) {
      m[freeallocatedsize+i]=0;
    }
    allocatedsize[freeallocatedsize]=size;
    int aux=freeallocatedsize;
    freeallocatedsize+=size/4;
    return 4*aux;
  }
  void free(int memposition) {
    memposition=memposition/4;
    if (allocatedsize.find(memposition)==allocatedsize.end()) {
      cout<<"Execution error: free of a non-allocated memory position."<<endl;
      exit(1);
    }
    for (int i=0;i<allocatedsize[memposition]/4;i++) {
      m.erase(memposition+i);
    }
    allocatedsize.erase(memposition);
  }
};

memorygest memory;

//vector<int> memory;
map<string,map<string,int> > relativeoffset;
map<string,int> relativeoffsetextern;
map<string,codesubroutine*> code;
map<string,int> variablesize;
map<string,map<string,codenode *> > label;
map<int,string> inttosubroutinename;
map<int,string> inttolabelname;
map<string,int> subroutinetoint;
map<string,map<string,int> > labeltoint;
int timelimit;
int stepbystep;

// In case of a temporal as variablename it is assumed to contain a memory address.
int memoryaddress(int scopebase,string subroutinename,string variablename,
		 map<string,int> &temporal)
{
  if (istemporal(variablename)) {
    return temporal[variablename];
  } else {
    return scopebase+relativeoffset[subroutinename][variablename];
  }
}

// In case of a temporal as variablename it is assumed to contain a memory address.
int &memoryvalue(int scopebase,string subroutinename,string variablename,
		 map<string,int> &temporal)
{
  if (istemporal(variablename)) {
    return memory[temporal[variablename]/4];
  } else {
    return memory[(scopebase+relativeoffset[subroutinename][variablename])/4];
  }
}

int &valuereferenceable(int scopebase,string subroutinename,string variablename,
	   map<string,int> &temporal)
{
  if (istemporal(variablename)) {
    return temporal[variablename];
  } else {
    return memory[(scopebase+relativeoffset[subroutinename][variablename])/4];
  }
}

int value(int scopebase,string subroutinename,string variablename,
	   map<string,int> &temporal)
{
  if (isint(variablename)) {
    return stringtoint(variablename);
  } else if (istemporal(variablename)) {
    return temporal[variablename];
  } else if (relativeoffsetextern.find(variablename)!=
	     relativeoffsetextern.end()) {
    return relativeoffsetextern[variablename];
  } else if (relativeoffset[subroutinename].find(variablename)!=
	     relativeoffset[subroutinename].end()) {
    return memory[(scopebase+relativeoffset[subroutinename][variablename])/4];
  } else if (labeltoint[subroutinename].find(variablename)!=
	     labeltoint[subroutinename].end()) {
    return labeltoint[subroutinename][variablename];
  } else if (subroutinetoint.find(variablename)!=
	     subroutinetoint.end()) {
    return subroutinetoint[variablename];
  } else {
    cout<<"EXECUTION ERROR: expression "<<variablename<<" has not a value."<<endl;
    exit(1);
  }
}

string obtainlabelname(int scopebase,string subroutinename,string variablename,
	   map<string,int> &temporal)
{
  if (label[subroutinename].find(variablename)!=label[subroutinename].end()) {
    return variablename;
  } else {
    return inttolabelname[value(scopebase,subroutinename,variablename,temporal)];
  }
}

string obtainsubroutinename(int scopebase,string subroutinename,string variablename,
	   map<string,int> &temporal)
{
  if (code.find(variablename)!=code.end()) {
    return variablename;
  } else {
    return inttosubroutinename[value(scopebase,subroutinename,variablename,temporal)];
  }
}


void writememoryandtemporalcontents(map<string,int> &temporal)
{
  cout<<"Memory:";
  memory.write();
  cout<<"temporal:";
  for (map<string,int>::iterator it=temporal.begin();it!=temporal.end();it++) {
    cout<<","<<it->first<<"="<<it->second;
  }
  cout<<endl;
}


int executesubroutine(codesubroutine *cs)
{
  int scopebase=4*(memory.stacksize-1);
  memory.executepushsize(variablesize[cs->name]);
  map<string,int> temporal;
  for (codenode *instruction=cs->c.first;
       instruction!=NULL;instruction=instruction->next) {
    if (stepbystep) {
      cout<<endl;
      writememoryandtemporalcontents(temporal);
      cout<<"executing: "<<instruction->codop;
      for (int i=0;i<(int)instruction->op.size();i++) {
	cout<<" "<<instruction->op[i];
      }
      cout<<endl;
    }

    if (--timelimit==0) {
      cout<<endl<<"Time limit exceeded: the execution is aborted"<<endl;
      return 1;
    }    
    if (instruction->codop=="addi") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)+
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="aload") {
      valuereferenceable(scopebase,cs->name,instruction->op[1],temporal)=
	memoryaddress(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="call") {
      if (executesubroutine(code[obtainsubroutinename(scopebase,
						      cs->name,
						      instruction->op[0],
						      temporal)])) {
	memory.executepopsize(variablesize[cs->name]);
	return 1;
      }
    } else if (instruction->codop=="copy") {
      for (int i=0;i<value(scopebase,cs->name,instruction->op[2],temporal)/4;i++) {
	memory[memoryaddress(scopebase,cs->name,instruction->op[1],temporal)/4+i]=
	  memory[memoryaddress(scopebase,cs->name,instruction->op[0],temporal)/4+i];
      }
    } else if (instruction->codop=="divi") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)/
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="equi") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)==
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="etiq") {
    } else if (instruction->codop=="fjmp") {
      if (!value(scopebase,cs->name,instruction->op[0],temporal))
	instruction=label[cs->name][obtainlabelname(scopebase,
						    cs->name,
						    instruction->op[1],
						    temporal)];
    } else if (instruction->codop=="free") {
      memory.free(value(scopebase,cs->name,instruction->op[0],temporal));
    } else if (instruction->codop=="grti") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)>
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="iload") {
      valuereferenceable(scopebase,cs->name,instruction->op[1],temporal)=
        value(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="killparam") {
      memory.executepop();
    } else if (instruction->codop=="land") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)&&
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="lesi") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)<
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="lnot") {
      valuereferenceable(scopebase,cs->name,instruction->op[1],temporal)=
	!value(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="load") {
      valuereferenceable(scopebase,cs->name,instruction->op[1],temporal)=
	memoryvalue(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="loor") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)||
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="mini") {
      valuereferenceable(scopebase,cs->name,instruction->op[1],temporal)=
	-value(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="move") {
      valuereferenceable(scopebase,cs->name,instruction->op[1],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="muli") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)*
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="new") {
      valuereferenceable(scopebase,cs->name,instruction->op[1],temporal)=
	memory.malloc(value(scopebase,cs->name,instruction->op[0],temporal));
    } else if (instruction->codop=="popparam") {
      valuereferenceable(scopebase,cs->name,instruction->op[0],temporal)=memory.executepop();
    } else if (instruction->codop=="pushparam") {
      memory.executepush(value(scopebase,cs->name,instruction->op[0],temporal));
    } else if (instruction->codop=="reai") {
      cin>>valuereferenceable(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="retu") {
      memory.executepopsize(variablesize[cs->name]);
      return 0;
    } else if (instruction->codop=="stop") {
      memory.executepopsize(variablesize[cs->name]);
      return 1;
    } else if (instruction->codop=="stor") {
      memoryvalue(scopebase,cs->name,instruction->op[1],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="subi") {
      valuereferenceable(scopebase,cs->name,instruction->op[2],temporal)=
	value(scopebase,cs->name,instruction->op[0],temporal)-
	value(scopebase,cs->name,instruction->op[1],temporal);
    } else if (instruction->codop=="ujmp") {
      instruction=label[cs->name][obtainlabelname(scopebase,
						  cs->name,
						  instruction->op[0],
						  temporal)];
    } else if (instruction->codop=="wrii") {
      cout<<value(scopebase,cs->name,instruction->op[0],temporal);
    } else if (instruction->codop=="wris") {
      cout<<instruction->op[0].substr(1,int(instruction->op[0].size())-2);
    } else if (instruction->codop=="wrln") {
      cout<<endl;
    } else {
      cout<<endl<<"EXECUTION ERROR: non existing instruction "<<instruction->codop<<endl;
      return 1;
    }
  }
  cout<<endl<<"Error, list of instructions does not end with stop or retu"<<endl;
  return 1;
}

void initializeexecution(codeglobal &cg)
{
  memory=memorygest();//initializes the memory gestor.
  // gives the offset for every pair subroutine name-variable name.
  relativeoffset= map<string,map<string,int> > ();
  // The same, but the pair is combined in one string.
  relativeoffsetextern= map<string,int> ();
  // Gives the codesubroutine information for every subroutine name.
  code=map<string,codesubroutine*>();
  // Gives the required size of local variables for every subroutine name.
  variablesize=map<string,int> ();
  // Gives the codenode address for every pair subroutine name-label name.
  label=map<string,map<string,codenode *> > ();
  // Gives the subroutine name associated to a subroutine integer identifier.
  inttosubroutinename=map<int,string>();
  // Gives the label name associated to a label integer identifier.
  inttolabelname=map<int,string>();
  // The converse of the two previous mappings.
  subroutinetoint=map<string,int>();
  labeltoint=map<string,map<string,int> >();

  int indexsubroutinename=0;
  int indexlabelname=0;

  int offsetparameter=4-4*int(cg.mainsub.parameters.size());
  for (list<string>::iterator it=cg.mainsub.parameters.begin();
       it!=cg.mainsub.parameters.end();it++) {
    relativeoffset["program"][*it]=offsetparameter;
    relativeoffsetextern["offset(program:"+*it+")"]=offsetparameter;
    offsetparameter+=4;
  }
  int offsetvariables=4;
  for (list<variable_data>::iterator it=cg.mainsub.localvariables.begin();
       it!=cg.mainsub.localvariables.end();it++) {
    relativeoffset["program"][(*it).name]=offsetvariables;
    relativeoffsetextern["offset(program:"+(*it).name+")"]=offsetvariables;
    offsetvariables+=(*it).size;
  }
  code["program"]=&(cg.mainsub);
  variablesize["program"]=offsetvariables-4;
  for (codenode *c=cg.mainsub.c.first;c!=NULL;c=c->next) {
    if (c->codop=="etiq") {
      label["program"][c->op[0]]=c;
      labeltoint["program"][c->op[0]]=indexlabelname;
      inttolabelname[indexlabelname++]=c->op[0];
    }
  }
  subroutinetoint["program"]=indexsubroutinename;
  inttosubroutinename[indexsubroutinename++]="program";

  for (list<codesubroutine>::iterator it2=cg.l.begin();
       it2!=cg.l.end();it2++) {
    offsetparameter=4-4*int((*it2).parameters.size());
    for (list<string>::iterator it=(*it2).parameters.begin();
	 it!=(*it2).parameters.end();it++) {
      relativeoffset[(*it2).name][*it]=offsetparameter;
      relativeoffsetextern["offset("+(*it2).name+":"+*it+")"]=offsetparameter;
      offsetparameter+=4;
    }
    offsetvariables=4;
    for (list<variable_data>::iterator it=(*it2).localvariables.begin();
	 it!=(*it2).localvariables.end();it++) {
      relativeoffset[(*it2).name][(*it).name]=offsetvariables;
      relativeoffsetextern["offset("+(*it2).name+":"+(*it).name+")"]=offsetvariables;
      offsetvariables+=(*it).size;
    }
    code[(*it2).name]=&(*it2);
    variablesize[(*it2).name]=offsetvariables-4;
    for (codenode *c=(*it2).c.first;c!=NULL;c=c->next) {
      if (c->codop=="etiq") {
	label[(*it2).name][c->op[0]]=c;
	labeltoint[(*it2).name][c->op[0]]=indexlabelname;
	inttolabelname[indexlabelname++]=c->op[0];
      }
    }
    subroutinetoint[(*it2).name]=indexsubroutinename;
    inttosubroutinename[indexsubroutinename++]=(*it2).name;
  }
  if (stepbystep) {
    cout<<"Computed information:"<<endl;
    for (map<string,int>::iterator it=relativeoffsetextern.begin();
	 it!=relativeoffsetextern.end();it++) {
      cout<<it->first<<"="<<it->second<<endl;
    }
  }
}

int executecodeglobal(codeglobal &cg,int executiontime,int doitstepbystep)
{
  timelimit=executiontime;
  stepbystep=doitstepbystep;
  initializeexecution(cg);

  return executesubroutine(&cg.mainsub);
}

// a=address (temporal or variable), s=string, i=integer value (address or int)
// t=temporal, l=label, v=variable, c=intconst.
void initnumops()
{
  numops["addi"]="iia";
  numops["aload"]="aa";
  numops["call"]="a";
  numops["copy"]="aai";
  numops["divi"]="iia";
  numops["equi"]="iia";
  numops["etiq"]="v";
  numops["fjmp"]="ia";
  numops["free"]="a";
  numops["grti"]="iia";
  numops["iload"]="ca";
  numops["killparam"]="";
  numops["land"]="iia";
  numops["lesi"]="iia";
  numops["lnot"]="ia";
  numops["load"]="at";
  numops["loor"]="iia";
  numops["mini"]="ia";
  numops["move"]="vt";
  numops["muli"]="iia";
  numops["new"]="ia";
  numops["popparam"]="a";
  numops["pushparam"]="i";
  numops["reai"]="a";
  numops["retu"]="";
  numops["stop"]="";
  numops["stor"]="vv";
  numops["subi"]="iia";
  numops["ujmp"]="a";
  numops["wrii"]="i";
  numops["wris"]="s";
  numops["wrln"]="";
}
