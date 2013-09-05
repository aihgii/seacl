#include <stdlib.h>
#include <libintl.h>
#include "acl.h"

#define SPC 1
#define DELIM '-'
#define _(s) gettext(s)

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

wstring acl::get_wstring(const char *c_str){
  size_t count;
  wchar_t *buf;
  wstring wstr;

  count = mbsrtowcs(NULL,&c_str,0,NULL);
  buf = new wchar_t [count];
  mbsrtowcs(buf,&c_str,count,NULL);
  wstr = wstring(buf,count);
  delete buf;

  return wstr;
}

string acl::get_string(const wchar_t *c_wstr){
  size_t count;
  char *buf;
  string str;

  count = wcsrtombs(NULL,&c_wstr,0,NULL);
  buf = new char [count];
  wcsrtombs(buf,&c_wstr,count,NULL);
  str = string(buf,count);
  delete buf;

  return str;
}

acl::wTable* acl::get_table(mysqlpp::UseQueryResult &res){
  wRow row;
  wTable *table = new wTable;

  while (mysqlpp::Row r = res.fetch_row()){
    for (auto it = r.begin(); it < r.end(); it++){
      row.push_back(get_wstring(it->c_str()));
    }
    table->push_back(row);
    row.clear();
  }

  return table;
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

string acl::table(){
  int i;
  int table_width;
  int table_size;
  vector <int> column_width;
  wRow header;
  wstringstream result,sep;

  mysqlpp::Query query = db.query();
  query << "SELECT `Users`.*";
  if((table_flags & F_SRC) == F_SRC) query << ",`Source`.`Value`";
  if((table_flags & F_EUI48) == F_EUI48) query << ",`EUI48`.`Value`";
  query << " FROM `Users`";
  if((table_flags & F_SRC) == F_SRC) query << " LEFT JOIN `Source` ON (`Users`.`UID` = `Source`.`UID`)";
  if((table_flags & F_EUI48) == F_EUI48) query << " LEFT JOIN `EUI48` ON (`Users`.`UID` = `EUI48`.`UID`)";

  mysqlpp::UseQueryResult res = query.use();
  wTable *table = get_table(res);
  table_size = table->size();

  header.push_back( get_wstring(_("UID")) );
  header.push_back( get_wstring(_("GID")) );
  header.push_back( get_wstring(_("Username")) );
  if((table_flags & F_SRC) == F_SRC)
    header.push_back( get_wstring(_("Source")) );
  if((table_flags & F_EUI48) == F_EUI48)
    header.push_back( get_wstring(_("MAC address")) );

  for (auto it = header.begin(); it < header.end(); it++)
    column_width.push_back(it->length());

  for (auto it = table->begin(); it < table->end(); it++)
    for (i = 0; i < column_width.size(); i++)
      if (it->at(i).length() > column_width[i]) column_width[i] = it->at(i).length();

  table_width = SPC;
  for (auto it = column_width.begin(); it < column_width.end(); it++)
    table_width += *it + SPC*2;
  sep << std::setfill(wchar_t(DELIM)) << setw(table_width) << '\n' << std::setfill(wchar_t(' '));

  i = 0;
  result << sep.str();
  result << setw(SPC+column_width[i]) << header[i];
  result << setw(SPC*2+column_width[++i]);
  result << header[i] << setw(SPC*2) << ' ';
  result << left << setw(column_width[++i]+SPC*2);
  result << header[i];
  if((table_flags & F_SRC) == F_SRC){
    result << setw(column_width[++i]+SPC*2);
    result << header[i];}
  if((table_flags & F_EUI48) == F_EUI48)
    result << header[++i];
  result << endl << sep.str();

  while (table->size()){
    i = 0;
    result << right
           << setw(SPC+column_width[i]) << table->front().at(i) << setw(SPC*2) << ' ';
    result << setw(column_width[++i]);
    result << table->front().at(i) << setw(SPC*2) << ' ';
    result << left << setw(column_width[++i]);
    result << table->front().at(i) << setw(SPC*2) << ' ';
    if((table_flags & F_SRC) == F_SRC){
      result << setw(column_width[++i]);
      result << table->front().at(i) << setw(SPC*2) << ' ';}
    if((table_flags & F_EUI48) == F_EUI48)
      result << table->front().at(++i);
    result << endl;
    table->pop_front();
  }
  delete table;

  result << sep.str();
  result << get_wstring(_("Users in table: ")) << table_size << endl;

  return get_string(result.str().c_str());
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
