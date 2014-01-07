/*
 * Copyright (C) 2013—2014, Evgeny Vais <vais.evgeny@gmail.com>
 *
 * This file is part of SEACL.
 *
 *  SEACL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.

 *  SEACL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with SEACL.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <sstream>
#include <boost/regex.hpp>

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
