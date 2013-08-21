#include <stdlib.h>
#include <getopt.h>
#include <libintl.h>
#include <iostream>
#include <locale>
#include <string>
#include <sstream>
#include <map>
#include <property_tree/ini_parser.hpp>

#include "acl.h"
#include "eui48_t.h"

#define _(s) gettext(s)

using namespace std;

void show(int,char**,seacl::acl&);
void add(int,char**,seacl::acl&);
void del(int,char**,seacl::acl&);
void mod(int,char**,seacl::acl&);

int main(int argc,char* argv[]){
  locale::global(std::locale(""));
  if(argc < 2) { cerr << "ERR: Too few arguments\n"; exit(1); }

  int cur_arg = 0;
  bool global_args = true;
  string config_path = "/etc/conf.d/seacl";
  map <string,int> cmd;
  boost::property_tree::ptree config;
  boost::property_tree::ptree subcfg;
  seacl::acl users;
  
  cmd["show"]	= 1;
  cmd["add"]	= 2;
  cmd["del"]	= 3;
  cmd["mod"]	= 4;

  cmd["-c"]			= 5;
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
        cerr << "ERR: Unknown command: " << argv[cur_arg] << endl;
        exit (1);
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

  return EXIT_SUCCESS;
}

void show(int argc,char **argv,seacl::acl &users){
  int arg;
  const char* short_options = "g:m";
  const struct option long_options[] = {
    {"gid",required_argument,NULL,'g'},
    {"mac",no_argument,NULL,'m'},
    {NULL,0,NULL,0}
  };

  users.unsetf();
  while ((arg = getopt_long(argc,argv,short_options,long_options,NULL)) != -1){
    switch(arg){
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
        if(mac.empty()) { cerr << "ERR: MAC address is not correctly entered: " << optarg << endl; exit(1); }
        break;
    }
  }

  if(name == "") { cerr << "ERR: Null name\n"; exit(1); }
  uid = users.add(gid,name);
  if(!mac.empty())
    users.mod_eui48(uid,mac.str());
}

void del(int argc,char **argv,seacl::acl &users){
  if(argc < 2) { cerr << "ERR: Too few arguments\n"; exit(1); }

  int uid = atoi(argv[1]);
  if(uid)
    users.del(uid);
  else{
    cerr << "ERR: UID is not valid: " << argv[1] << endl;
    exit(1);}
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
        if(!uid) { cerr << "ERR: UID is not valid: " << optarg << endl; exit(1); }
        break;
      case 'g':
        gid = atoi(optarg);
        break;
      case 'n':
        name = optarg;
        break;
      case 'm':
        mac.parse(optarg);
        if(mac.empty()) { cerr << "ERR: MAC address is not correctly entered: " << optarg << endl; exit(1); }
        break;
    }
  }

  if(!uid) { cerr << "ERR: UID required\n"; exit(1); }
  if(!users.exist(uid)) { cerr << "ERR: User with UID " << uid << " does not exist\n"; exit(1); }
  if(gid == -1) gid = users.gid(uid);
  if(name == "") name = users.username(uid);

  users.mod(uid,gid,name);
  if(!mac.empty())
    users.mod_eui48(uid,mac.str());
}
