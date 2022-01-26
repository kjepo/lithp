// lithp.C, kjepo 4/25/89.

#include <iostream>
using namespace std;
 
// class hierarchy
class Env;	                // Environment : Id -> Sexp
class Sexp;
class   Atom;
class     Id;
class     Num;
class   Pair;
class   Procedure;              // Procedure objects (a.k.a. closures)

// these symbols are predefined and have special meaning
class Id *NIL;                  // NIL plays the role of false
class Id *TRUE;        class Id *IF_SYM;     class Id *EQ_SYM;
class Id *LET_SYM;     class Id *ADD_SYM;    class Id *SUB_SYM;
class Id *MUL_SYM;     class Id *DIV_SYM;    class Id *DEF_SYM; 
class Id *CAR_SYM;     class Id *CDR_SYM;    class Id *CONS_SYM;
class Id *ATOM_SYM;    class Id *QUOTE_SYM;  class Id *LETREC_SYM;
class Id *LAMBDA_SYM;

inline void error(string s) { cout << s << "\n"; exit(1); }

struct env_pair {               // represents an environment binding
  Atom *id;
  Sexp *val;
};

class Env {
  env_pair *vec;
  int max;
  int free;
 public:
  Env(int);			// constructor: create objects
  Env(Env&);			// constructor: copy in initialization
  void operator=(Env&);	        // assignment: cleanup and copy
  ~Env() { delete vec; }	// destructor: cleanup
  Sexp *lookup(Atom *);	        // e->lookup(id) is Sexp
  void bind(Atom *, Sexp *);	// bind id to an Sexp in env
  void print();		        // dump the environment
};

class Sexp {
 public:
  virtual void print(ostream &s, int dotted =0) {}
  virtual Id *eq(Sexp *)       { return NIL; }
  virtual Id *atom()           { return NIL; }
  virtual Sexp *car()          { return (Sexp *) NIL; }
  virtual Sexp *cdr()          { return (Sexp *) NIL; }
  virtual Sexp *eval(Env& env) { error("bug"); env.print(); return 0; }
  virtual Num *add(Sexp *)     { error("+: non-numeric arg"); return 0; }
  virtual Num *sub(Sexp *)     { error("-: non-numeric arg"); return 0; }
  virtual Num *mul(Sexp *)     { error("*: non-numeric arg"); return 0; }
  virtual Num *div(Sexp *)     { error("/: non-numeric arg"); return 0; }
  virtual void recbind(Id *)   { }  // for Procedure objects 
};

class Atom : public Sexp {
 public:
  Id *atom()                   { return TRUE; }
};

class Num : public Atom {
  int num;
 public:
  Num(int n)                   { num = n; }
  void print(ostream &s, int)  { s << num; }
  Id *eq(Sexp *x)              { return ((Num *)x)->num == num ? TRUE : NIL; }
  Sexp *eval(Env&)             { return this; }
  Num *add(Sexp *x)            { return new Num(num + ((Num *)x)->num); }
  Num *sub(Sexp *x)            { return new Num(num - ((Num *)x)->num); }
  Num *mul(Sexp *x)            { return new Num(num * ((Num *)x)->num); }
  Num *div(Sexp *x)            { return new Num(num / ((Num *)x)->num); }
};

class Id : public Atom {
  string ident;
 public:
 Id(const string &id) : ident(id) { }
  void print(ostream &s, int)  { s << ident; }
  Id *eq(Sexp *x)              { return ((Id *)x)->ident.compare(ident) ? NIL : TRUE; }    
  Sexp *eval(Env& env)         { return env.lookup(this); }
};

class Pair : public Sexp {
  Sexp *first;
  Sexp *rest;
 public:
  Pair(Sexp *x, Sexp *y)       { first = x; rest = y; }
  void print(ostream &s, int);
  Sexp *car()                  { return first; }
  Sexp *cdr()                  { return rest; }
  Sexp *eval(Env&);
};

void Pair::print(ostream &s, int dotted=0) {
  if (!dotted) s << "(";  // rule 1, (x .(y z)) == (x y z)
  first->print(s);
  if (rest != NIL) {      // rule 2, (x . NIL) == (x)
    cout << " ";
    if (rest->atom() == TRUE)
      s << " . ";
    rest->print(s, 1);
  }
  if (!dotted) s << ")";  // rule 1 again
}       

class Procedure : public Sexp {
  Env *env;
  Sexp *sexp;
 public:
  Procedure(Env& e, Sexp *s) { env = new Env(e); sexp = s; }
  Sexp *code() { return sexp; }
  Env& environment() { return *env; }
  void recbind(Id *name) { env->bind(name, this); } // enable recursion
  void print(ostream &s, int) { s << "*-procedure-*"; }
};

