#include <iomanip>
#include <sstream>
#include <regex.hpp>

#include "eui48_t.h"

using namespace std;
using namespace seacl;

eui48_t::eui48_t(){
  exp_eui48 = "^([0-9a-fA-F]{2})[:| |-]?([0-9a-fA-F]{2})[:| |-]?([0-9a-fA-F]{2})[:| |-]?([0-9a-fA-F]{2})[:| |-]?([0-9a-fA-F]{2})[:| |-]?([0-9a-fA-F]{2})$";
  _empty = true;
}
eui48_t::~eui48_t(){}

bool eui48_t::parse(string str){
  boost::regex exp(exp_eui48);
  boost::smatch res;

  if (!boost::regex_match(str,res,exp)) return false;
  for (int buf,i=0 ; i < 6 ; i++){
    stringstream((string)res[i+1]) >> hex >> buf;
    value[i] = buf;
  }

  _empty = false;
  return true;
}

bool eui48_t::empty(){
  return _empty;
}

void eui48_t::clear(){
  _empty = true;
}

string eui48_t::str(){
  stringstream buf;

  if(_empty) return string("");

  buf << hex << setfill('0')
      << setw(2) << (int)value[0] << ':'
      << setw(2) << (int)value[1] << ':'
      << setw(2) << (int)value[2] << ':'
      << setw(2) << (int)value[3] << ':'
      << setw(2) << (int)value[4] << ':'
      << setw(2) << (int)value[5];

  return buf.str();
}
