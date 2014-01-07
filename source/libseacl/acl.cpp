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

#include <stdlib.h>
#include <libseacl/acl.h>

using namespace std;
using namespace seacl;

acl::acl(){
  table_flags = 0;
}

acl::acl(std::string database,
         std::string server,
         std::string user,
         std::string password)
{
  connect(database,server,user,password);
  table_flags = 0;
}

acl::~acl(){
  db.disconnect();
}

bool acl::connect(std::string database,
                  std::string server,
                  std::string user,
                  std::string password)
{
  return db.connect(database.c_str(),server.c_str(),user.c_str(),password.c_str());
}

void acl::setf(acl::flag f){
  table_flags |= f;
}

void acl::unsetf(){
  table_flags = 0;
}

void acl::unsetf(acl::flag f){
  table_flags -= f;
}

int acl::add(int gid,string username){
  int uid;
  vector <mysqlpp::Row> res;
  mysqlpp::Query query = db.query();

  query << "SELECT * FROM `Users` WHERE `Username` = '" << username << "'";
  query.storein(res);
  if(!res.empty()) throw logic_error("User with such name is already exist");

  query << "INSERT INTO `Users` SET `GID` = '" << gid << "',`Username` = '" << username << "'";
  query.execute();

  query << "SELECT `UID` FROM `Users` WHERE `Username` = '" << username << "'";
  query.storein(res);
  uid = atoi(res[0]["UID"].c_str());

  query << "INSERT INTO `Source` SET `UID` = '" << uid << "'";
  query.execute();
  query << "INSERT INTO `EUI48` SET `UID` = '" << uid << "'";
  query.execute();

  return uid;
}

void acl::del(int uid){
  mysqlpp::Query query = db.query();

  if(!exist(uid)) throw logic_error("User with that UID does't exist");

  query << "DELETE FROM `Users` WHERE `UID` = '" << uid << "'";
  query.execute();
}

void acl::mod(int uid,int gid,string username){
  vector <mysqlpp::Row> res;
  mysqlpp::Query query = db.query();

  if(username != acl::username(uid)){
    query << "SELECT * FROM `Users` WHERE `Username` = '" << username << "'";
    query.storein(res);
    if (!res.empty()) throw logic_error("User with such name is already exist");
  }

  query << "UPDATE `Users` SET `GID` = '" << gid << "',`Username` = '" << username << "' WHERE `UID` = '" << uid << "'";
  query.execute();
}

void acl::mod_eui48(int uid,string mac){
  vector <mysqlpp::Row> res;
  mysqlpp::Query query = db.query();

  if(mac != acl::eui48(uid)){
    query << "SELECT * FROM `EUI48` WHERE `Value` = '" << mac << "'";
    query.storein(res);
    if(!res.empty()) throw logic_error("User with such MAC address is already exist");
  }

  query << "UPDATE `EUI48` SET `Value` = '" << mac << "' WHERE `UID` = '" << uid << "'";
  query.execute();
}

bool acl::exist(int uid){
  vector <mysqlpp::Row> res;
  mysqlpp::Query query = db.query();

  if(!uid) return false;
  query << "SELECT * FROM `Users` WHERE `UID` = '" << uid << "'";
  query.storein(res);

  return !res.empty();
}

int acl::gid(int uid){
  vector <mysqlpp::Row> res;
  mysqlpp::Query query = db.query();

  query << "SELECT `GID` FROM `Users` WHERE `UID` = '" << uid << "'";
  query.storein(res);
  if (res.empty()) throw logic_error("User with that UID does't exist");
  return atoi(res[0]["GID"]);
}

string acl::username(int uid){
  vector <mysqlpp::Row> res;
  mysqlpp::Query query = db.query();

  query << "SELECT `Username` FROM `Users` WHERE `UID` = '" << uid << "'";
  query.storein(res);
  if (res.empty()) throw logic_error("User with that UID does't exist");
  return string(res[0]["Username"]);
}

string acl::eui48(int uid){
  vector <mysqlpp::Row> res;
  mysqlpp::Query query = db.query();

  query << "SELECT `Value` FROM `EUI48` WHERE `UID` = '" << uid << "'";
  query.storein(res);
  if (res.empty()) throw logic_error("User with that UID does't exist");
  return string(res[0]["Value"]);
}

acl::Table* acl::table(){
  char* buf;
  mysqlpp::UseQueryResult res;
  mysqlpp::Query query = db.query();
  Row row;
  auto* table = new Table;

  query << "SELECT `Users`.*";
  if((table_flags & F_EUI48) == F_EUI48) query << ",`EUI48`.`Value`";
  query << " FROM `Users`";
  if((table_flags & F_EUI48) == F_EUI48) query << " LEFT JOIN `EUI48` ON (`Users`.`UID` = `EUI48`.`UID`)";
  res = query.use();

  while (mysqlpp::Row r = res.fetch_row()){
    for(auto it = r.begin(); it < r.end(); it++){
      buf = new char [(it->length() + 1)];
      row.push_back(strcpy(buf,it->c_str()));
    }
    table->push_back(row);
    row.clear();
  }

  return table;
}
