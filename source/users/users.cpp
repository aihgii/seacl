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
#include <getopt.h>
#include <libintl.h>
#include <iostream>
#include <locale>
#include <string>
#include <sstream>
#include <map>
#include <boost/property_tree/ini_parser.hpp>
#include <libseacl.h>

#include "eui48_t.h"

#define SPC 1
#define DELIM '-'
#define _(s) gettext(s)

using namespace std;

wstring get_wstring(const char *);

void show(int,char**,seacl::acl&);
void add(int,char**,seacl::acl&);
void del(int,char**,seacl::acl&);
void mod(int,char**,seacl::acl&);

int main(int argc,char* argv[]){
  try
  {
    locale::global(std::locale(""));
    ios::sync_with_stdio(false);

    if(argc < 2) throw invalid_argument("Too few arguments");

    int cur_arg = 0;
    bool global_args = true;
    string config_path = "/etc/seacl/seacl.conf";
    boost::property_tree::ptree config;
    boost::property_tree::ptree subcfg;
    boost::property_tree::ptree myauth;
    map <string,int> cmd;
    seacl::acl users;
  
    cmd["show"]	= 1;
    cmd["add"]	= 2;
    cmd["del"]	= 3;
    cmd["mod"]	= 4;

    cmd["-c"]		= 5;
    cmd["--config"]	= 5;

    while(global_args){
      cur_arg++;
      switch(cmd[argv[cur_arg]])
      {
        case 1:
        case 2:
        case 3:
        case 4:
          global_args = false;
          break;
        case 5:
          if(argc <= cur_arg+1) break;
          config_path = argv[++cur_arg];
          break;
        default:
          throw invalid_argument("Unknown command");
      }
      if(argc <= cur_arg+1) break;
    }

    boost::property_tree::read_ini(config_path,config);
    subcfg = config.get_child("MySQL");
    boost::property_tree::read_ini(subcfg.get<std::string>("auth"),myauth);
    users.connect(myauth.get<std::string>("database"),
                  myauth.get<std::string>("server"),
                  myauth.get<std::string>("user"),
                  myauth.get<std::string>("password"));

    switch(cmd[argv[cur_arg]])
    {
      case 1:
        show(argc-cur_arg,argv+cur_arg,users);
        break;
      case 2:
        add(argc-cur_arg,argv+cur_arg,users);
        break;
      case 3:
        del(argc-cur_arg,argv+cur_arg,users);
        break;
      case 4:
        mod(argc-cur_arg,argv+cur_arg,users);
        break;
    }
  }

  catch(exception& e)
  {
    int code;
    stringstream message;
    map <string,int> excp;

    excp["Too few arguments"]						= 100;
    excp["Unknown command"]							= 101;
    excp["MAC address is not correctly entered"]	= 102;
    excp["Null name"]								= 103;
    excp["UID is not valid"]						= 104;
    excp["UID required"]							= 105;

    excp["User with that UID does't exist"]				= 200;
    excp["User with such name is already exist"]		= 201;
    excp["User with such MAC address is already exist"]	= 202;

    message << "ERR ";
    code = excp[e.what()];
    switch(code)
    {
      case 100:
        message << code << ": "
                << _("Too few arguments");
        break;
      case 101:
        message << code << ": "
                << _("Unknown command");
        break;
      case 102:
        message << code << ": "
                << _("MAC address is not correctly entered");
        break;
      case 103:
        message << code << ": "
                << _("Null name");
        break;
      case 104:
        message << code << ": "
                << _("UID is not valid");
        break;
      case 105:
        message << code << ": "
                << _("UID required");
        break;

      case 200:
        message << code << ": "
                << _("User with that UID does't exist");
        break;
      case 201:
        message << code << ": "
                << _("User with such name is already exist");
        break;
      case 202:
        message << code << ": "
                << _("User with such MAC address is already exist");
        break;
      default:
        message << ++code << ": "
                << "Non provided exception: "
                << e.what();
    }
    cerr << message.str() << endl;

    exit(code);
  }

  catch(...)
  {
    cerr << "ERR 2: Unknown error\n";
    exit(2);
  }

  return EXIT_SUCCESS;
}