// syntactic sugar (causes cancer of the semicolon --A.J.Perlis)
Id   *EQ(Sexp *x, Sexp *y)     { return x->eq(y); }
Id   *ATOM(Sexp *x)            { return x->atom(); }
Sexp *CAR(Sexp *x)             { return x->car(); }
Sexp *CDR(Sexp *x)             { return x->cdr(); }
Sexp *CONS(Sexp *x, Sexp *y)   { return new Pair(x, y); }
Sexp *CADR(Sexp *x)            { return CAR(CDR(x)); }
Sexp *CADDR(Sexp *x)	       { return CAR(CDR(CDR(x))); }
Sexp *EVAL(Sexp *x, Env& env)  { return x->eval(env); }
Num  *ADD(Sexp *x, Sexp *y)    { return x->add(y); }
Num  *SUB(Sexp *x, Sexp *y)    { return x->sub(y); }
Num  *MUL(Sexp *x, Sexp *y)    { return x->mul(y); }
Num  *DIV(Sexp *x, Sexp *y)    { return x->div(y); }

ostream &operator<<(ostream &s, Sexp *x) { x->print(s); return s; }

Sexp *apply(Procedure *proc, Sexp *arg) {
  Sexp *form = proc->code();			// (lambda (formal) body)
  Sexp *formal = CAR(CAR(CDR(form)));		//            ^
  Sexp *body = CAR(CDR(CDR(form)));		//                   ^
  Env nenv = proc->environment();             // evaluate in this env
  nenv.bind((Id *) formal, arg);		// extend env with formal=arg
  return EVAL(body, nenv);
}

Sexp *Pair::eval(Env& env) {  		        // metacircular evaluator
  if (EQ(first, IF_SYM) == TRUE) {    	        // (IF pred true false)
    return EVAL(CAR(rest), env) == NIL 
      ? EVAL(CADDR(rest), env)	                // false arm taken
      : EVAL(CADR(rest), env);	                // true arm taken
  } else if (EQ(first, ADD_SYM) == TRUE)    	// (ADD x y)
    return ADD(EVAL(CAR(rest), env), EVAL(CADR(rest), env));
  else if (EQ(first, SUB_SYM) == TRUE)    	// (SUB x y)
    return SUB(EVAL(CAR(rest), env), EVAL(CADR(rest), env));
  else if (EQ(first, MUL_SYM) == TRUE)    	// (MUL x y)
    return MUL(EVAL(CAR(rest), env), EVAL(CADR(rest), env));
  else if (EQ(first, DIV_SYM) == TRUE)    	// (DIV x y)
    return DIV(EVAL(CAR(rest), env), EVAL(CADR(rest), env));
  else if (EQ(first, QUOTE_SYM) == TRUE)	// (QUOTE x)
    return CAR(rest);
  else if (EQ(first, CONS_SYM) == TRUE)         // (CONS x y)
    return CONS(EVAL(CAR(rest), env), EVAL(CADR(rest), env));
  else if (EQ(first, CAR_SYM) == TRUE)          // (CAR x)
    return CAR(EVAL(CAR(rest), env));
  else if (EQ(first, CDR_SYM) == TRUE)    	// (CDR x)
    return CDR(EVAL(CAR(rest), env));
  else if (EQ(first, ATOM_SYM) == TRUE)    	// (ATOM x)
    return (Sexp *) ATOM(EVAL(CAR(rest), env));
  else if (EQ(first, EQ_SYM) == TRUE)    	// (EQ x y)
    return (Sexp *) EQ(EVAL(CAR(rest), env), EVAL(CADR(rest), env));
  else if (EQ(first, LAMBDA_SYM) == TRUE)       // (LAMBDA x)  
    return new Procedure(env, this);
  else if (EQ(first, DEF_SYM) == TRUE) {	// (DEFINE x y)
    Sexp *rhs = EVAL(CADR(rest), env);
    env.bind((Id *) CAR(rest), rhs);	        // notice Env& env
    return rhs;
  } else if (EQ(first, LETREC_SYM) == TRUE) {   // (LETREC (id e1) e2)
    Sexp *id = CAR(CAR(rest));
    Sexp *e1 = EVAL(CAR(CDR(CAR(rest))), env);
    Sexp *e2 = CAR(CDR(rest));
    e1->recbind((Id *) id);			// if e1 is proc object it...
    Env nenv = env;				// ...will bind id to itself
    nenv.bind((Id *) id, e1);  		        // id visible in let-body
    return EVAL(e2, nenv);
  } else if (EQ(first, LET_SYM) == TRUE) {      // (LET (id e1) e2)
    Sexp *id = CAR(CAR(rest));
    Sexp *e1 = EVAL(CAR(CDR(CAR(rest))), env);
    Sexp *e2 = CAR(CDR(rest));
    Env nenv = env;
    nenv.bind((Id *) id, e1);
    return EVAL(e2, nenv);
  } else    					//  default is application
    return apply((Procedure *) EVAL(first, env), EVAL(CAR(rest), env));
}

