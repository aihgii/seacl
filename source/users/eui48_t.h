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

#ifndef EUI48_T_H
#define EUI48_T_H

#include <string>;

namespace seacl {

  class eui48_t
  {
  public:
     eui48_t();
    ~eui48_t();

    bool parse(std::string);
    bool empty();
    void clear();
    std::string str();
  private:
    uint8_t value[6];
    std::string exp_eui48;
    bool _empty;
  };

}

#endif
