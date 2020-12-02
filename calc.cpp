// Tyler Amos, Justin Canedy
// tamos5@jhu.edu, jcanedy1@jhu.edu
#include "calc.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <algorithm>
using std::stringstream;
using std::vector; using std::map;
using std::string;
struct Calc {
};

class CalcImpl : public Calc {
  public:
    map<string, int> vars;
    // Member Functions
    int evalExpr(const char *expr, int &result);
    int parsed_eval(vector<string> &expr, bool &error);
    bool validVar(string tok);
    bool validOpnd(string tok);
};

// constructor for calculator object
extern "C" struct Calc *calc_create(void) {
  return new CalcImpl();
}

// Destructor for calculator object
extern "C" void calc_destroy(struct Calc *calc) {
  CalcImpl *obj = static_cast<CalcImpl *>(calc);
  delete obj;
}

// Compute an evaluation given a calculator object, and expression. 
// Store the result in a pointer to result
extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {
  CalcImpl *obj = static_cast<CalcImpl *>(calc);
  return obj->evalExpr(expr, *result);
}

// Split an expression into tokens
vector<string> tokenize(const char *expr) {
  vector<string> vec;
  stringstream s(expr);

  string tok;
  while (s >> tok) {
    vec.push_back(tok);
  }

  return vec;
}

// Given an expression, calculate its result
// return 1 if successful, 0 if error
int CalcImpl::evalExpr(const char *expr, int &result) {
  vector<string> parsed_expr = tokenize(expr);
  vector<string> remExpr;
  bool error = false;
  // handle variable assignment
  if (std::find(parsed_expr.begin(), parsed_expr.end(), "=") != parsed_expr.end()) {
    string var = parsed_expr[0];
    if (!validVar(var)) {
      return 0;
    }

    // evaluate expression following declaration
    for (int i = 2; i < int(parsed_expr.size()); i++) {
      remExpr.push_back(parsed_expr[i]);
    }

    // evaluate rest of expression to store in variable
    result = parsed_eval(remExpr, error);
    if (error) {
      return 0;
    }

    // declare and set iterator to variable name input
    map<string, int>::iterator itr;
    itr = this->vars.find(var);
   
    // update variable entry or create new one if it doesn't exist
    if (itr != this->vars.end()) {
      itr->second = result;
    } else {
      this->vars.insert({var, result});
    }
    return 1;
  }

  // compute an expression without variables
  result = parsed_eval(parsed_expr, error);
  if (error) {
    return 0;
  }
  return 1;
}

// Check if a valid operand
bool CalcImpl::validOpnd(string tok) {
  for (int i = 0; i < int(tok.length()); i++) {
    if (!(tok[i] > 47 && tok[i] < 58)) {
      if (i == 0) {
        if (tok[i] == 45) {
	  continue;
	}
      }
      return false;
    }
  }
  return true;
}

// Check if a valid variable
bool CalcImpl::validVar(string tok) {
  for (int i = 0; i < int(tok.length()); i++) {
    if (!(tok[i] > 64 && tok[i] < 91) && !(tok[i] > 96 && tok[i] < 123)) {
      return false;
    }
  }
  return true;
}

// specific arithmetic evaluation
int CalcImpl::parsed_eval(vector<string> &expr, bool &error) {
  vector<int> operands;
  int size = expr.size();
  // only 1 operand
  if (size == 1) {
    // check if valid token
    if (!validOpnd(expr[0]) && !validVar(expr[0])) { 
      error = true;
      return 0;
    }
  
    // check if a variable
    if (validVar(expr[0])) {
      map<string, int>::iterator itr;
      // search dictionary for variable
      itr = this->vars.find(expr[0]);
      if (itr != this->vars.end()) {
        return itr->second;
      } else {
        error = true;
	return 0;
      }
    }
    // if not a variable, return the number
    return atoi(const_cast<char*>(expr[0].c_str()));
  }

  // invalid combination
  if (size == 2) {
    error = true;
    return 0;
  }
  
  // full expression
  if (size == 3) {
    // check if 1st operand is a variable
    if (validVar(expr[0])) {
      map<string, int>::iterator itr;
      itr = this->vars.find(expr[0]);
      if (itr != this->vars.end()) {
        operands.push_back(itr->second);
      } else {
        error = true;
	return 0;
      }
    } else if (!validOpnd(expr[0])) { // must be valid opnd then
      error = true;
      return 0;
    } else {
      operands.push_back(atoi(const_cast<char*>(expr[0].c_str())));
    }

    // check if 2nd operand is a variable
    if (validVar(expr[2])) {
      map<string, int>::iterator itr;
      itr = this->vars.find(expr[2]);
      if (itr != this->vars.end()) {
        operands.push_back(itr->second);
      } else {
        error = true;
	return 0;
      }
    } else if (!validOpnd(expr[2])) { // must be valid opnd then
      error = true;
      return 0;
    } else {
      operands.push_back(atoi(const_cast<char*>(expr[2].c_str())));
    }
    
    // extract the op as a char
    char* opr = const_cast<char*>(expr[1].c_str());
    char op = opr[0];  
    
    // do computation 
    switch(op) {
      case '+':
        return operands[0] + operands[1];
    
      case '-':
        return operands[0] - operands[1];

      case '*':
        return operands[0] * operands[1];

      case '/':
        // catch divide by 0
	if (operands[1] == 0) {
	  error = true;
	  return 0;
	} 
	return operands[0] / operands[1];

      default:
        error = true;
	return 0;
    }
  }
  // if all goes wrong, error
  error = true;
  return 0;
}