// scanner and parser for LISP-style input
enum toktype { END, ID, NUM, LPAR = '(', RPAR = ')', DOT = '.' };
toktype token;  // current token
char id[80];    // string value when token == ID
int nval;       // numeric value when token == NUM

toktype scan() {
  char ch;
  do {                     // skip whitespace
    if (!cin.get(ch))
      return token = END;
  } while (isspace(ch));

  switch (ch) {
  case LPAR:
    return token = LPAR;
  case RPAR:
    return token = RPAR;
  case DOT:
    return token = DOT;
  case '0': case '1': case '2': case '3': case '4': 
  case '5': case '6': case '7': case '8': case '9':
    cin.putback(ch);
    cin >> nval;
    return token = NUM;
  default:
    if (isalpha(ch)) {
      char *p = id;
      *p++ = ch;
      while (cin.get(ch) && isalnum(ch))
	*p++ = ch;
      cin.putback(ch);
      *p = 0;
      return token = ID;
    } else
      error("lexical error.");
    break;
  }
  return token = END;
}

void expect(toktype tok, const string &msg) {
  if (token == tok)
    scan();
  else
    error(msg);
}

Sexp *parse_atom() {
  Sexp *x;
  if (token == ID)
    x = new Id(id);
  else if (token == NUM)
    x = new Num(nval);
  else
    error("expected <number> or <ident>.");
  scan();
  return x;
}

Sexp *parse();
Sexp *parse2();

Sexp *parse3(Sexp *p) {
  Sexp *x;
  if (token == RPAR) {
    x = new Pair(p, NIL);
    scan();
  } else if (token == DOT) {
    scan();
    x = new Pair(p, parse());
    expect(RPAR, "expected ).");
  } else if (token == NUM || token == ID || token == LPAR) 
    x = new Pair(p, parse2());
  else
    error("expected (, ), ., <number>, or <ident>.");
  return x;
}

Sexp *parse2() {
  return parse3(parse());
}

Sexp *parse() {		// recursive-descent parser
  if (token == ID || token == NUM)
    return parse_atom();
  else if (token == LPAR) {
    scan();
    return parse2();
  } else if (token == END)
    return 0;
  else
    error("expected <number>, <ident>, or '('.");
  return 0;
}

Env::Env(int s) {
  max = (s < 16 && s > 0) ? s : 16;
  free = 0;
  vec = new env_pair[max];
}

Env::Env(Env& e) {
  free = e.free;
  vec = new env_pair[max = e.max];
  for (int i = 0; i < free; i++)
    vec[i] = e.vec[i];
}

void Env::operator=(Env& e) {
  if (this == &e)			// beware of env=env;
    return;
  delete vec;				// cleanup lhs first
  free = e.free;
  vec = new env_pair[max = e.max];
  for (int i = 0; i < free; i++)	// do the assignment
    vec[i] = e.vec[i];
}

Sexp *Env::lookup(Atom *p) {
  register env_pair *pp;
  for (pp = &vec[free-1]; vec <= pp; pp--) // must be top-down search
    if (EQ(p, pp->id) == TRUE)
      return pp->val;
  error("unbound atom: "); return 0;
}

void Env::bind(Atom *p, Sexp *val) {
  register env_pair *pp;
  if (free == max) {			// overflow, grow the vector
    env_pair *nvec = new env_pair[max*2];
    for (int i=0; i < max; i++)
      nvec[i] = vec[i];
    delete vec;
    vec = nvec;
    max *= 2;
  }
  pp = &vec[free++];
  pp->id = p;
  pp->val = val;
}

void Env::print() {
  for (int i = 0; i < free; i++)
    cout << vec[i].id  << ":" << vec[i].val << "\n";
}

int main() {
  // define some symbols, setup initial environment, init scanner
  NIL        = new Id("NIL");     TRUE       = new Id("T");
  IF_SYM     = new Id("IF");      LET_SYM    = new Id("LET");
  EQ_SYM     = new Id("EQ");      ADD_SYM    = new Id("ADD");
  SUB_SYM    = new Id("SUB");     MUL_SYM    = new Id("MUL");
  DIV_SYM    = new Id("DIV");     DEF_SYM    = new Id("DEFINE");
  CAR_SYM    = new Id("CAR");     CDR_SYM    = new Id("CDR");
  CONS_SYM   = new Id("CONS");    ATOM_SYM   = new Id("ATOM");
  QUOTE_SYM  = new Id("QUOTE");   LAMBDA_SYM = new Id("LAMBDA");
  LETREC_SYM = new Id("LETREC");

  Env env(0);
  env.bind(TRUE, TRUE);	// TRUE evaluates to itself
  env.bind(NIL, NIL);		// so does NIL

  scan();

  // read, eval, print...
  Sexp *x;
  while ((x = parse())) {
    cout << "==> " << x << "\n";
    cout << EVAL(x, env) << "\n";
  }
  return 0;
}