wstring get_wstring(const char *c_str){
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

void show(int argc,char **argv,seacl::acl &users){
  int i,arg,
      table_width,
      table_size;
  bool show_mac = false;
  vector <int> column_width;
  stringstream sep;
  seacl::acl::Row header;
  seacl::acl::Table* table;

  const char* short_options = "g:m";
  const struct option long_options[] = {
    {"gid",required_argument,NULL,'g'},
    {"mac",no_argument,NULL,'m'},
    {NULL,0,NULL,0} };
  users.unsetf();
  while ((arg = getopt_long(argc,argv,short_options,long_options,NULL)) != -1){
    switch(arg){
      case 'm':
        users.setf(seacl::acl::F_EUI48);
        show_mac = true;
        break;
    }
  }

  table = users.table();
  table_size = table->size();

  header.push_back( _("UID") );
  header.push_back( _("GID") );
  header.push_back( _("Username") );
  if(show_mac)
    header.push_back( _("MAC address") );

  for(auto it = header.begin(); it < header.end(); it++)
    column_width.push_back(get_wstring(*it).length());

  for(auto it = table->begin(); it < table->end(); it++)
    for (i = 0; i < column_width.size(); i++)
      if (get_wstring(it->at(i)).length() > column_width[i]) column_width[i] = get_wstring(it->at(i)).length();

  table_width = SPC;
  for (auto it = column_width.begin(); it < column_width.end(); it++)
    table_width += *it + SPC*2;
  sep << std::setfill(DELIM) << setw(table_width) << '\n';

  i = 0;
  cout << sep.str() << flush;
  wcout << setw(SPC+column_width[i]) << get_wstring(header[i]);
  wcout << setw(SPC*2+column_width[++i]);
  wcout << get_wstring(header[i]) << setw(SPC*2) << ' ';
  wcout << left << setw(column_width[++i]+SPC*2);
  wcout << get_wstring(header[i]);
  if(show_mac)
    wcout << get_wstring(header[++i]);
  wcout.flush();
  cout << endl << sep.str();

  while (!table->empty()){
    i = 0;
    cout << right
         << setw(SPC+column_width[i]) << table->front().at(i) << setw(SPC*2) << ' ';
    delete table->front().at(i);
    cout << setw(column_width[++i]);
    cout << table->front().at(i) << setw(SPC*2) << ' ' << flush;
    delete table->front().at(i);
    wcout << left << setw(column_width[++i]);
    wcout << get_wstring(table->front().at(i)) << setw(SPC*2) << ' ' << flush;
    delete table->front().at(i);
    if(show_mac){
      cout << table->front().at(++i);
      delete table->front().at(i);}
    cout << endl;
    table->pop_front();
  }
  delete table;

  cout << sep.str();
  cout << _("Users in table: ") << table_size << endl;
}

void add(int argc,char **argv,seacl::acl &users){
  int arg,uid,gid = 0;
  string name;
  seacl::eui48_t mac;
  const char* short_options = "g:n:m:";
  const struct option long_options[] = {
    {"gid",required_argument,NULL,'g'},
    {"name",required_argument,NULL,'n'},
    {"mac",required_argument,NULL,'m'},
    {NULL,0,NULL,0}
  };

  while ((arg = getopt_long(argc,argv,short_options,long_options,NULL)) != -1){
    switch(arg){
      case 'g':
        gid = atoi(optarg);
        break;
      case 'n':
        name = optarg;
        break;
      case 'm':
        mac.parse(optarg);
        if(mac.empty()) throw invalid_argument("MAC address is not correctly entered");
        break;
    }
  }

  if(name == "") throw invalid_argument("Null name");
  uid = users.add(gid,name);
  if(!mac.empty())
    users.mod_eui48(uid,mac.str());
}

void del(int argc,char **argv,seacl::acl &users){
  if(argc < 2) throw invalid_argument("Too few arguments");

  int uid = atoi(argv[1]);
  if(uid)
    users.del(uid);
  else
    throw invalid_argument("UID is not valid");
}

void mod(int argc,char **argv,seacl::acl &users){
  int arg,uid = 0,gid = -1;
  string name;
  seacl::eui48_t mac;
  const char* short_options = "u:g:n:m:";
  const struct option long_options[] = {
    {"uid",required_argument,NULL,'u'},
    {"gid",required_argument,NULL,'g'},
    {"name",required_argument,NULL,'n'},
    {"mac",required_argument,NULL,'m'},
    {NULL,0,NULL,0}
  };

  

  while ((arg = getopt_long(argc,argv,short_options,long_options,NULL)) != -1){
    switch(arg){
      case 'u':
        uid = atoi(optarg);
        if(!uid) throw invalid_argument("UID is not valid");
        break;
      case 'g':
        gid = atoi(optarg);
        break;
      case 'n':
        name = optarg;
        break;
      case 'm':
        mac.parse(optarg);
        if(mac.empty()) throw invalid_argument("MAC address is not correctly entered");
        break;
    }
  }

  if(!uid) throw invalid_argument("UID required");
  if(gid == -1) gid = users.gid(uid);
  if(name == "") name = users.username(uid);

  users.mod(uid,gid,name);
  if(!mac.empty())
    users.mod_eui48(uid,mac.str());
}
