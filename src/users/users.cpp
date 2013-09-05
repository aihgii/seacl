#include <stdlib.h>
#include <getopt.h>
#include <libintl.h>
#include <iostream>
#include <locale>
#include <string>
#include <sstream>
#include <map>
#include <boost/property_tree/ini_parser.hpp>

#include "acl.h"
#include "eui48_t.h"

#define _(s) gettext(s)

using namespace std;

void show(int,char**,seacl::acl&);
void add(int,char**,seacl::acl&);
void del(int,char**,seacl::acl&);
void mod(int,char**,seacl::acl&);

int main(int argc,char* argv[]){
  try
  {
    locale::global(std::locale(""));
    if(argc < 2) throw invalid_argument("Too few arguments");

    int cur_arg = 0;
    bool global_args = true;
    string config_path = "/etc/conf.d/seacl";
    boost::property_tree::ptree config;
    boost::property_tree::ptree subcfg;
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
    users.connect(subcfg.get<std::string>("database"),
                  subcfg.get<std::string>("server"),
                  subcfg.get<std::string>("user"),
                  subcfg.get<std::string>("password"));

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

void show(int argc,char **argv,seacl::acl &users){
  int arg;
  const char* short_options = "g:sm";
  const struct option long_options[] = {
    {"gid",required_argument,NULL,'g'},
    {"source",no_argument,NULL,'s'},
    {"mac",no_argument,NULL,'m'},
    {NULL,0,NULL,0}
  };

  users.unsetf();
  while ((arg = getopt_long(argc,argv,short_options,long_options,NULL)) != -1){
    switch(arg){
      case 's':
        users.setf(seacl::acl::F_SRC);
        break;
      case 'm':
        users.setf(seacl::acl::F_EUI48);
        break;
    }
  }

  cout << users.table();
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
