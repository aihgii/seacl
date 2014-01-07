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

#ifndef ACL_H
#define ACL_H

#include <deque>
#include <vector>
#include <string>
#include <mysql++.h>

namespace seacl {

  class acl
  {
  public:
    typedef uint8_t flag;
    typedef std::vector <const char*> Row;
    typedef std::deque  <Row>         Table;

    acl();
    acl(std::string,
        std::string,
        std::string,
        std::string);
    ~acl();

    bool connect(std::string,
                 std::string,
                 std::string,
                 std::string);

    static const flag F_EUI48	= 1 << 0;

    void setf(flag);
    void unsetf();
    void unsetf(flag);

    int  add(int,std::string);
    void del(int);
    void mod(int,int,std::string);
    void mod_eui48(int,std::string);

    bool exist(int);
    int gid(int);
    std::string username(int);
    std::string eui48(int);
    Table* table();
  protected:
    mysqlpp::Connection db;
    flag table_flags;
  };

}

#endif
